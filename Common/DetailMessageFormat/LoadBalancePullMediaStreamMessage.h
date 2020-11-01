#ifndef LOADBALANCE_PULL_MEDIA_STREAM_MESSAGE_H_
#define LOADBALANCE_PULL_MEDIA_STREAM_MESSAGE_H_

#include <vector>
#include <string>
#include <zlog.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MessageType.h"

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) LoadBalancePullMediaStreamMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		uint32_t	ssrc;
		uint16_t	client_receive_port;
		uint16_t	video_name_length;
		uint8_t*	video_name;
	};

	void InitLoadBalancePullMediaStreamMessage(LoadBalancePullMediaStreamMessage* message)
	{
		message->type					= MSGTYPE_LOADBALANCEPULLMEDIASTREAM;
		message->length					= 0x00;
		message->tid					= 0x00;
		message->ssrc					= 0x00;
		message->client_receive_port	= 0x00;
		message->video_name_length		= 0x00;
		message->video_name				= nullptr;
	}

	bool SendLoadBalancePullMediaStreamMessage(int Socket, const LoadBalancePullMediaStreamMessage* message)
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

			if (message->length < 9)
			{
				dzlog_error("message->length: %d", message->length);
				return false;
			}

			if (message->video_name_length <= 0)
			{
				dzlog_error("message->video_name_length <= 0");
				return false;
			}

			if (message->video_name == nullptr)
			{
				dzlog_error("message->video_name == nullptr");
				return false;
			}
		}

		int length	= sizeof(LoadBalancePullMediaStreamMessage) - sizeof(uint8_t*);
		int once	= 0;

		for (int i = 0; i != length; i += once)
		{
			once = send(Socket, (const uint8_t*)message + i, length - i, MSG_CONFIRM);

			if (once == -1)
			{
				dzlog_error("error, errno: %d", errno);
				return false;
			}
		}

		uint16_t name_length = message->video_name_length;
		once = 0;

		for (uint16_t i = 0; i != name_length; i += once)
		{
			once = send(Socket, (const uint8_t*)message->video_name + i, name_length - i, MSG_CONFIRM);

			if (once == -1)
			{
				dzlog_error("error, errno: %d", errno);
				return false;
			}
		}

		dzlog_info("success for socket: %d", Socket);
		return true;
	}

	int SetLoadBalancePullMediaStreamData(LoadBalancePullMediaStreamMessage* message, const std::string& media2play, uint32_t ssrc, uint16_t client_receive_port)
	{
		unsigned int size = media2play.size();

		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return -1;
			}

			if (message->video_name != nullptr)
			{
				dzlog_error("message->video_name != nullptr");
				return -2;
			}

			if (size == 0)
			{
				dzlog_error("try to play a video without name");
				return -3;
			}

			if (ssrc <= 0)
			{
				dzlog_error("ssrc <= 0");
				return -4;
			}
		}

		message->client_receive_port	= client_receive_port;
		message->video_name_length		= size;
		message->length					= 8 + size;
		message->ssrc					= ssrc;

		message->video_name	= new uint8_t[size];
		memcpy(message->video_name, media2play.c_str(), size);

		return 0;
	}

	int ClearLoadBalancePullMediaStreamMessage(LoadBalancePullMediaStreamMessage* const message)
	{
		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return -1;
			}

			if (message->video_name == nullptr)
			{
				dzlog_error("message->video_name == nullptr");
				return -2;
			}
		}

		delete[] message->video_name;
		message->video_name = nullptr;

		return 0;
	}

	int FullFromNet(LoadBalancePullMediaStreamMessage* message, const connection& conn)
	{
		if (message == nullptr)
		{
			dzlog_error("message == nullptr");
			return -1;
		}

		if (message->length <= 8)
		{
			dzlog_error("message->length <= 8");
			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		int once = recv(conn.sockfd, &message->ssrc, 8, MSG_WAITALL);
		if (once != 8)
		{
			dzlog_error("not enough data");
			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		dzlog_info("video name length == %d", message->video_name_length);

		message->video_name = new uint8_t[message->video_name_length];
		once = recv(conn.sockfd, &message->video_name, message->video_name_length, MSG_WAITALL);
		if (once != message->video_name_length)
		{
			dzlog_error("not enough data");
			delete[] message->video_name;
			return NEED_2_CLOSE_SOCKET_ERROR;
		}

		dzlog_info("pulling media stream of %s", message->video_name);
		return 0;
	}
}

#endif 
