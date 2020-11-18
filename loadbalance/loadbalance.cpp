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

	_self_port	= port;
	_status		= 1;
	_sid		= sid;

	std::thread* p = new std::thread(&loadbalance::shoting_dead, this);
	p->detach();
	delete p;

	p = new std::thread(&server::rolling_map, this);
	p->detach();
	delete p;

	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<client_pull_media_stream_message> mess)
{
	const int sock = conn.show_sockfd();
	if (fresh_client(sock))
	{
		dzlog_error("pull stream from a unknown client %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	std::string video_2_play;
	uint16_t recv_port	= 0;
	uint32_t tid		= 0;

	mess->give_me_data(tid, video_2_play, recv_port);

	uint32_t ssrc		= find_idle_ssrc();
	uint32_t client_sid	= sockfd_2_sid(sock);

	int	selected_resource_sid = check_video_and_find_resource_least_load(video_2_play);	// 这个是 sid
	if (selected_resource_sid == -1)
	{
		dzlog_error("failed to alloc resource server");
		return -1;
	}
	else if (selected_resource_sid == -2)
	{
		dzlog_error("video: %s not exist", video_2_play.c_str());

		std::shared_ptr<resource_server_report_message> next_mess = std::make_shared<resource_server_report_message>();
		next_mess->full_data_direct(tid, 1);
		conn.send_message(next_mess);

		return -1;
	}

	if (insert_id_tid_2_ssrc(INDEX_SERVER, selected_resource_sid, tid, ssrc)) return -1;
	if (insert_id_tid_2_ssrc(INDEX_CLIENT, client_sid, tid, ssrc))
	{
		remove_id_tid_2_ssrc(INDEX_SERVER, selected_resource_sid, tid);
		return -1;
	}

	std::shared_ptr<media_chain> ptr;

	{
		std::lock(_client_map_lock, _resource_map_lock);

		// 函数开始时候 fresh 的时候没有失败, 所以这里 id 是必然存在的
		ptr = std::make_shared<media_chain>(ssrc, _client_map[client_sid], _resource_map[selected_resource_sid], video_2_play);

		std::shared_ptr<loadbalance_pull_media_stream_message> next_mess = std::make_shared<loadbalance_pull_media_stream_message>();
		next_mess->full_data_direct(tid, recv_port, ssrc, video_2_play);

		_resource_map[selected_resource_sid]->send_message(next_mess);
	}

	ptr->set_client_recv_port(recv_port);

	{
		std::lock_guard<std::mutex> lk(_media_chain_map_lock);
		_media_chain_map[ssrc] = ptr;
	}

	dzlog_info("send pull %s stream %s:%d -> %s:? tid(%d)->ssrc(%d)", video_2_play.c_str(), conn.show_ip(), recv_port, _resource_map[selected_resource_sid]->get_connection().show_ip(), tid, ssrc);
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<keepalive_message> mess)
{
	const int sock = conn.show_sockfd();

	uint32_t tid, sid; 
	uint16_t listening_port;
	uint8_t	count;
	int	ret;

	mess->give_me_data(tid, sid, listening_port, count);

	int type = tell_me_type(sid);
	if (type != PEER_TYPE_CLIENT && type != PEER_TYPE_LOADBALANCE && type != PEER_TYPE_RESOURCE)
	{
		dzlog_error("get a unknown type: %d", type);
		return -1;
	}

	switch (type)
	{
	// 新注册的负载均衡应该发给所有的客户端和视频资源
	case PEER_TYPE_LOADBALANCE:
		ret = fresh_or_insert_loadbalance(sock, conn, sid, listening_port, count);
		if (0 == ret)
		{
			dzlog_info("%s:%d keepalive", conn.show_ip(), conn.show_port());
			return 0;
		}

		set_loadbalance_map(sid, conn, true);

		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::send_new_loadbalance_2_client_resource, this, sid, conn));
		dzlog_info("get a new loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return 0;

	// 新注册的视频资源应该拉取视频目录
	case PEER_TYPE_RESOURCE:		
		ret = fresh_or_insert_resource(sock, conn, sid, listening_port, count);
		if (0 == ret)
		{
			dzlog_info("%s:%d keepalive", conn.show_ip(), conn.show_port());
			return 0;
		}

		set_resource_map(sid, conn, true);

		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::send_media_menu_pulling, this, conn));
		dzlog_info("get a new resource %s:%d", conn.show_ip(), conn.show_port());
		return 0;

	// 新注册的客户端不需要做额外的事
	case PEER_TYPE_CLIENT:			
		ret = fresh_or_insert_client(sock, conn, sid, listening_port, count);
		if (0 == ret)
		{
			dzlog_info("%s:%d keepalive", conn.show_ip(), conn.show_port());
			return 0;
		}

		set_client_map(sid, conn, true);
		
		dzlog_info("get a new client %s:%d", conn.show_ip(), conn.show_port());
		return 0;

	default:
		dzlog_error("something never happen");
		return -1;
	}
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<pull_other_loadbalance_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_client(id) && fresh_loadbalance(id) && fresh_resource(id)) 
	{
		dzlog_error("get pull_other_loadbalance_message from unknown %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid;
	mess->give_me_data(tid);

	std::shared_ptr<respond_loadbalance_pull_message> next_mess = std::make_shared<respond_loadbalance_pull_message>();

	std::vector<uint32_t> sids;
	std::vector<uint32_t> ips;
	std::vector<uint16_t> ports;

	{
		std::lock_guard<std::mutex> lk(_loadbalance_map_lock);

		for (auto i = _loadbalance_map.begin(); i != _loadbalance_map.end(); ++i)
		{
			std::shared_ptr<peer> temp = i->second;

			sids.push_back(temp->get_sid());
			ips.push_back(temp->get_connection().show_ip_raw());
			ports.push_back(temp->get_listening_port());
		}
	}

	next_mess->full_data_direct(tid, sids, ips, ports);
	conn.send_message(next_mess);

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<push_media_pull_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_resource(id))
	{
		dzlog_error("push media menu from unknown resource: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	std::vector<std::string> video_names;
	uint32_t sid = sockfd_2_sid(id);
	uint32_t tid;

	mess->give_me_data(tid, video_names);

	{
		std::lock_guard<std::mutex> lk(_video_resources_lock);

		for (auto i = video_names.begin(); i != video_names.end(); ++i)
		{
			if (_video_resources.count(*i) && _video_resources[*i].count(id))
				continue;

			std::lock_guard<std::mutex> lk(_resource_map_lock);
			_video_resources[*i][id] = _resource_map[sid];
		}
	}

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<resource_server_report_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_resource(id))
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
	if (fresh_resource(id)) 
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
	const int sock = conn.show_sockfd();
	if (fresh_loadbalance(sock)) 
	{
		dzlog_error("get loadbalance pulling respond from unknown loadbalance: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid; 
	std::vector<uint32_t> sids;
	std::vector<uint32_t> ips; 
	std::vector<uint16_t> ports;

	if (mess->give_me_data(tid, sids, ips, ports)) return -1;
	
	{
		std::lock_guard<std::mutex> lk(_loadbalance_map_lock);

		for (unsigned int i = 0; i < ips.size(); ++i)
		{
			set_new_loadbalance_map(sids[i], conn, false);
		}
	}
	
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<respond_media_menu_pull_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_resource(id)) 
	{
		dzlog_error("get media menu from unknown resource: %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	std::vector<std::string> video_names;
	uint32_t tid;
	uint32_t sid = sockfd_2_sid(id);

	mess->give_me_data(tid, video_names);

	{
		std::lock_guard<std::mutex> lk(_video_resources_lock);

		for (auto i = video_names.begin(); i != video_names.end(); ++i)
		{
			if (_video_resources.count(*i) && _video_resources[*i].count(id))
				continue;

			std::lock_guard<std::mutex> lk(_resource_map_lock);
			_video_resources[*i][id] = _resource_map[sid];
		}
	}

	mess->print_data();
	return 0;
}

int loadbalance::deal_message(const connection conn, std::shared_ptr<stop_stream_message> mess)
{
	const int id = conn.show_sockfd();
	if (fresh_client(id)) 
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

int loadbalance::send_new_loadbalance_2_client_resource(uint32_t sid, const connection lb)
{
	std::vector<uint32_t> sids;
	std::vector<uint32_t> ip;
	std::vector<uint16_t> port;

	struct in_addr inp;
	inet_aton(lb.show_ip(), &inp);

	sids.push_back(sid);
	ip.push_back(inp.s_addr);
	port.push_back(lb.show_port());

	std::shared_ptr<push_loadbalance_pull_message> mess = std::make_shared<push_loadbalance_pull_message>();
	mess->full_data_direct(get_tid(), sids, ip, port);

	{
		std::lock_guard<std::mutex> lk(_client_map_lock);
		for (auto i = _client_map.begin(); i != _client_map.end(); ++i)
		{
			dzlog_info("send new loadbalance(%s:%d) to client(%s:%d)", lb.show_ip(), lb.show_port(), i->second->get_connection().show_ip(), i->second->get_connection().show_port());

			i->second->get_connection().send_message(mess);
		}
	}

	{
		std::lock_guard<std::mutex> lk(_resource_map_lock);
		for (auto i = _resource_map.begin(); i != _resource_map.end(); ++i)
		{
			dzlog_info("send new loadbalance(%s:%d) to client(%s:%d)", lb.show_ip(), lb.show_port(), i->second->get_connection().show_ip(), i->second->get_connection().show_port());

			i->second->get_connection().send_message(mess);
		}
	}

	return 0;
}

int loadbalance::send_media_menu_pulling(const connection resource)
{
	std::shared_ptr<pull_media_menu_message> mess = std::make_shared<pull_media_menu_message>();
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
	int	resource_id		= -1;

	const std::map<int, std::shared_ptr<peer>>& selected_map = _video_resources[video];
	for (auto i = selected_map.begin(); i != selected_map.end(); ++i)
	{
		temp_load = i->second->get_load();
		if (load > temp_load)
		{
			load = temp_load;
			resource_id = i->second->get_sid();
		}
	}

	return resource_id;
}

uint32_t loadbalance::find_resource_least_load()
{
	uint32_t load		= 0;
	uint32_t temp_load	= 0;
	uint32_t resource_id = -1;

	std::lock_guard<std::mutex> lk(_resource_map_lock);
	for (auto i = _resource_map.begin(); i != _resource_map.end(); ++i)
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
 
int loadbalance::fresh_or_insert_client(int sock, const connection conn, const uint32_t sid, const uint16_t listening_port, const uint32_t load)
{
	std::lock_guard<std::mutex> lk(_client_map_lock);

	if (_client_map.count(sid))
	{
		dzlog_info("fresh socket %d@%s:%d", sock, conn.show_ip(), conn.show_port());

		_client_map[sid]->fresh();
		_client_map[sid]->set_load(load);
		return 0;
	}

	if (insert_sock_sid(sock, sid))
	{
		// 这里就算是失败也不要返回
		dzlog_info("mapping %d->%d already exist", sock, sid);
	}

	std::shared_ptr<peer> p = std::make_shared<peer>(conn, sid, listening_port);
	p->set_load(load);
	_client_map[sid] = p;

	return 1;
}

int loadbalance::fresh_client(int sock)
{
	{
		std::lock_guard<std::mutex> lk(_client_map_lock);

		uint32_t sid = sockfd_2_sid(sock);
		if (!_client_map.count(sid))
		{
//			dzlog_error("fresh client %d not exist", sock);
			return -1;
		}

		_client_map[sid]->fresh();
	}

	dzlog_info("fresh socket %d", sock);
	return 0;
}

int loadbalance::fresh_or_insert_loadbalance(int sock, const connection conn, const uint32_t sid, const uint16_t listening_port, const uint32_t load)
{
	std::lock_guard<std::mutex> lk(_loadbalance_map_lock);

	if (_loadbalance_map.count(sid))
	{
		dzlog_info("fresh socket %d@%s:%d", sock, conn.show_ip(), conn.show_port());

		_loadbalance_map[sid]->fresh();
		_loadbalance_map[sid]->set_load(load);
		return 0;
	}

	if (insert_sock_sid(sock, sid))
	{
		// 这里就算是失败也不要返回
		dzlog_info("mapping %d->%d already exist", sock, sid);
	}

	std::shared_ptr<peer> p = std::make_shared<peer>(conn, sid, listening_port);
	p->set_load(load);
	_loadbalance_map[sid] = p;

	return 1;
}

int loadbalance::fresh_loadbalance(int sock)
{
	uint32_t sid = sockfd_2_sid(sock);

	{
		std::lock_guard<std::mutex> lk(_loadbalance_map_lock);

		if (!_loadbalance_map.count(sid))
		{
			return -1;
		}
	}

	_loadbalance_map[sid]->fresh();
	return 0;
}

int loadbalance::fresh_or_insert_resource(int sock, const connection conn, const uint32_t sid, const uint16_t listening_port, const uint32_t load)
{
	std::lock_guard<std::mutex> lk(_resource_map_lock);

	if (_resource_map.count(sid))
	{
		dzlog_info("fresh socket %d@%s:%d", sock, conn.show_ip(), conn.show_port());

		_resource_map[sid]->fresh();
		_resource_map[sid]->set_load(load);
		return 0;
	}

	if (1 == insert_sock_sid(sock, sid))
	{
		// 这里就算是失败也不要返回
		dzlog_info("mapping %d->%d already exist", sock, sid);
	}

	std::shared_ptr<peer> p = std::make_shared<peer>(conn, sid, listening_port);
	p->set_load(load);
	_resource_map[sid] = p;

	return 1;
}

int loadbalance::fresh_resource(int sock)
{
	uint32_t sid = sockfd_2_sid(sock);

	{
		std::lock_guard<std::mutex> lk(_resource_map_lock);

		if (!_resource_map.count(sid))
		{
			dzlog_error("fresh resource %d not exist", sock);
			return -1;
		}
	}

	_resource_map[sid]->fresh();
	return 0;
}

void loadbalance::shoting_dead()
{
	while (_status)
	{
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::shoting_client, this));
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::shoting_loadbalance, this));
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::shoting_resource, this));
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance::shoting_media_chain, this));

		std::this_thread::sleep_for(std::chrono::seconds(5));
		swi_load();
	}
}

void loadbalance::shoting_client()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, _self_port, get_load());

	std::lock_guard<std::mutex> lk(_client_map_lock);

	dzlog_info("shoting %ld client", _client_map.size());

	for (auto i = _client_map.begin(); i != _client_map.end(); )
	{
		std::shared_ptr<peer> p = i->second;

		if (p->is_expires()) 
		{
			dzlog_info("peer %s:%d expires", p->get_connection().show_ip(), p->get_connection().show_port());

			// 这里不清除其他资源, 因为死锁, 在其他地方查到没有的时候, 清除
			_client_map.erase(i++);
		}
		else
		{
			p->get_connection().send_message(mess);
			++i;
		}
	}
}

void loadbalance::shoting_loadbalance()
{
	std::lock_guard<std::mutex> lk(_loadbalance_map_lock);

	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, _self_port, get_load());

	std::shared_ptr<pull_other_loadbalance_message> lb_mess = std::make_shared<pull_other_loadbalance_message>();
	lb_mess->full_data_direct(get_tid());

	dzlog_info("shoting %ld loadbalance", _loadbalance_map.size());

	for (auto i = _loadbalance_map.begin(); i != _loadbalance_map.end(); )
	{
		std::shared_ptr<peer> p = i->second;

		if (p->is_expires()) 
		{
			dzlog_info("peer %s:%d expires", p->get_connection().show_ip(), p->get_connection().show_port());

			// 这里不清除其他资源, 因为死锁 在其他地方查到没有的时候, 清除
			_loadbalance_map.erase(i++);
		}
		else
		{
			p->get_connection().send_message(mess);
			p->get_connection().send_message(lb_mess);

			++i;
		}
	}
}

void loadbalance::shoting_resource()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, _self_port, get_load());

	std::lock_guard<std::mutex> lk(_resource_map_lock);
	dzlog_info("shoting %ld resource", _resource_map.size());

	for (auto i = _resource_map.begin(); i != _resource_map.end(); )
	{
		std::shared_ptr<peer> p = i->second;
		if (p->is_expires()) 
		{
			dzlog_info("peer %s:%d expires", p->get_connection().show_ip(), p->get_connection().show_port());

			// 这里不清除其他资源, 因为死锁 在其他地方查到没有的时候, 清除
			_resource_map.erase(i++);
		}
		else
		{
			p->get_connection().send_message(mess);
			++i;
		}
	}
}

void loadbalance::shoting_media_chain()
{
	std::lock_guard<std::mutex> lk(_media_chain_map_lock);
	for (auto i = _media_chain_map.begin(); i != _media_chain_map.end(); ++i)
	{
		if (i->second->is_expires())
		{
			dzlog_info("media chain %d expires", i->second->get_ssrc());
			_media_chain_map.erase(i);
		}
	}
}

loadbalance::loadbalance()
{
}

loadbalance::~loadbalance()
{
	net_message_listener::get_instance()->stop();
	_status = 0;
}
