#include "net_message_listener.h"
#include "net_message_listener_impl.h"

net_message_listener* const net_message_listener::get_instance()
{
	static net_message_listener instance;
	return &instance;
}

void net_message_listener::stop()
{
	impl->stop();
}

void net_message_listener::set_callback(void (*message_2_go)(const connection, uint32_t, std::shared_ptr<media_core_message::message>))
{
	impl->set_callback(message_2_go);
}

int net_message_listener::listening(uint16_t port)
{
	return impl->listening(port);
}

net_message_listener::net_message_listener()
{
	impl = std::make_unique<net_message_listener_impl>();
}

net_message_listener::~net_message_listener()
{
}
