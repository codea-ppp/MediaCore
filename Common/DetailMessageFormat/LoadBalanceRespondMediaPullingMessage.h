#ifndef LOADBALANCE_RESPOND_MEDIA_PULLING_MESSAGE_H_
#define LOADBALANCE_RESPOND_MEDIA_PULLING_MESSAGE_H_

#include <zlog.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) LoadBalanceRespondMediaPullMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		uint32_t	ssrc;
		uint16_t	send_port;
	};

	void InitLoadBalanceRespondMediaPullMessage(LoadBalanceRespondMediaPullMessage* message)
	{
		message->type		= 0x33;
		message->length		= 0x06;
		message->tid		= 0x00;
		message->ssrc		= 0x00;
		message->send_port	= 0x00;
	}

	bool SendLoadBalanceRespondMediaPullMessage(int Socket, const LoadBalanceRespondMediaPullMessage* message)
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
		}

		int length	= sizeof(LoadBalanceRespondMediaPullMessage);
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

		dzlog_info("success for socket: %d", Socket);
		return true;
	}

	int SetLoadBalanceRespondMediaPullData(LoadBalanceRespondMediaPullMessage* message, uint32_t ssrc, uint16_t send_port)
	{
		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return -1;
			}

			if (send_port <= 0)
			{
				dzlog_error("send port <= 0");
				return -2;
			}

			if (ssrc <= 0)
			{
				dzlog_error("ssrc <= 0");
				return -3;
			}
		}

		message->send_port	= send_port;
		message->ssrc		= ssrc;

		return 0;
	}
}

#endif 
