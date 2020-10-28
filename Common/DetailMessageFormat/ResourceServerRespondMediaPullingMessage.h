#ifndef RESOURCE_SERVER_RESPOND_MEDIA_STREAM_PULLING_H_
#define RESOURCE_SERVER_RESPOND_MEDIA_STREAM_PULLING_H_

#include <zlog.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) ResourceServerRespondMediaPullMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		uint16_t	send_port;
	};

	void InitResourceServerRespondMediaPullMessage(ResourceServerRespondMediaPullMessage* message)
	{
		message->type		= 0x32;
		message->length		= 0x02;
		message->tid		= 0x00;
		message->send_port	= 0x00;
	}

	bool SendResourceServerRespondMediaPullMessage(int Socket, const ResourceServerRespondMediaPullMessage* message)
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

		int length	= sizeof(ResourceServerRespondMediaPullMessage);
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

	int SetResourceServerRespondMediaPullData(ResourceServerRespondMediaPullMessage* message, uint16_t send_port)
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
		}

		message->send_port = send_port;

		return 0;
	}
}

#endif 
