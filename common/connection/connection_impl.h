#ifndef CONNECTION_IMPL_H_
#define CONNECTION_IMPL_H_

#include <mutex>
#include <queue>
#include <memory>
#include <string>
#include <stdint.h>
#include "message_inerface.h"

class connection_impl
{
public: 
	int send_message(std::shared_ptr<media_core_message::message>);
	int give_message(std::shared_ptr<media_core_message::message>&);
	
	const int		show_sockfd();
	const char*		show_ip();
	const uint32_t	show_ip_raw();
	const uint16_t	show_port();

	connection_impl(const int sockfd, const uint32_t ip, const uint16_t port);
	~connection_impl();

private:
	void _rolling(std::shared_ptr<media_core_message::message>);

private:
	std::mutex message_sending_lock;

	const int		_sockfd; // also the connection_id
	const uint32_t	_ip;
	const uint16_t	_port;
};

#endif 
