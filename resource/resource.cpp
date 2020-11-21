#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "resource.h"
#include <sys/types.h>
#include "stream_pusher_err.h"

void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> mess)
{
#ifdef PUSH_INTO_THREAD_POOL
#undef PUSH_INTO_THREAD_POOL
#endif 

#define PUSH_INTO_THREAD_POOL(type) threadpool_instance::get_instance()->schedule(std::bind(static_cast<int(resource_server::*)(const connection, std::shared_ptr<type>)>(&resource_server::deal_message), resource_server::get_instance(), conn, std::dynamic_pointer_cast<type>(mess))); break;

	switch (message_type)
	{
	case MSGTYPE_KEEPALIVE:						PUSH_INTO_THREAD_POOL(keepalive_message)
	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM:	PUSH_INTO_THREAD_POOL(loadbalance_pull_media_stream_message)
	case MSGTYPE_PULLMEDIAMENU:					PUSH_INTO_THREAD_POOL(pull_media_menu_message)
	case MSGTYPE_PUSHLOADBANLANCE:				PUSH_INTO_THREAD_POOL(push_loadbalance_pull_message)
	case MSGTYPE_RESPONDLOADBALANCEPULL:		PUSH_INTO_THREAD_POOL(respond_loadbalance_pull_message);
	case MSGTYPE_STOPSTREAM:					PUSH_INTO_THREAD_POOL(stop_stream_message)
	case MSGTYPE_CLIENTSTREAMTRIGGER:
	case MSGTYPE_RESPONDMEDIAMENU:
	case MSGTYPE_PULLOTHERLOADBALANCE:			
	case MSGTYPE_PUSHMEDIAMENU:	
	case MSGTYPE_CLIENTPULLMEDIASTREAM:
	case MSGTYPE_RESOURCERESPONDMEDIAPULL:
	case MSGTYPE_RESOURCESERVERREPORT:
	case MSGTYPE_STREAMFRAME:
	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:
		dzlog_error("recv a message should not send to loadbalance: %d", message_type);
		mess->print_data();
		return;

	default: 
		dzlog_error("recv a message of unknown type: %d", message_type);
		return;
	}

	resource_server::get_instance()->inc_load();

#undef PUSH_INTO_THREAD_POOL
}

void streaming_end_callback(uint32_t ssrc)
{
	dzlog_info("%d end streaming", ssrc);
	resource_server::get_instance()->send_streaming_end(ssrc);
}

resource_server* resource_server::get_instance()
{
	static resource_server instance;
	return &instance;
}

void resource_server::set_video_path(const std::string& video_path)
{
	_videos_path = video_path;
}

int resource_server::listening(uint16_t port, uint32_t sid)
{
	_self_port	= port;
	_sid		= sid;

	net_message_listener::get_instance()->set_callback(net_message_listener_callback);
	net_message_listener::get_instance()->listening(port);

	_status = 1;

	std::thread* p = new std::thread(&resource_server::rolling, this);
	p->detach();
	delete p;

	return 0;
}

void resource_server::rolling()
{
	while (_status)
	{
		threadpool_instance::get_instance()->schedule(std::bind(&resource_server::rolling_videos,				this));
		threadpool_instance::get_instance()->schedule(std::bind(&resource_server::rolling_mediapushing_expires, this));
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance_ability::rolling_loadbalance_map,	this));
		threadpool_instance::get_instance()->schedule(std::bind(&loadbalance_ability::shoting_loadbalance,		this));

		std::this_thread::sleep_for(std::chrono::seconds(3));
		swi_load();
	}

	dzlog_info("rolling end");
}

void resource_server::rolling_videos()
{
	struct dirent** namelist;

	int n = scandir(_videos_path.c_str(), &namelist, NULL, alphasort);
	if (n == -1) 
	{
		dzlog_error("scan dir failed %d", errno);
		return;
	}

	std::shared_ptr<std::vector<std::string>> new_videos = std::make_shared<std::vector<std::string>>();

	std::string temp;

	std::lock_guard<std::mutex> lk(_video_mutex);

	while (n--) 
	{
		bool is_new = true;

		temp = namelist[n]->d_name;
		for (auto i = _videos.begin(); i != _videos.end(); ++i)
		{
			if (temp == *i || temp[0] == '.')
			{
				is_new = false;
				break;
			}
		}

		if (is_new) 
		{
			dzlog_info("scan new video: %s", temp.c_str());
			_videos.push_back(temp);
			new_videos->push_back(temp);

			threadpool_instance::get_instance()->schedule(std::bind(&resource_server::send_new_video_to_all_lb, this, new_videos));
		}

		free(namelist[n]);
	}

	free(namelist);
}

void resource_server::rolling_mediapushing_expires()
{
	std::shared_ptr<std::vector<uint32_t>> expires_ssrc = std::make_shared<std::vector<uint32_t>>();

	std::lock_guard<std::mutex> lk(_mediastreams_lock);
	for (auto i = _mediastreams.begin(); i != _mediastreams.end(); ++i)
	{
		if (i->second->is_expires())
		{
			dzlog_error("media chain %d expires", i->first);
			expires_ssrc->push_back(i->first);
		}
	}

	threadpool_instance::get_instance()->schedule(std::bind(&resource_server::send_media_chains_expires, this, expires_ssrc));
}

void resource_server::send_new_video_to_all_lb(std::shared_ptr<std::vector<std::string>> new_videos)
{
	std::shared_ptr<media_core_message::push_media_pull_message> mess = 
		std::make_shared<push_media_pull_message>();
	mess->full_data_direct(get_tid(), *new_videos);

	std::lock_guard<std::mutex> lk(_loadbalance_map_lock);
	for (auto i = _loadbalance_map.begin(); i != _loadbalance_map.end(); ++i)
	{
		connection conn = i->second->get_connection();
		conn.send_message(mess);
		dzlog_info("push video menu to %s:%d", conn.show_ip(), conn.show_port());
	}
}

void resource_server::send_media_chains_expires(std::shared_ptr<std::vector<uint32_t>> expires_ssrc)
{
	std::vector<uint32_t> lbfds;
	std::vector<uint32_t> lbsids;

	std::shared_ptr<media_core_message::resource_server_report_message> mess = 
		std::make_shared<media_core_message::resource_server_report_message>();
	mess->full_data_direct(get_tid(), 5);

	{
		std::lock_guard<std::mutex> lk(_ssrc_2_lbfd_lock);

		for (auto i = expires_ssrc->begin(); i != expires_ssrc->end(); ++i)
		{
			if (!_ssrc_2_lbfd.count(*i))
			{
				dzlog_error("cannot find lb of ssrc %d", *i);
				continue;
			}

			lbfds.push_back(_ssrc_2_lbfd[*i]);
		}
	}

	loadbalance_sockfd_2_sid(lbsids, lbfds);

	{
		std::lock_guard<std::mutex> lk(_loadbalance_map_lock);
		
		for (auto i = lbsids.begin(); i != lbsids.end(); ++i)
		{
			if (!_loadbalance_map.count(*i))
			{
				dzlog_error("lb %d not exist", *i);
				continue;
			}

			_loadbalance_map[*i]->get_connection().send_message(mess);
		}
	}
}

void resource_server::send_streaming_end(uint32_t ssrc)
{
	std::shared_ptr<media_core_message::stop_stream_message> mess = 
		std::make_shared<media_core_message::stop_stream_message>();

	mess->full_data_direct(get_tid(), ssrc);

	uint32_t lbfd;
	
	{
		std::lock_guard<std::mutex> lk(_ssrc_2_lbfd_lock);
		if (!_ssrc_2_lbfd.count(ssrc))
		{
			dzlog_error("lb for ssrc: %d not exist", ssrc);
			return;
		}

		lbfd = _ssrc_2_lbfd[ssrc];
	}

	int lbsid = loadbalance_sockfd_2_sid(lbfd);
	if (lbsid == -1)
	{
		dzlog_error("lb for %d not exist", lbfd);
		return;
	}

	{
		std::lock_guard<std::mutex> lk(_loadbalance_map_lock);
		if (!_loadbalance_map.count(lbsid))
		{
			dzlog_error("cannot find lb by sid %d", lbsid);
			return;
		}

		_loadbalance_map[lbsid]->get_connection().send_message(mess);
	}
}

resource_server::resource_server()
{
}

resource_server::~resource_server()
{
	_status = 0;

	net_message_listener::get_instance()->stop();

	std::lock_guard<std::mutex> lk(_mediastreams_lock);
	for (auto i = _mediastreams.begin(); i != _mediastreams.end(); ++i)
	{
		i->second->stop();
	}
}

int resource_server::deal_message(const connection conn, std::shared_ptr<keepalive_message> mess)
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

int resource_server::deal_message(const connection conn, std::shared_ptr<loadbalance_pull_media_stream_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv media pull message from a known loadbalance %s:%d", conn.show_ip(), conn.show_port());
		return -1;
	}

	uint32_t	tid, ssrc; 
	uint32_t	w, h;
	uint16_t	client_recv_port; 
	std::string video_name;

	mess->give_me_data(tid, client_recv_port, ssrc, video_name);

	std::shared_ptr<stream_pusher> mediastream = 
		std::make_shared<stream_pusher>();

	int ret = mediastream->set_video(tid, ssrc, video_name, streaming_end_callback);
	if (ERR_CANNOT_OPEN_VIDEO == ret)
	{
		std::shared_ptr<media_core_message::resource_server_report_message> next_mess = 
			std::make_shared<media_core_message::resource_server_report_message>();
		next_mess->full_data_direct(get_tid(), 1);

		conn.send_message(next_mess);
		return -1;
	}
	else if (0 != ret)
	{
		std::shared_ptr<media_core_message::resource_server_report_message> next_mess = 
			std::make_shared<media_core_message::resource_server_report_message>();
		next_mess->full_data_direct(get_tid(), 7);

		conn.send_message(next_mess);
		return -1;
	}

	uint16_t sending_port;

	mediastream->tell_me_size(w, h);
	mediastream->listening(sending_port);

	bool duplicate_ssrc = false;

	{
		std::lock_guard<std::mutex> lk(_mediastreams_lock);
		if (!_mediastreams.count(ssrc))
			_mediastreams[ssrc] = mediastream;
		else
			duplicate_ssrc = true;
	}

	if (duplicate_ssrc)
	{
		std::shared_ptr<media_core_message::resource_server_report_message> next_mess = 
			std::make_shared<media_core_message::resource_server_report_message>();

		next_mess->full_data_direct(tid, 8);
		conn.send_message(next_mess);
	}
	else
	{
		std::shared_ptr<media_core_message::resource_server_respond_media_pull_message> next_mess =
			std::make_shared<media_core_message::resource_server_respond_media_pull_message>();

		next_mess->full_data_direct(tid, w, h, sending_port);
		conn.send_message(next_mess);
	}

	return 0;
}

int resource_server::deal_message(const connection conn, std::shared_ptr<pull_media_menu_message> mess)
{
	fresh_loadbalance(conn.show_sockfd());

	uint32_t tid;
	mess->give_me_data(tid);

	std::shared_ptr<media_core_message::respond_media_menu_pull_message> next_mess =
		std::make_shared<media_core_message::respond_media_menu_pull_message>();

	{
		std::lock_guard<std::mutex> lk(_video_mutex);
		next_mess->full_data_direct(tid, _videos);
	}

	conn.send_message(next_mess);
	return 0;
}

int resource_server::deal_message(const connection conn, std::shared_ptr<push_loadbalance_pull_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv media pull message from a known loadbalance %s:%d", conn.show_ip(), conn.show_port());
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

int resource_server::deal_message(const connection conn, std::shared_ptr<respond_loadbalance_pull_message> mess)
{
	if (fresh_loadbalance(conn.show_sockfd()))
	{
		dzlog_error("recv media pull message from a known loadbalance %s:%d", conn.show_ip(), conn.show_port());
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

int resource_server::deal_message(const connection conn, std::shared_ptr<stop_stream_message> mess)
{
	uint32_t tid; 
	uint32_t ssrc;

	mess->give_me_data(tid, ssrc);

	std::lock_guard<std::mutex> lk(_mediastreams_lock);
	if (_mediastreams.count(ssrc))
	{
		dzlog_error("cannot find ssrc %d", ssrc);
		return -1;
	}

	_mediastreams[ssrc]->stop();
	_mediastreams.erase(ssrc);
	return 0;
}
