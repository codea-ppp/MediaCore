#ifndef PEER_H_
#define PEER_H_

#include <chrono>
#include "connection.h"
#include "message_headers.h"

class peer
{
public:
	peer(const connection conn, const uint32_t sid, const uint32_t pid);

	int send_message(std::shared_ptr<media_core_message::message>);

	void fresh();
	bool is_expires();

	const connection get_connection();

	const uint32_t get_pid();
	const uint32_t get_sid();
	const uint32_t get_type();
	const uint32_t get_load();
	void set_load(uint32_t);

private:
	std::chrono::time_point<std::chrono::steady_clock> _last_time_refresh;

	const connection _conn;
	const uint32_t _sid;
	const uint32_t _pid;

	uint32_t _load;
};

#endif 
