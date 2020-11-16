#include <vector>
#include <thread>
#include <chrono>
#include <zlog.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "net_message_listener_impl.h"

using namespace media_core_message;

#define MAX_CONNECTION_PENDING 10

net_message_listener_impl* const net_message_listener_impl::get_instance()
{
	static net_message_listener_impl instance;
	return &instance; 
}

void net_message_listener_impl::stop()
{
	_status = NET_MESSAGE_LISTENER_STOP;
}

void net_message_listener_impl::set_callback(void (*message_2_go)(const connection conn, uint32_t message_type, std::shared_ptr<media_core_message::message>))
{
	if (message_2_go == nullptr)
	{
		dzlog_error("message_ptr == nullptr");
		return;
	}

	_message_2_go = message_2_go;
}

int net_message_listener_impl::connect_with_message(uint32_t ip, uint16_t port, std::shared_ptr<media_core_message::message> mess)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family	= AF_INET;
	addr.sin_port	= port;

	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
	{
		dzlog_error("failed to convern ip");
		return -1;
	}
	
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		dzlog_error("connect failed, errno: %d", errno);
		return -1;
	}

	connection conn(sockfd, ip, port);
	conn.send_message(mess);

	threadpool_instance::get_instance()->schedule(std::bind(&net_message_listener_impl::_rolling, this, conn));
	return 0;
}

int net_message_listener_impl::listening(uint16_t port)
{
	if (_message_2_go == nullptr)
	{
		dzlog_error("_message_2_go == nullptr");
		return -1;
	}

	if (_status == 1)
	{
		dzlog_error("_status == 1");
		return -2;
	}

	_status = NET_MESSAGE_LISTENER_RUNNING;
	_port	= port;

	threadpool_instance::get_instance()->schedule(std::bind(&net_message_listener_impl::_listening,	this));

	return 0;
}

void net_message_listener_impl::_listening()
{
	if (!_status)
	{
		dzlog_error("_status == 0");
		return;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		dzlog_error("socket fd == -1, errno = %d", errno);
		exit(0);
	}

	struct sockaddr_in my_sock, peer_sock;
	memset(&my_sock,	0, sizeof(struct sockaddr_in));
	memset(&peer_sock,	0, sizeof(struct sockaddr_in));

	my_sock.sin_family		= AF_INET;
	my_sock.sin_addr.s_addr	= INADDR_ANY;
	my_sock.sin_port		= htons(_port);

	if (bind(sockfd, (struct sockaddr*)&my_sock, sizeof(my_sock)))
	{
		dzlog_error("bind socket %d failed, errno: %d", sockfd, errno);
		exit(0);
	}
	
	if (listen(sockfd, MAX_CONNECTION_PENDING))
	{
		dzlog_error("listening to socked %d failed, errno: %d", sockfd, errno);
		exit(0);
	}

	dzlog_info("start listening on socket %d", sockfd);

	while (_status)
	{
		unsigned int temp = sizeof(peer_sock);

		dzlog_info("trying to accept a connection");

		int peer = accept(sockfd, (struct sockaddr*)&peer_sock, &temp);
		if (peer == -1)
		{
			dzlog_error("peer == -1, errno: %d", errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		connection conn(peer, peer_sock.sin_addr.s_addr, peer_sock.sin_port);

		dzlog_info("accept a socket: %d@%s:%d", peer, conn.show_ip(), conn.show_port());

		threadpool_instance::get_instance()->schedule(std::bind(&net_message_listener_impl::_rolling, this, conn));

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	dzlog_info("at the end of _listening");
}

void net_message_listener_impl::_rolling(const connection conn)
{
	if (!_status)
	{
		dzlog_error("_status == 0");
		return;
	}

	if (_message_2_go == nullptr)
	{
		dzlog_error("_message_2_go == nullptr");
		return;
	}

	while (_status)
	{
		std::shared_ptr<message> mess;

		if (conn.give_message(mess))
		{
			dzlog_error("failed to get message for %s:%d", conn.show_ip(), conn.show_port());
			return;
		}

		_message_2_go(conn, mess->tell_me_type(), mess);

		dzlog_info("rolling for %s:%d end", conn.show_ip(), conn.show_port());
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
	}

	dzlog_info("end connection %s:%d", conn.show_ip(), conn.show_port());
}

net_message_listener_impl::net_message_listener_impl()
{
	_message_2_go	= nullptr;
	_status			= NET_MESSAGE_LISTENER_STOP;
	_port			= -1;
}

net_message_listener_impl::~net_message_listener_impl()
{
	stop();
	std::this_thread::sleep_for(std::chrono::seconds(3));
}

