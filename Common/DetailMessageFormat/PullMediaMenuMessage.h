#ifndef PULL_MEDIA_MENU_H_
#define PULL_MEDIA_MENU_H_

#include <zlog.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MessageType.h"

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) PullMediaMenuMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
	};

	void PrintMessage(PullMediaMenuMessage* message)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return;
		}

		dzlog_info("PullMediaMenuMessage(type: [%d], length: [%d], tid: [%d])", message->type, message->length, message->tid);
	}

	void InitPullMediaMenuMessage(PullMediaMenuMessage* message)
	{
		message->type	= MSGTYPE_PULLMEDIAMENU;
		message->length = 0x00;
		message->tid	= 0x00;
	}

	bool SendPullMediaMenuMessage(int Socket, const PullMediaMenuMessage* message)
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

		int length	= sizeof(PullMediaMenuMessage);
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
