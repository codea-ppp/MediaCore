#ifndef LOADBALANCE_H_
#define LOADBALANCE_H_

#include <map>
#include <list>
#include <mutex>
#include "peer.h"
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "media_chain.h"
#include "client_ability.h"
#include "resource_ability.h"
#include "loadbalance_ability.h"

void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> ptr);

class loadbalance : 
	public client_ability, 
	public loadbalance_ability, 
	public resource_ability
{
public:
	static loadbalance* get_instance();
	int listening(uint16_t port, uint32_t sid);

private:
	friend void net_message_listener_callback(const connection conn, uint32_t message_type, std::shared_ptr<message> ptr);

	// 注释掉的是不会发到 loadbalance 的信令 
	int deal_message(const connection, std::shared_ptr<client_pull_media_stream_message>);					// client -> loadbalance
//	int deal_message(const connection, std::shared_ptr<client_stream_trigger_message>);						// client -> resource
	int deal_message(const connection, std::shared_ptr<keepalive_message>);									// client/loadbalance/resource -> client/loadbalance/resource
//	int deal_message(const connection, std::shared_ptr<loadbalance_pull_media_stream_message>);				// loadbalance -> resource
//	int deal_message(const connection, std::shared_ptr<loadbalance_respond_media_pull_message>);		// loadbalance -> client
	int deal_message(const connection, std::shared_ptr<pull_media_menu_message>);							// loadbalance -> resource / client -> loadbalance
	int deal_message(const connection, std::shared_ptr<pull_other_loadbalance_message>);					// client/loadbalance/resource -> loadbalance
//	int deal_message(const connection, std::shared_ptr<push_loadbalance_pull_message>);						// loadbalance -> client/resource
	int deal_message(const connection, std::shared_ptr<push_media_pull_message>);							// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<resource_server_report_message>);					// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<resource_server_respond_media_pull_message>);		// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<respond_loadbalance_pull_message>);					// loadbalance -> client/loadbalance/resource
	int deal_message(const connection, std::shared_ptr<respond_media_menu_pull_message>);					// resource -> loadbalance
	int deal_message(const connection, std::shared_ptr<stop_stream_message>);								// client -> loadbalance / loadbalance -> resource
//	int deal_message(const connection, std::shared_ptr<stream_message>);									// resource -> client

	int send_new_loadbalance_2_client_resource(uint32_t sid, const connection lb);
	int send_media_menu_pulling(const connection resource);

	int check_video_and_find_resource_least_load(const std::string&);

	uint32_t find_resource_least_load();
	uint32_t find_idle_ssrc();

	void shoting_and_rolling();
	void shoting_media_chain();

	loadbalance();
	~loadbalance();

	loadbalance(const loadbalance&)		= delete;
	loadbalance(const loadbalance&&)	= delete;
	const loadbalance& operator=(const loadbalance&)	= delete;
	const loadbalance&& operator=(const loadbalance&&)	= delete;

private:
	// 以视频名称为 key
	std::map<std::string, std::map<int, std::shared_ptr<peer>>> _video_resources;
	std::mutex _video_resources_lock;

	// ssrc for key
	std::map<int, std::shared_ptr<media_chain>> _media_chain_map;
	std::mutex _media_chain_map_lock;

	uint32_t _ssrc_boundary;
};

#endif 
