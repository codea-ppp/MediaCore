#ifndef SERVER_INERFACE_H_
#define SERVER_INERFACE_H_

#include <map>
#include <mutex>
#include <utility>
#include <stdint.h>
#include "connection.h"
#include "message_headers.h"
#include "net_message_listener.h"

using namespace media_core_message;

class server
{
public:
	virtual int listening(uint16_t port, uint32_t sid) = 0;

	uint32_t get_load();
	void inc_load();
	void swi_load();

	server();

	int tell_me_type(uint32_t sid);

	uint32_t get_tid();

	int	get_ssrc(int which, int id, uint32_t tid);
	int insert_id_tid_2_ssrc(int which, int id, uint32_t tid, uint32_t ssrc);
	int remove_id_tid_2_ssrc(int which, int id, uint32_t tid);

	void set_client_map(uint32_t sid, const connection conn, bool is_connect);
	void set_new_client_map(uint32_t sid, const connection conn, bool is_connect);
	void erase_client_map(const connection conn);
	void rolling_client_map();

	void set_loadbalance_map(uint32_t sid, const connection conn, bool is_connect);
	void set_new_loadbalance_map(uint32_t sid, const connection conn, bool is_connect);
	void erase_loadbalance_map(const connection conn);
	void rolling_loadbalance_map();

	void set_resource_map(uint32_t sid, const connection conn, bool is_connect);
	void set_new_resource_map(uint32_t sid, const connection conn, bool is_connect);
	void erase_resource_map(const connection conn);
	void rolling_resource_map();

	void rolling_map();

	int sockfd_2_sid(int);
	int insert_sock_sid(int sock, uint32_t sid);
	int remove_sock_sid(int sock);

protected:
	virtual int deal_message(const connection, std::shared_ptr<client_pull_media_stream_message>);
	virtual int deal_message(const connection, std::shared_ptr<client_stream_trigger_message>);
	virtual int deal_message(const connection, std::shared_ptr<keepalive_message>);
	virtual int deal_message(const connection, std::shared_ptr<loadbalance_pull_media_stream_message>);
	virtual int deal_message(const connection, std::shared_ptr<loadbalance_respond_media_menu_pull_message>);
	virtual int deal_message(const connection, std::shared_ptr<pull_media_menu_message>);
	virtual int deal_message(const connection, std::shared_ptr<pull_other_loadbalance_message>);
	virtual int deal_message(const connection, std::shared_ptr<push_loadbalance_pull_message>);
	virtual int deal_message(const connection, std::shared_ptr<push_media_pull_message>);
	virtual int deal_message(const connection, std::shared_ptr<resource_server_report_message>);
	virtual int deal_message(const connection, std::shared_ptr<resource_server_respond_media_menu_pull_message>);
	virtual int deal_message(const connection, std::shared_ptr<respond_loadbalance_pull_message>);
	virtual int deal_message(const connection, std::shared_ptr<respond_media_menu_pull_message>);
	virtual int deal_message(const connection, std::shared_ptr<stop_stream_message>);
	virtual int deal_message(const connection, std::shared_ptr<stream_message>);

protected:
	std::vector<uint32_t> _self_ip;
	uint16_t _self_port;
	uint32_t _sid;
	int	_status;

private:
	std::map<std::pair<int, uint32_t>, uint32_t> _sid_tid_2_ssrc[2];
	std::mutex _sid_tid_2_ssrc_lock[2];

	// map<pair<ip, port>, pair<is_connect, sid>>
	std::map<std::pair<uint32_t, uint16_t>, std::pair<bool, uint32_t>> _client_2_is_connect_map;
	std::vector<uint32_t> _client_sids;
	std::mutex _client_2_is_connect_map_lock;

	std::map<std::pair<uint32_t, uint16_t>, std::pair<bool, uint32_t>> _loadbalance_2_is_connect_map;
	std::vector<uint32_t> _loadbalance_sids;
	std::mutex _loadbalance_2_is_connect_map_lock;

	std::map<std::pair<uint32_t, uint16_t>, std::pair<bool, uint32_t>> _resource_2_is_connect_map;
	std::vector<uint32_t> _resource_sids;
	std::mutex _resource_2_is_connect_map_lock;

	std::map<int, uint32_t> _sock_2_sid;
	std::mutex _sock_2_sid_lock;
	
	uint32_t _tid_boundary;

	uint32_t load_index;
	uint32_t load[2];
};

#endif 
