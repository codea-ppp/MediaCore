#ifndef RESPOND_LOADBALANCE_PULLING_MESSAGE_H_
#define RESPOND_LOADBALANCE_PULLING_MESSAGE_H_

#include <vector>
#include <zlog.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MessageType.h"

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) RespondLoadBalancePullMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		uint32_t*	ip;
		uint16_t*	port;
	};

	void InitRespondLoadBalancePullMessage(RespondLoadBalancePullMessage* message)
	{
		message->type	= MSGTYPE_RESPONDLOADBALANCEPULL;
		message->length = 0x00;
		message->tid	= 0x00;
		message->ip		= nullptr;
		message->port	= nullptr;
	}

	void PrintMessage(RespondLoadBalancePullMessage* message)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return;
		}

		if (message->length % 6 != 0)
		{
			dzlog_error("message->length mod 5 != 0");
			return;
		}

		dzlog_info("RespondLoadBalancePullMessage(type: [%d], length: [%d], tid: [%d])", message->type, message->length, message->tid);
		
		static char ipAddr[16];
		for (uint32_t i = 0; i < message->length / 6; ++i)
		{
			memset(ipAddr, 0, 16);
			snprintf(ipAddr, sizeof(ipAddr), "%u.%u.%u.%u", (message->ip[i] & 0x000000ff), (message->ip[i] & 0x0000ff00) >> 8, (message->ip[i] & 0x00ff0000) >> 16, (message->ip[i] & 0xff000000) >> 24);

			dzlog_info("get loadbalance %d@%s:%d", i, ipAddr, ntohs(message->port[i]));
		}
	}

	bool SendRespondLoadBalancePullMessage(int Socket, const RespondLoadBalancePullMessage* message)
	{
		{
			if (Socket < 0) 
			{
				dzlog_error("socketfd: %d", Socket);
				return false;
			}

			if (message == nullptr) 
			{
				dzlog_error("message == nullptr");
				return false;
			}

			if (message->length < 0 || message->length % 6 != 0)
			{
				dzlog_error("wrong length: %d", message->length);
				return false;
			}

			if (message->ip == nullptr)
			{
				dzlog_error("ip == nullptr");
				return false;
			}

			if (message->port == nullptr)
			{
				dzlog_error("port == nullptr");
				return false;
			}
		}

		int length	= sizeof(RespondLoadBalancePullMessage) - sizeof(uint32_t*) - sizeof(uint16_t*);
		int once	= 0;

		for (int i = 0; i != length; i += once)
		{
			once = send(Socket, (const uint8_t*)message + i, length - i, MSG_CONFIRM);

			if (once == -1)
			{
				dzlog_error("errno: %d", errno);
				return false;
			}
		}
		
		// message->length = n * (sizeof(ip) + sizeof(port))
		int port_length	= message->length / 3;
		int ip_length	= message->length - port_length;

		dzlog_info("message->length %d which ip %d bytes port %d bytes", message->length, ip_length, port_length);

		once = 0;
		for (int i = 0; i != ip_length; i += once)
		{
			// ip_length 就是以字节计数的
			once = send(Socket, (const uint8_t*)message->ip + i, ip_length - i, MSG_CONFIRM);

			if (once == -1)
			{
				dzlog_error("errno: %d", errno);
				return false;
			}
		}

		once = 0;
		for (int i = 0; i != port_length; i += once)
		{
			once = send(Socket, (const uint8_t*)message->port + i, port_length - i, MSG_CONFIRM);

			if (once == -1)
			{
				dzlog_error("errno: %d", errno);
				return false;
			}
		}

		dzlog_info("success for socket: %d", Socket);
		return true;
	}

	int FullFromNet(RespondLoadBalancePullMessage* message, const connection& conn)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		if (message->length < 0)
		{
			dzlog_error("message->length < 0");
			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		int port_length	= message->length / 3;
		int ip_length	= message->length - port_length;

		message->ip		= new uint32_t[message->length / 6];
		message->port	= new uint16_t[message->length / 6];

		dzlog_info("message->length %d which ip length %d, port length %d", message->length, ip_length, port_length);

		int once = recv(conn.sockfd, message->ip, ip_length, MSG_WAITALL);
		if (once != ip_length)
		{
			dzlog_error("no enough data");

			delete[] message->ip;
			delete[] message->port;

			message->ip		= nullptr;
			message->port	= nullptr;

			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		once = recv(conn.sockfd, message->port, port_length, MSG_WAITALL);
		if (once != port_length)
		{
			dzlog_error("no enough data");

			delete[] message->ip;
			delete[] message->port;

			message->ip		= nullptr;
			message->port	= nullptr;

			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		return 0;
	}

	int SetOtherLoadBalanceData(RespondLoadBalancePullMessage* message, const std::vector<uint32_t>& IPs, const std::vector<uint16_t>& Ports)
	{
		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return -1;
			}

			if (message->ip != nullptr)
			{
				dzlog_error("message->ip != nullptr");
				return -2;
			}

			if (message->port != nullptr)
			{
				dzlog_error("message->port != nullptr");
				return -3;
			}

			if (IPs.size() != Ports.size())
			{
				dzlog_error("IPs size != Ports size");
				return -4;
			}
		}

		int size = IPs.size();
		message->length = size * 6;

		message->ip		= new uint32_t[size];
		message->port	= new uint16_t[size];

		for (int i = 0; i < size; ++i)
		{
			message->ip[i]		= IPs[i];
			message->port[i]	= Ports[i];
		}

		return 0;
	}

	int ClearRespondLoadBalancePullMessage(RespondLoadBalancePullMessage* message)
	{
		{
			if (message->ip == nullptr)
			{
				dzlog_error("message->ip == nullptr");
				return -1;
			}

			if (message->port == nullptr)
			{
				dzlog_error("message->port == nullptr");
				return -2;
			}
		}

		delete[] message->ip;
		delete[] message->port;

		message->ip		= nullptr;
		message->port	= nullptr;

		return 0;
	}
}

#endif 
