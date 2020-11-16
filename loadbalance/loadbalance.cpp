#include <thread>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "loadbalance.h"
#include "message_headers.h"
#include "threadpool_instance.h"
#include "net_message_listener.h"

using namespace media_core_message;

#define INDEX_SERVER 0
#define INDEX_CLIENT 1

void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> mess)
{
#ifdef PUSH_INTO_THREAD_POOL
#undef PUSH_INTO_THREAD_POOL
#endif 

#define PUSH_INTO_THREAD_POOL(type) threadpool_instance::get_instance()->schedule(std::bind(static_cast<int(loadbalance::*)(const connection, std::shared_ptr<type>)>(&loadbalance::deal_message), loadbalance::get_instance(), conn, std::dynamic_pointer_cast<type>(mess))); break;

	switch (message_type)
	{
	case MSGTYPE_KEEPALIVE:						PUSH_INTO_THREAD_POOL(keepalive_message);
	case MSGTYPE_PULLOTHERLOADBALANCE:			PUSH_INTO_THREAD_POOL(pull_other_loadbalance_message);
	case MSGTYPE_RESPONDLOADBALANCEPULL:		PUSH_INTO_THREAD_POOL(respond_loadbalance_pull_message);
	case MSGTYPE_RESPONDMEDIAMENU:				PUSH_INTO_THREAD_POOL(respond_media_menu_pull_message);
	case MSGTYPE_PUSHMEDIAMENU:					PUSH_INTO_THREAD_POOL(push_media_pull_message);
	case MSGTYPE_CLIENTPULLMEDIASTREAM:			PUSH_INTO_THREAD_POOL(client_pull_media_stream_message);
	case MSGTYPE_RESOURCERESPONDMEDIAPULL:		PUSH_INTO_THREAD_POOL(resource_server_respond_media_menu_pull_message);
	case MSGTYPE_STOPSTREAM:					PUSH_INTO_THREAD_POOL(stop_stream_message);
	case MSGTYPE_RESOURCESERVERREPORT:			PUSH_INTO_THREAD_POOL(resource_server_report_message);
	case MSGTYPE_STREAMFRAME:
	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM:
	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:
	case MSGTYPE_CLIENTSTREAMTRIGGER:
	case MSGTYPE_PUSHLOADBANLANCE:
	case MSGTYPE_PULLMEDIAMENU:
		dzlog_error("recv a message should not send to loadbalance: %d", message_type);
		mess->print_data();
		return;

	default: 
		dzlog_error("recv a message of unknown type: %d", message_type);
		return;
	}

	loadbalance::get_instance()->inc_load();

#undef PUSH_INTO_THREAD_POOL
}

loadbalance* loadbalance::get_instance()
{
	static loadbalance instance;
	return &instance;
}

int loadbalance::listening(uint16_t port, uint32_t sid)
{
	net_message_listener::get_instance()->set_callback(&net_message_listener_callback);
	net_message_listener::get_instance()->listening(port);

	_port = _self_port = port;
	_sid	= sid;
	_status = 1;

	threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::shoting_dead, this));

	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<client_pull_media_stream_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_peer(id, PEER_TYPE_CLIENT))
	{
		dzlog_error("pull stream from a unknown client %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	std::string video_2_play;
	uint16_t recv_port	= 0;
	uint32_t tid		= 0;

	mess->give_me_data(tid, video_2_play, recv_port);

	uint32_t ssrc = find_idle_ssrc();

	int	selected_resource = check_video_and_find_resource_least_load(video_2_play);	// 这个同样也是 socket fd
	if (selected_resource == -1)
	{
		dzlog_error("failed to alloc resource server");
		return -1;
	}
	else if (selected_resource == -2)
	{
		dzlog_error("video: %s not exist", video_2_play.c_str());

		std::shared_ptr<resource_server_report_message> next_mess = std::make_shared<resource_server_report_message>();
		next_mess->full_data_direct(tid, 1);
		conn.send_message(next_mess);

		return -1;
	}

	if (insert_id_tid_2_ssrc(INDEX_SERVER, selected_resource, tid, ssrc)) return -1;
	if (insert_id_tid_2_ssrc(INDEX_CLIENT, id, tid, ssrc))
	{
		remove_id_tid_2_ssrc(INDEX_SERVER, selected_resource, tid);
		return -1;
	}

	std::shared_ptr<media_chain> ptr;
	ptr->set_client_recv_port(recv_port);
	ptr->set_client_recv_port(ssrc);

	{
		std::lock(_peer_map_lock[PEER_TYPE_CLIENT], _peer_map_lock[PEER_TYPE_RESOURCE]);

		ptr = std::make_shared<media_chain>(ssrc, _peer_map[PEER_TYPE_CLIENT][id], _peer_map[PEER_TYPE_RESOURCE][selected_resource], video_2_play);

		std::shared_ptr<loadbalance_pull_media_stream_message> next_mess = std::make_shared<loadbalance_pull_media_stream_message>();
		next_mess->full_data_direct(tid, recv_port, ssrc, video_2_play);

		_peer_map[PEER_TYPE_RESOURCE][selected_resource]->send_message(next_mess);
	}

	{
		std::lock_guard<std::mutex> lk(_media_chain_map_lock);
		_media_chain_map[ssrc] = ptr;
	}

	dzlog_info("send pull %s stream %s:%d -> %s:? socket(%d)->tid(%d)->ssrc(%d)", video_2_play.c_str(), conn.show_ip(), recv_port, _peer_map[PEER_TYPE_RESOURCE][selected_resource]->get_connection().show_ip(), id, tid, ssrc);
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<keepalive_message> mess)
{
	const int id = conn.show_sockfd();

	uint32_t tid; 
	uint32_t sid; 
	uint8_t count;

	mess->give_me_data(tid, sid, count);

	int type = tell_me_type(sid);
	if (type != PEER_TYPE_CLIENT || type != PEER_TYPE_LOADBALANCE || type != PEER_TYPE_RESOURCE)
	{
		dzlog_error("get a unknown type: %d", type);
		return -1;
	}

	if (2 == fresh_or_insert_peer(id, type, conn, sid))
	{
		dzlog_info("%s:%d keepalive", conn.show_ip(), conn.show_port());
		return 0;
	}

	switch (type)
	{
	// 新注册的负载均衡应该发给所有的客户端和视频资源
	case PEER_TYPE_LOADBALANCE:
		set_lb_map(conn.show_ip_raw(), conn.show_port(), true);

		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::send_new_loadbalance_2_client_resource, this, conn));
		dzlog_info("get a new loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return 0;

	// 新注册的视频资源应该拉取视频目录
	case PEER_TYPE_RESOURCE:		
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::send_media_menu_pulling, this, conn));
		dzlog_info("get a new resource %s:%d", conn.show_ip(), conn.show_port());
		return 0;

	// 新注册的客户端不需要做额外的事
	case PEER_TYPE_CLIENT:			
		dzlog_info("get a new client %s:%d", conn.show_ip(), conn.show_port());
		return 0;

	default:
		dzlog_error("something never happen");
		return -1;
	}

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<pull_other_loadbalance_message> mess)
{
	const int id = conn.show_sockfd();
	if (
		fresh_peer(id, PEER_TYPE_CLIENT) && 
		fresh_peer(id, PEER_TYPE_LOADBALANCE) && 
		fresh_peer(id, PEER_TYPE_RESOURCE)
	) {
		dzlog_error("get pull_other_loadbalance_message from unknown %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid;
	mess->give_me_data(tid);

	std::shared_ptr<respond_loadbalance_pull_message> next_mess = std::make_shared<respond_loadbalance_pull_message>();
	std::vector<uint32_t> ips;
	std::vector<uint16_t> ports;

	{
		std::lock_guard<std::mutex> lk(_peer_map_lock[PEER_TYPE_LOADBALANCE]);

		for (auto i = _peer_map[PEER_TYPE_LOADBALANCE].begin(); i != _peer_map[PEER_TYPE_LOADBALANCE].end(); ++i)
		{
			std::shared_ptr<peer> temp = i->second;

			ips.push_back(temp->get_connection().show_ip_raw());
			ports.push_back(temp->get_connection().show_port());
		}
	}

	next_mess->full_data_direct(tid, ips, ports);
	conn.send_message(next_mess);

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<push_media_pull_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_peer(id, PEER_TYPE_RESOURCE)) 
	{
		dzlog_error("push media menu from unknown resource: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	std::vector<std::string> video_names;
	uint32_t tid;

	mess->give_me_data(tid, video_names);

	{
		std::lock_guard<std::mutex> lk(_video_resources_lock);

		for (auto i = video_names.begin(); i != video_names.end(); ++i)
		{
			if (_video_resources.count(*i) && _video_resources[*i].count(id))
				continue;

			std::lock_guard<std::mutex> lk(_peer_map_lock[PEER_TYPE_RESOURCE]);
			_video_resources[*i][id] = _peer_map[PEER_TYPE_RESOURCE][id];
		}
	}

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<resource_server_report_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_peer(id, PEER_TYPE_RESOURCE)) 
	{
		dzlog_error("get report from unknown resource: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint8_t error;
	uint32_t tid; 
	mess->give_me_data(tid, error);

	int ssrc = get_ssrc(INDEX_SERVER, id, tid);
	if (ssrc == -1)
	{
		dzlog_error("cannot find ssrc by [id, tid](%d, %d)", id, tid);
		return -1;
	}

	{
		std::lock_guard<std::mutex> lk(_media_chain_map_lock);
		if (!_media_chain_map.count(ssrc))
			return 0;

		_media_chain_map[ssrc]->get_client()->get_connection().send_message(mess);
		_media_chain_map.erase(ssrc);
	}

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<resource_server_respond_media_menu_pull_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_peer(id, PEER_TYPE_RESOURCE)) 
	{
		dzlog_error("get media pulling respond from unknown resource: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid; 
	uint32_t length; 
	uint32_t width; 
	uint16_t server_send_port;

	mess->give_me_data(tid, length, width, server_send_port);

	int ssrc = get_ssrc(INDEX_SERVER, id, tid);
	if (ssrc == -1)
	{
		dzlog_error("cannot find ssrc by [id, tid](%d, %d)", id, tid);
		return -1;
	}

	std::shared_ptr<loadbalance_respond_media_menu_pull_message> next_mess;
	std::shared_ptr<media_chain> temp;

	{
		std::lock_guard<std::mutex> lk(_media_chain_map_lock);
		if (!_media_chain_map.count(ssrc)) 
		{
			dzlog_error("media chain donnot have %d", ssrc);
			return -1;
		}
 
		temp = _media_chain_map[ssrc];
	}

	connection client = temp->get_client()->get_connection();

	temp->set_server_send_port(server_send_port);
	next_mess->full_data_direct(tid, ssrc, length, width, client.show_ip_raw(), server_send_port);

	client.send_message(next_mess);

	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<respond_loadbalance_pull_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_peer(id, PEER_TYPE_LOADBALANCE)) 
	{
		dzlog_error("get loadbalance pulling respond from unknown loadbalance: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid; 
	std::vector<uint32_t> ips; 
	std::vector<uint16_t> ports;

	if (mess->give_me_data(tid, ips, ports)) return -1;
	
	{
		std::lock_guard<std::mutex> lk(_peer_map_lock[PEER_TYPE_LOADBALANCE]);

		for (unsigned int i = 0; i < ips.size(); ++i)
		{
			uint32_t ip		= ips[i];
			uint16_t port	= ports[i];

			if (ip == _self_ip && port == _self_port) continue;

			set_new_lb_map(ip, port, false);
		}
	}
	
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<respond_media_menu_pull_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_peer(id, PEER_TYPE_RESOURCE)) 
	{
		dzlog_error("get media menu from unknown resource: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	std::vector<std::string> video_names;
	uint32_t tid;

	mess->give_me_data(tid, video_names);

	{
		std::lock_guard<std::mutex> lk(_video_resources_lock);

		for (auto i = video_names.begin(); i != video_names.end(); ++i)
		{
			if (_video_resources.count(*i) && _video_resources[*i].count(id))
				continue;

			std::lock_guard<std::mutex> lk(_peer_map_lock[PEER_TYPE_RESOURCE]);
			_video_resources[*i][id] = _peer_map[PEER_TYPE_RESOURCE][id];
		}
	}

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<stop_stream_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_peer(id, PEER_TYPE_CLIENT)) 
	{
		dzlog_error("stop stream from a unknown client: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid; 
	uint32_t ssrc;

	mess->give_me_data(tid, ssrc);

	std::shared_ptr<media_chain> ptr;

	{
		std::lock_guard<std::mutex> lk(_media_chain_map_lock);
		if (!_media_chain_map.count(ssrc))
		{
			dzlog_error("ssrc %d not exist", ssrc);
			return -1;
		}

		ptr = _media_chain_map[ssrc];
		_media_chain_map.erase(ssrc);
	}

	dzlog_info("stop stream %d", ssrc);

	ptr->get_resource()->get_connection().send_message(mess);
	return 0;
}

int loadbalance::send_new_loadbalance_2_client_resource(const connection lb)
{
	std::vector<uint32_t> ip;
	std::vector<uint16_t> port;

	struct in_addr inp;
	inet_aton(lb.show_ip(), &inp);

	ip.push_back(inp.s_addr);
	port.push_back(lb.show_port());

	std::shared_ptr<push_loadbalance_pull_message> mess;
	mess->full_data_direct(get_tid(), ip, port);

	{
		std::lock_guard<std::mutex> lk(_peer_map_lock[PEER_TYPE_CLIENT]);
		for (auto i = _peer_map[PEER_TYPE_CLIENT].begin(); i != _peer_map[PEER_TYPE_CLIENT].end(); ++i)
		{
			dzlog_info("send new loadbalance(%s:%d) to client(%s:%d)", lb.show_ip(), lb.show_port(), i->second->get_connection().show_ip(), i->second->get_connection().show_port());

			i->second->get_connection().send_message(mess);
		}
	}

	{
		std::lock_guard<std::mutex> lk(_peer_map_lock[PEER_TYPE_RESOURCE]);
		for (auto i = _peer_map[PEER_TYPE_RESOURCE].begin(); i != _peer_map[PEER_TYPE_RESOURCE].end(); ++i)
		{
			dzlog_info("send new loadbalance(%s:%d) to client(%s:%d)", lb.show_ip(), lb.show_port(), i->second->get_connection().show_ip(), i->second->get_connection().show_port());

			i->second->get_connection().send_message(mess);
		}
	}

	return 0;
}

int loadbalance::send_media_menu_pulling(const connection resource)
{
	std::shared_ptr<pull_media_menu_message> mess;
	mess->full_data_direct(get_tid());
	resource.send_message(mess);

	return 0;
}

int loadbalance::check_video_and_find_resource_least_load(const std::string& video)
{
	std::lock_guard<std::mutex> lk(_video_resources_lock);
	if (_video_resources.count(video))
	{
		dzlog_error("not video: %s", video.c_str());
		return -2;
	}

	uint32_t load		= 0;
	uint32_t temp_load	= 0;
	int		resource_id = -1;

	const std::map<int, std::shared_ptr<peer>>& selected_map = _video_resources[video];
	for (auto i = selected_map.begin(); i != selected_map.end(); ++i)
	{
		temp_load = i->second->get_load();
		if (load > temp_load)
		{
			load = temp_load;
			resource_id = i->second->get_pid();
		}
	}

	return resource_id;
}

uint32_t loadbalance::find_resource_least_load()
{
	uint32_t load		= 0;
	uint32_t temp_load	= 0;
	uint32_t resource_id = -1;

	std::lock_guard<std::mutex> lk(_peer_map_lock[PEER_TYPE_RESOURCE]);
	for (auto i = _peer_map[PEER_TYPE_RESOURCE].begin(); i != _peer_map[PEER_TYPE_RESOURCE].end(); ++i)
	{
		temp_load = i->second->get_load();
		if (load > temp_load)
		{
			load = temp_load;
			resource_id = i->first;
		}
	}

	return resource_id;
}

uint32_t loadbalance::find_idle_ssrc()
{
	while (_media_chain_map.count(++_ssrc_boundary));
	return _ssrc_boundary;
}
 
int loadbalance::fresh_or_insert_peer(int id, int type, const connection conn, const uint32_t sid)
{
	std::lock_guard<std::mutex> lk(_peer_map_lock[type]);

	if (!_peer_map[type].count(id))
	{
		std::shared_ptr<peer> ptr = std::make_shared<peer>(conn, sid);
		_peer_map[type][id] = ptr;

		return 1;
	}

	_peer_map[type][id]->fresh();
	return 2;
}

int loadbalance::fresh_peer(int id, int type)
{
	std::lock_guard<std::mutex> lk(_peer_map_lock[type]);

	if (!_peer_map[type].count(id))
		return -1;

	_peer_map[type][id]->fresh();
	return 0;
}

int loadbalance::tell_me_type(uint32_t sid)
{
	return (sid & 0xff000000) >> 24;
}

void loadbalance::set_lb_map(uint32_t ip, uint16_t port, bool status)
{
	std::lock_guard<std::mutex> lk(_lb_ip_port_2_is_connect_lock);
	_lb_ip_port_2_is_connect[std::make_pair(ip, port)] = status;
}

void loadbalance::set_new_lb_map(uint32_t ip, uint16_t port, bool status)
{
	std::pair<uint32_t, uint16_t> key = std::make_pair(ip, port);

	std::lock_guard<std::mutex> lk(_lb_ip_port_2_is_connect_lock);

	if (!_lb_ip_port_2_is_connect.count(key)) return;

	_lb_ip_port_2_is_connect[key] = status;
}

void loadbalance::erase_lb_map(uint32_t ip, uint16_t port)
{
	std::pair<uint32_t, uint16_t> key = std::make_pair(ip, port);

	std::lock_guard<std::mutex> lk(_lb_ip_port_2_is_connect_lock);
	if (!_lb_ip_port_2_is_connect.count(key))
		return;
	
	_lb_ip_port_2_is_connect.erase(key);
}

void loadbalance::shoting_dead()
{
	std::shared_ptr<keepalive_message> mess;
	mess->full_data_direct(get_tid(), _sid, get_load());

	while (_status)
	{
		for (int i = 0; i < 3; ++i)
		{
			std::lock_guard<std::mutex> lk(_peer_map_lock[i]);

			for (auto j = _peer_map[i].begin(); j != _peer_map[i].end(); ++j)
			{
				std::shared_ptr<peer> p = j->second;
				if (!p->is_expires()) 
				{
					p->get_connection().send_message(mess);
					continue;
				}
					
				// 这里不清除其他资源, 因为死锁
				// 在其他地方查到没有的时候, 清除
				_peer_map[i].erase(j);
			}
		}
	}

	std::this_thread::sleep_for(std::chrono::seconds(60));
	swi_load();
}

loadbalance::loadbalance()
{
	_status = 0;

	struct ifaddrs* ifAddrStruct = nullptr ;
    struct ifaddrs* ifa	= nullptr ;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) 
	{
        if (!ifa->ifa_addr) 
			continue;

        if (ifa->ifa_addr->sa_family == AF_INET) 
            _self_ip = (uint32_t)((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
    }

    if (ifAddrStruct != NULL) 
		freeifaddrs(ifAddrStruct);
}

loadbalance::~loadbalance()
{
	net_message_listener::get_instance()->stop();
	_status = 0;
}
