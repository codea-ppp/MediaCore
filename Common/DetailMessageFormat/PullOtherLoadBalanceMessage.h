#ifndef	PULL_OTHER_LOADBALANCE_MESSAGE_H_
#define PULL_OTHER_LOADBALANCE_MESSAGE_H_

#include <zlog.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) PullOtherLoadBalanceMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
	};

	void InitPullOtherLoadBalanceMessage(PullOtherLoadBalanceMessage* message)
	{
		message->type	= 0x02;
		message->length = 0x00;
		message->tid	= 0x00;
	}

	bool SendPullOtherLoadBalanceMessage(int Socket, const PullOtherLoadBalanceMessage* message)
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

		int length	= sizeof(PullOtherLoadBalanceMessage);
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
}

#endif 
