#ifndef NET_MESSAGE_LISTENER_H_
#define NET_MESSAGE_LISTENER_H_

#include <memory>
#include "connection.h"
#include "message_type.h"
#include "message_headers.h"

class net_message_listener_impl;

class net_message_listener
{
public:
	static net_message_listener* const get_instance();

	void stop();
	void set_callback(void (*message_2_go)(const connection conn, uint32_t message_type, std::shared_ptr<media_core_message::message> ptr));

	int connect_with_message(uint32_t ip, uint16_t port, std::shared_ptr<media_core_message::message>);
	int listening(uint16_t port);

private:
	net_message_listener();
	~net_message_listener();

private:
	std::unique_ptr<net_message_listener_impl> impl;
};

#endif 
