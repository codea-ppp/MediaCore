#ifndef CONNECTION_IMPL_H_
#define CONNECTION_IMPL_H_

#include <string>
#include <stdint.h>
#include <memory>
#include "message_inerface.h"

class connection_impl
{
public: 
	int send_message(media_core_message::message*);
	int give_message(std::shared_ptr<media_core_message::message>&);
	
	const char*		show_ip();
	const uint16_t	show_port();

	connection_impl(const int sockfd, const uint32_t ip, const uint16_t port);
	~connection_impl();

private:
	const int _sockfd;		// also the connection_id
	const uint32_t _ip;
	const uint16_t _port;
};

#endif 
