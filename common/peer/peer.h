#ifndef PEER_H_
#define PEER_H_

#include <chrono>
#include "connection.h"
#include "message_headers.h"

#define PEER_TYPE_CLIENT		0
#define PEER_TYPE_LOADBALANCE	1
#define	PEER_TYPE_RESOURCE		2

class peer
{
public:
	peer(const connection conn, const uint32_t sid);

	int send_message(std::shared_ptr<media_core_message::message>);

	void fresh();
	bool is_expires();

	const connection get_connection();

	const uint32_t get_pid() const;
	const uint32_t get_sid() const;
	const uint32_t get_type() const;
	const uint32_t get_load() const;
	void set_load(uint32_t);

private:
	std::chrono::time_point<std::chrono::steady_clock> _last_time_refresh;

	const connection _conn;
	const uint32_t _sid;
	const uint32_t _pid;

	uint32_t _load;
};

#endif 
