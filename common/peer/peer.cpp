#include "peer.h"

peer::peer(const connection conn, const uint32_t sid, const uint16_t listening_port) 
	: _conn(conn), _sid(sid), _pid(conn.show_sockfd()), _listening_port(listening_port)
{
	_last_time_refresh = std::chrono::steady_clock::now();
}

int peer::send_message(std::shared_ptr<media_core_message::message> mess)
{
	int result = _conn.send_message(mess);

	if (!result)
	{
		fresh();
	}

	return result;
}

void peer::fresh()
{
	_last_time_refresh = std::chrono::steady_clock::now();
}

bool peer::is_expires()
{
	return (std::chrono::steady_clock::now() - _last_time_refresh) > std::chrono::seconds(120);
}

const connection peer::get_connection()
{
	return _conn;
}

const uint32_t peer::get_pid() const 
{
	return _pid;
}

const uint32_t peer::get_sid() const 
{
	return _sid;
}

const uint32_t peer::get_type() const 
{
	return (_sid & 0xff000000) >> 24;
}

const uint32_t peer::get_load() const
{
	return _load;
}

const uint16_t peer::get_listening_port() const
{
	return _listening_port;
}

void peer::set_load(uint32_t load)
{
	_load = load;
}

