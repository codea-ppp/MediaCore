#ifndef RESPOND_LOADBALANCE_PULLING_MESSAGE_H_
#define RESPOND_LOADBALANCE_PULLING_MESSAGE_H_

#include <vector>
#include <zlog.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

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
		message->type	= 0x03;
		message->length = 0x00;
		message->tid	= 0x00;
		message->ip		= nullptr;
		message->port	= nullptr;
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

		// 标准规定了 vector 的内部是连续存储且 &std::vector[0] 就是内部的 buffer 首地址
		message->ip	= new uint32_t[size];
		memcpy(message->ip, &IPs[0], size);

		message->port = new uint16_t[size];
		memcpy(message->port, &Ports[0], size);

		return 0;
	}

	int ClearRespondLoadBalancePullMessage(RespondLoadBalancePullMessage* message)
	{
		{
			if (message->ip != nullptr)
			{
				dzlog_error("message->ip == nullptr");
				return -1;
			}

			if (message->port != nullptr)
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
