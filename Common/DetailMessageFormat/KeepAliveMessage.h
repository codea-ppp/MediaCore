#ifndef KEEP_ALIVE_MESSAGE_H_
#define KEEP_ALIVE_MESSAGE_H_

#include <zlog.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MessageType.h"

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) KeepAliveMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		uint32_t	sid;
		uint8_t		count;
	};

	void InitKeepAliveMessage(KeepAliveMessage* message)
	{
		message->type	= MSGTYPE_KEEPALIVE;
		message->length = 0x05;
		message->tid	= 0x00;
		message->sid	= 0x00;
		message->count	= 0x00;
	}

	void PrintMessage(KeepAliveMessage* message)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return;
		}

		dzlog_info("KeepAliveMessage(type: [%d], length: [%d], tid: [%d], sid: [%d], count: [%d])", message->type, message->length, message->tid, message->sid, message->count);
	}

	bool SendKeepAliveMessage(int Socket, const KeepAliveMessage* message)
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

		int length	= sizeof(KeepAliveMessage);
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

	int FullFromNet(KeepAliveMessage* message, const connection& conn)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		int once = recv(conn.sockfd, &message->sid, 5, MSG_WAITALL);
		if (once != 5)
		{
			dzlog_error("recv from socket %d failed, errno: %d", conn.sockfd, errno);
			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		return 0;
	}

	int SetKeepAliveData(KeepAliveMessage* message, uint32_t sid, uint8_t count)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		message->count	= count;
		message->sid	= sid;

		return 0;
	}
}

#endif 
