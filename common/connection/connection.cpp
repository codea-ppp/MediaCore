#include "connection.h"
#include "connection_impl.h"

int connection::send_message(std::shared_ptr<media_core_message::message> mess) const
{
	return impl->send_message(mess);
}

int connection::give_message(std::shared_ptr<media_core_message::message>& mess) const
{
	return impl->give_message(mess);
}

const int connection::show_sockfd() const
{
	return impl->show_sockfd();
}

const char* connection::show_ip() const  
{
	return impl->show_ip();
}

const uint32_t connection::show_ip_raw() const
{
	return impl->show_ip_raw();
}

const uint16_t connection::show_port() const
{
	return impl->show_port();
}

connection::connection(const int sockfd, const uint32_t ip, const uint16_t port)
{
	impl = std::make_shared<connection_impl>(sockfd, ip, port);
}

connection::~connection()
{
}
