#ifndef RESOURCE_SERVER_REPORT_H_
#define RESOURCE_SERVER_REPORT_H_

#include <zlog.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) ResourceServerReportMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		uint8_t		error;
	};

	void InitResourceServerReportMessage(ResourceServerReportMessage* message)
	{
		message->type	= MSGTYPE_RESOURCESERVERREPORT;
		message->length	= 0x01;
		message->tid	= 0x00;
		message->error	= 0x00;
	}

	void PrintMessage(ResourceServerReportMessage* message)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return;
		}

		dzlog_info("ResourceServerReportMessage(type: [%d], length: [%d], tid: [%d], erro: [%d])", message->type, message->length, message->tid, message->error);
	}

	bool SendResourceServerReportMessage(int Socket, const ResourceServerReportMessage* message)
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

		int length	= sizeof(ResourceServerReportMessage);
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

	int SetResourceServerReportData(ResourceServerReportMessage* message, uint8_t error)
	{
		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return -1;
			}

			if (error <= 0)
			{
				dzlog_error("send port <= 0");
				return -2;
			}
		}

		message->error = error;

		return 0;
	}

	int FullFromNet(ResourceServerReportMessage* message, const connection& conn)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		int once = recv(conn.sockfd, &message->error, 1, MSG_WAITALL);
		if (once != 1)
		{
			dzlog_error("not enough data");
			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		return 0;
	}
}

#endif 
