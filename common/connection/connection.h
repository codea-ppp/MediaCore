#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <memory>
#include "message_inerface.h"

class connection_impl;

class connection
{
public:
	int send_message(std::shared_ptr<media_core_message::message>) const;
	int give_message(std::shared_ptr<media_core_message::message>&) const;

	const int show_sockfd() const; // socket fd for connection id
	const char* show_ip() const;
	const uint16_t show_port() const;

	connection(const int sockfd, const uint32_t ip, const uint16_t port);
	~connection();

private:
	std::shared_ptr<connection_impl> impl;
};

#endif 
