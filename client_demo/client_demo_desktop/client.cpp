#include "client.h"
#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include "threadpool_instance.h"

void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> mess)
{
#ifdef PUSH_INTO_THREAD_POOL
#undef PUSH_INTO_THREAD_POOL
#endif 

#define PUSH_INTO_THREAD_POOL(type) threadpool_instance::get_instance()->schedule(std::bind(static_cast<int(client::*)(const connection, std::shared_ptr<type>)>(&client::deal_message), client::get_instance(), conn, std::dynamic_pointer_cast<type>(mess))); break;

	switch (message_type)
	{
	case MSGTYPE_KEEPALIVE:						PUSH_INTO_THREAD_POOL(keepalive_message)
	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:	PUSH_INTO_THREAD_POOL(loadbalance_respond_media_pull_message)
	case MSGTYPE_PUSHLOADBANLANCE:				PUSH_INTO_THREAD_POOL(push_loadbalance_pull_message)
	case MSGTYPE_RESOURCESERVERREPORT:			PUSH_INTO_THREAD_POOL(resource_server_report_message)
	case MSGTYPE_RESPONDLOADBALANCEPULL:		PUSH_INTO_THREAD_POOL(respond_loadbalance_pull_message)
	case MSGTYPE_RESPONDMEDIAMENU:				PUSH_INTO_THREAD_POOL(respond_media_menu_pull_message)
	case MSGTYPE_STOPSTREAM:					PUSH_INTO_THREAD_POOL(stop_stream_message)
	case MSGTYPE_PULLMEDIAMENU:					
	case MSGTYPE_PULLOTHERLOADBALANCE:
	case MSGTYPE_PUSHMEDIAMENU:
	case MSGTYPE_CLIENTPULLMEDIASTREAM:
	case MSGTYPE_RESOURCERESPONDMEDIAPULL:
	case MSGTYPE_STREAMFRAME:
	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM:
	case MSGTYPE_CLIENTSTREAMTRIGGER:
        dzlog_error("recv a message should not send to client: %d", message_type);
		mess->print_data();
		return;

	default: 
		dzlog_error("recv a message of unknown type: %d", message_type);
		return;
	}

	client::get_instance()->inc_load();

#undef PUSH_INTO_THREAD_POOL
}

client* client::get_instance()
{
	static client instance;
	return &instance;
}

void client::play(const std::string& video)
{
    dzlog_info("trying to play %s", video.c_str());

	std::shared_ptr<media_core_message::client_pull_media_stream_message> mess = 
		std::make_shared<media_core_message::client_pull_media_stream_message>();

	uint32_t tid = get_tid();

	{
		std::lock_guard<std::mutex> lk(_tid_2_media_lock);
		_tid_2_media[tid] = video;
	}

	mess->full_data_direct(tid, video, 5455);

	std::shared_ptr<connection> selected_conn;
	int currnet_load	= 0;
	int min_load		= INT_MAX;

	{
		std::lock_guard<std::mutex> lk(_loadbalance_map_lock);
		for (auto i = _loadbalance_map.begin(); i != _loadbalance_map.end(); ++i)
		{
			currnet_load = i->second->get_load();
			if (currnet_load > min_load) continue;

			min_load = currnet_load;
			selected_conn = std::make_shared<connection>(i->second->get_connection());
		}
	}

	selected_conn->send_message(mess);
	dzlog_info("pulling %s to %s:%d", video.c_str(), selected_conn->show_ip(), selected_conn->show_port());
}

int client::listening(uint16_t port, uint32_t sid)
{
	_status		= 1;
	_self_port	= port;
	_sid		= sid;

	net_message_listener::get_instance()->set_callback(net_message_listener_callback);
	net_message_listener::get_instance()->listening(port);

	std::thread* p = new std::thread(&client::rolling, this);
	p->detach();
	delete p;

	return 0;
}

void client::set_update_videos_callback(int (*video_update_callback)(std::shared_ptr<std::vector<std::string>>))
{
	_video_update_callback = video_update_callback;
}

void client::stop()
{
	_status = 0;
}

void client::rolling()
{
	while (_status)
	{
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance_ability::shoting_loadbalance,		this));
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance_ability::rolling_loadbalance_map,	this));
		threadpool_instance::get_instance()->schedule(std::bind(&client::rolling_pulling_videos,				this));

		std::this_thread::sleep_for(std::chrono::seconds(8));
		swi_load();
	}

	dzlog_info("rolling end");
}

void client::rolling_pulling_videos()
{
	std::shared_ptr<media_core_message::pull_media_menu_message> mess = 
		std::make_shared<media_core_message::pull_media_menu_message>();

	mess->full_data_direct(get_tid());

	std::lock_guard<std::mutex> lk(_loadbalance_map_lock);
	for (auto i = _loadbalance_map.begin(); i != _loadbalance_map.end(); ++i)
	{
		connection conn = i->second->get_connection();
		conn.send_message(mess);

		dzlog_info("pulling media menu to %s:%d", conn.show_ip(), conn.show_port());
	}
}

client::client()
{
	_video_update_callback	= nullptr;
	_status					= 0;

    SDL_Init(SDL_INIT_EVERYTHING);
}

client::~client()
{
	_status = 0;
    SDL_Quit();
}

int client::deal_message(const connection conn, std::shared_ptr<keepalive_message> mess)
{
	uint32_t	tid, sid; 
	uint16_t	listening_port; 
	uint8_t		count;

	mess->give_me_data(tid, sid, listening_port, count);

	int type = tell_me_type(sid);
	if (type != PEER_TYPE_LOADBALANCE)
	{
		dzlog_error("recv a keepalive not from lb but a %d, %s:%d", type, conn.show_ip(), conn.show_port());
		return -1;
	}

	// 0 代表不是新的 lb
	if (0 == fresh_or_insert_loadbalance(conn, sid, listening_port, count))
		return 0;

	// 新的需要建立映射 ip:port -> is_connect:sid
	set_loadbalance_map(sid, conn.show_ip_raw(), conn.show_port_raw(), true);

	std::shared_ptr<pull_other_loadbalance_message> lb_mess = std::make_shared<pull_other_loadbalance_message>();
	lb_mess->full_data_direct(get_tid());
	conn.send_message(lb_mess);

	return 0;
}

int client::deal_message(const connection conn, std::shared_ptr<loadbalance_respond_media_pull_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv media pull respond from a unknown lb %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid, ssrc, length, width, ip; 
	uint16_t server_send_port;

	mess->give_me_data(tid, ssrc, length, width, ip, server_send_port);

	std::string video;

	{
		std::lock_guard<std::mutex> lk(_tid_2_media_lock);
		if (!_tid_2_media.count(tid))
		{
			dzlog_error("video for tid %d not found", tid);
			return -1;
		}
			
		video = _tid_2_media[tid];
	}
	
	std::shared_ptr<stream_render> mediastream = std::make_shared<stream_render>(video, length, width);

	{
		std::lock_guard<std::mutex> lk(_mediastreams_lock);
		if (_mediastreams.count(ssrc))
		{
			dzlog_error("ssrc %d already exist", ssrc);
			return -1;
		}

		_mediastreams[ssrc] = mediastream;
	}

	mediastream->triggering(tid, ssrc, ip, server_send_port);

	char string_ip[16] = { 0 };
	snprintf(string_ip, 16, "%u.%u.%u.%u", (ip & 0x000000ff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);

	dzlog_info("create media chain %d@%s:%d", ssrc, string_ip, ntohs(server_send_port));
	return 0;
}

int client::deal_message(const connection conn, std::shared_ptr<push_loadbalance_pull_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv loadbalance push from unknown loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid; 
	std::vector<uint32_t> sids; 
	std::vector<uint32_t> ips; 
	std::vector<uint16_t> ports;

	mess->give_me_data(tid, sids, ips, ports);
	set_new_loadbalance_map(sids, ips, ports, false);

	return 0;
}

int client::deal_message(const connection conn, std::shared_ptr<resource_server_report_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv resource server report from unknow loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t	tid; 
	uint8_t		error;

	mess->give_me_data(tid, error);
	mess->print_data();
	return 0;
}

int client::deal_message(const connection conn, std::shared_ptr<respond_loadbalance_pull_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv loadbalance push from unknown loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid; 
	std::vector<uint32_t> sids; 
	std::vector<uint32_t> ips; 
	std::vector<uint16_t> ports;

	mess->give_me_data(tid, sids, ips, ports);
	set_new_loadbalance_map(sids, ips, ports, false);

	return 0;
}

int client::deal_message(const connection conn, std::shared_ptr<respond_media_menu_pull_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv media menu pull respond from a unknown loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	std::shared_ptr<std::vector<std::string>> update_videos = std::make_shared<std::vector<std::string>>();

	uint32_t tid; 
	std::vector<std::string> video_names;
	mess->give_me_data(tid, video_names);

	{
		std::lock_guard<std::mutex> lk(_videos_lock);
		for (auto i = video_names.begin(); i != video_names.end(); ++i)
		{
			if (_videos.count(*i)) continue;
			_videos[*i] = 1;
			update_videos->push_back(*i);
		}
	}

    dzlog_info("callback to set videos");
	_video_update_callback(update_videos);

	mess->print_data();
	return 0;
}

int client::deal_message(const connection conn, std::shared_ptr<stop_stream_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv stop stream from a unknow loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t tid, ssrc;
	mess->give_me_data(tid, ssrc);

	std::lock_guard<std::mutex> lk(_mediastreams_lock);
	if (!_mediastreams.count(ssrc))
	{
		dzlog_error("there is no ssrc: %d", ssrc);
		return -1;
	}

	_mediastreams[ssrc]->stop();
	_mediastreams.erase(ssrc);

	return 0;
}
