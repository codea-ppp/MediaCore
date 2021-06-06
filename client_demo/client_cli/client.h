#ifndef CLIENT_H
#define CLIENT_H

#include <mutex>
#include <vector>
#include <string>
#include <memory>
#include "stream_render.h"
#include "loadbalance_ability.h"

void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> mess);

class client : virtual public loadbalance_ability
{
public:
	static client* get_instance();
	void play(const std::string& video);
	int listening(uint16_t port, uint32_t sid);
	void set_update_videos_callback(int (*)(std::shared_ptr<std::vector<std::string>>));
	void stop();

private:
	void rolling();
	void rolling_pulling_videos();

    client();
    ~client();

	friend void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> mess);

	// 注释掉的是不会发到 client 的信令 
//	int deal_message(const connection, std::shared_ptr<client_pull_media_stream_message>);					// client -> loadbalance
//	int deal_message(const connection, std::shared_ptr<client_stream_trigger_message>);						// client -> resource
	int deal_message(const connection, std::shared_ptr<keepalive_message>);									// client/loadbalance/resource -> client/loadbalance/resource
//	int deal_message(const connection, std::shared_ptr<loadbalance_pull_media_stream_message>);				// loadbalance -> resource
	int deal_message(const connection, std::shared_ptr<loadbalance_respond_media_pull_message>);			// loadbalance -> client
//	int deal_message(const connection, std::shared_ptr<pull_media_menu_message>);							// loadbalance -> resource / client -> loadbalance
//	int deal_message(const connection, std::shared_ptr<pull_other_loadbalance_message>);					// client/loadbalance/resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<push_loadbalance_pull_message>);						// loadbalance -> client/resource
//	int deal_message(const connection, std::shared_ptr<push_media_pull_message>);							// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<resource_server_report_message>);					// resource -> loadbalance / loadbalance -> client
//	int deal_message(const connection, std::shared_ptr<resource_server_respond_media_pull_message>);		// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<respond_loadbalance_pull_message>);					// loadbalance -> client/loadbalance/resource
	int deal_message(const connection, std::shared_ptr<respond_media_menu_pull_message>);					// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<stop_stream_message>);								// client/loadbalance -> loadbalance/client / loadbalance/resource -> resource/loadbalance
//	int deal_message(const connection, std::shared_ptr<stream_message>);									// resource -> client 这个信令由 stream_render 代理

private:
	int (*_video_update_callback)(std::shared_ptr<std::vector<std::string>>);

	std::map<std::string, int> _videos;
	std::mutex _videos_lock;

	std::map<uint32_t, std::string> _tid_2_media;
	std::mutex _tid_2_media_lock;

	std::map<uint32_t, std::shared_ptr<stream_render>> _mediastreams;
	std::mutex _mediastreams_lock;
};

#endif // CLIENT_H
