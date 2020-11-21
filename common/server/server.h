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
	virtual int deal_message(const connection, std::shared_ptr<resource_server_respond_media_pull_message>);
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

	uint32_t _tid_boundary;

	uint32_t load_index;
	uint32_t load[2];
};

#endif 
