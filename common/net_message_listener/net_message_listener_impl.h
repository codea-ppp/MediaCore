#ifndef NET_MESSAGE_LISTENER_IMPL_H_
#define NET_MESSAGE_LISTENER_IMPL_H_

#include <mutex>
#include "connection.h"
#include "message_type.h"
#include "message_headers.h"
#include "threadpool_instance.h"

class net_message_listener_impl
{
public:
	static net_message_listener_impl* const get_instance();

	void stop();
	void set_callback(void (*message_2_go)(const connection conn, uint32_t message_type, std::shared_ptr<media_core_message::message> ptr));

	int listening(uint16_t port);

private:
	void _listening();
	void _rolling(const connection conn);

	net_message_listener_impl();
	~net_message_listener_impl();

	net_message_listener_impl(const net_message_listener_impl&)		= delete;
	net_message_listener_impl(const net_message_listener_impl&&)	= delete;
	const net_message_listener_impl& operator=(const net_message_listener_impl&)	= delete;
	const net_message_listener_impl& operator=(const net_message_listener_impl&&)	= delete;

private:
	void (*_message_2_go)(const connection conn, uint32_t message_type, std::shared_ptr<media_core_message::message> ptr);

	int	_status;
	int _port;
};

#endif 
