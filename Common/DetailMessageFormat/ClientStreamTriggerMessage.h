#ifndef CLIENT_STREAM_TRIGGER_MESSAGE_H_
#define CLIENT_STREAM_TRIGGER_MESSAGE_H_

#include <zlog.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MessageType.h"

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) ClientStreamTriggerMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		uint32_t	ssrc;
	};

	void InitClientStreamTriggerMessage(ClientStreamTriggerMessage* message)
	{
		message->type	= MSGTYPE_CLIENTSTREAMTRIGGER;
		message->length	= 0x04;
		message->tid	= 0x00;
		message->ssrc	= 0x00;
	}

	void PrintMessage(ClientStreamTriggerMessage* message)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return;
		}

		dzlog_info("ClientStreamTriggerMessage(type: [%d], length: [%d], tid: [%d], ssrc: [%d])", message->type, message->length, message->tid, message->ssrc);
	}

	bool SendClientStreamTriggerMessage(int Socket, const ClientStreamTriggerMessage* message)
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

		int length	= sizeof(ClientStreamTriggerMessage);
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

	int SetClientStreamTriggerData(ClientStreamTriggerMessage* message, uint32_t ssrc)
	{
		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return -1;
			}

			if (ssrc <= 0)
			{
				dzlog_error("send port <= 0");
				return -2;
			}
		}

		message->ssrc = ssrc;

		return 0;
	}

	int FullFromNet(ClientStreamTriggerMessage* message, const connection& conn)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		int once = recv(conn.sockfd, &message->ssrc, 4, MSG_WAITALL);
		if (once != 4)
		{
			dzlog_error("no enought data");
			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		return 0;
	}
}

#endif 
