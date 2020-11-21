#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <list>
#include <string>
#include <vector>
#include <stdint.h>
#include "stream_pusher.h"
#include "loadbalance_ability.h"
#include "threadpool_instance.h"
#include "net_message_listener.h"

void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> mess);
void streaming_end_callback(uint32_t ssrc);

class resource_server : virtual public loadbalance_ability 
{
public:
	static resource_server* get_instance();
	void set_video_path(const std::string& video_path);
	int listening(uint16_t port, uint32_t sid);

private:
	friend void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> mess);
	friend void streaming_end_callback(uint32_t ssrc);

	void rolling();
	void rolling_videos();
	void rolling_mediapushing_expires();

	void send_new_video_to_all_lb(std::shared_ptr<std::vector<std::string>> new_videos);
	void send_media_chains_expires(std::shared_ptr<std::vector<uint32_t>> expires_ssrc);
	void send_streaming_end(uint32_t ssrc);

	resource_server();
	~resource_server();

	// 注释掉的是不会发到 resource 的信令 
//	int deal_message(const connection, std::shared_ptr<client_pull_media_stream_message>);					// client -> loadbalance
//	这个信令在 stream_pusher 内部代理了
//	int deal_message(const connection, std::shared_ptr<client_stream_trigger_message>);						// client -> resource
	int deal_message(const connection, std::shared_ptr<keepalive_message>);									// client/loadbalance/resource -> client/loadbalance/resource
	int deal_message(const connection, std::shared_ptr<loadbalance_pull_media_stream_message>);				// loadbalance -> resource
//	int deal_message(const connection, std::shared_ptr<loadbalance_respond_media_menu_pull_message>);		// loadbalance -> client
	int deal_message(const connection, std::shared_ptr<pull_media_menu_message>);							// loadbalance -> resource
//	int deal_message(const connection, std::shared_ptr<pull_other_loadbalance_message>);					// client/loadbalance/resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<push_loadbalance_pull_message>);						// loadbalance -> client/resource
//	int deal_message(const connection, std::shared_ptr<push_media_pull_message>);							// resource -> loadbalance
//	int deal_message(const connection, std::shared_ptr<resource_server_report_message>);					// resource -> loadbalance
//	int deal_message(const connection, std::shared_ptr<resource_server_respond_media_pull_message>);	// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<respond_loadbalance_pull_message>);					// loadbalance -> client/loadbalance/resource
//	int deal_message(const connection, std::shared_ptr<respond_media_menu_pull_message>);					// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<stop_stream_message>);								// client -> loadbalance / loadbalance -> resource
//	int deal_message(const connection, std::shared_ptr<stream_message>);									// resource -> client

private:
	std::map<uint32_t, uint32_t> _ssrc_2_lbfd;
	std::mutex _ssrc_2_lbfd_lock;

	// map<ssrc, stream_pusher>
	std::map<uint32_t, std::shared_ptr<stream_pusher>> _mediastreams;
	std::mutex _mediastreams_lock;

	std::vector<std::string> _videos;
	std::string _videos_path;
	std::mutex _video_mutex;
};

#endif 

