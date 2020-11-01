#include <vector>
#include <thread>
#include <chrono>
#include <zlog.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "NetMessageListener.h"

using namespace MediaCoreMessageFormat;

#define MAX_CONNECTION_PENDING 20

NetMessageListener::NetMessageListener()
{
	_Message2Go = nullptr;
	_status		= 0;
	_port		= -1;
}

NetMessageListener::~NetMessageListener()
{
	_status = 0;
	std::this_thread::sleep_for(std::chrono::seconds(8));
}

void NetMessageListener::SetCallback(void (*Message2Go)(const connection& conn, void* ptr))
{
	_Message2Go = Message2Go;
}

int NetMessageListener::listening(uint16_t port)
{
	if (_Message2Go == nullptr)
	{
		dzlog_error("_Message2Go == nullptr");
		return -1;
	}

	if (_status == 1)
	{
		dzlog_error("_status == 1");
		return -2;
	}

	_status = 1;
	_port	= port;

	std::thread* ptr = new std::thread(&NetMessageListener::_listening, this);
	ptr->detach();
	delete ptr;

	std::thread* ptr2 = new std::thread(&NetMessageListener::_rolling, this);
	ptr2->detach();
	delete ptr2;

	return 0;
}

void NetMessageListener::_listening()
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

		connection conn;
		conn.ip		= inet_ntoa(peer_sock.sin_addr);
		conn.port	= ntohs(peer_sock.sin_port);
		conn.sockfd = peer;

		dzlog_info("accept a socket: %d@%s:%d", conn.sockfd, conn.ip.c_str(), conn.port);

		{
			std::lock_guard<std::mutex> lk(pending_lock);
			pending_sockfds.push_back(conn);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	dzlog_info("at the end of _listening");
}

void* NetMessageListener::construct_message(const connection& conn, uint8_t* buffer)
{
	uint32_t type	= ((uint32_t*)buffer)[0];
	uint32_t length	= ((uint32_t*)buffer)[1];
	uint32_t tid	= ((uint32_t*)buffer)[2];

	switch (type)
	{
	case MSGTYPE_KEEPALIVE: 
	{
		KeepAliveMessage* message = new KeepAliveMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_PULLOTHERLOADBALANCE:
	{
		PullOtherLoadBalanceMessage* message = new PullOtherLoadBalanceMessage;

		SetHead(message, type, length, tid);
		return (void*)message;
	}

	case MSGTYPE_RESPONDLOADBALANCEPULL:
	{
		RespondLoadBalancePullMessage* message = new RespondLoadBalancePullMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_PUSHLOADBANLANCE:
	{
		PushLoadBalanceMessage* message = new RespondLoadBalancePullMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_PULLMEDIAMENU:
	{
		PullMediaMenuMessage* message = new PullMediaMenuMessage;

		SetHead(message, type, length, tid);
		return (void*)message;
	}

	case MSGTYPE_RESPONDMEDIAMENU:
	{
		RespondMediaMenuPullMessage* message = new RespondMediaMenuPullMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_PUSHMEDIAMENU:
	{
		PushMediaMenuMessage* message = new PushMediaMenuMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_CLIENTPULLMEDIASTREAM:
	{
		ClientPullMediaStreamMessage* message = new ClientPullMediaStreamMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM:
	{
		LoadBalancePullMediaStreamMessage* message = new LoadBalancePullMediaStreamMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_RESOURCERESPONDMEDIAPULL:
	{
		ResourceServerRespondMediaPullMessage* message = new ResourceServerRespondMediaPullMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:
	{
		LoadBalanceRespondMediaPullMessage* message = new LoadBalanceRespondMediaPullMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_CLIENTSTREAMTRIGGER:
	{
		ClientStreamTriggerMessage* message = new ClientStreamTriggerMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_STREAMFRAME:
	{
		StreamMessage* message = new StreamMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;
		
		return (void*)message;
	}

	case MSGTYPE_STOPSTREAM:
	{
		StopStreamMessage* message = new StopStreamMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	case MSGTYPE_RESOURCESERVERREPORT:
	{
		ResourceServerReportMessage* message = new ResourceServerReportMessage;

		SetHead(message, type, length, tid);
		if (FullFromNet(message, conn) < 0)
			return nullptr;

		return (void*)message;
	}

	default: return nullptr;
	}
}

void NetMessageListener::_rolling()
{
	if (!_status)
	{
		dzlog_error("_status == 0");
		return;
	}

	if (_Message2Go == nullptr)
	{
		dzlog_error("_Message2Go == nullptr");
		return;
	}

	static uint8_t buffer[12];
	memset(buffer, 0, 12);

	bool is_data;

	while (_status)
	{
		int size = pending_sockfds.size();
		if (size)
		{
			dzlog_info("there are %d socket to add", size);
			std::lock_guard<std::mutex> lk(pending_lock);

			for (std::vector<connection>::iterator i = pending_sockfds.begin(); i != pending_sockfds.end(); ++i) 
			{
				rolling_sockfds.push_back(*i);
				dzlog_info("%d@%s:%d start to rolling", i->sockfd, i->ip.c_str(), i->port);
			}
			
			pending_sockfds.clear();
		}

		is_data = false;

		for (std::vector<connection>::iterator i = rolling_sockfds.begin(); i != rolling_sockfds.end(); ++i) 
		{
			int temp = recv(i->sockfd, buffer, 12, MSG_WAITALL);
			if (temp == -1 || temp != 12)
			{
				dzlog_error("recv failed when reading socket %d", i->sockfd);

				close(i->sockfd);
				rolling_sockfds.erase(i);
			}

			void* message = construct_message(*i, buffer);
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");

				close(i->sockfd);
				rolling_sockfds.erase(i);
			}

			_Message2Go(*i, message);
			is_data = true;
		}

		if (!is_data) std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

