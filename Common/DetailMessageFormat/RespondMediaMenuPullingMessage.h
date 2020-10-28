#ifndef RESPOND_MEDIA_MENU_PULLING_H_
#define RESPOND_MEDIA_MENU_PULLING_H_

#include <vector>
#include <zlog.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace MediaCoreMessageFormat
{
	struct __attribute__((packed)) MediaMeta
	{
		uint32_t VideoNameLength;
		uint8_t* VideoName;
	};

	struct __attribute__((packed)) RespondMediaMenuPullMessage
	{
		uint32_t	type;
		uint32_t	length;
		uint32_t	tid;
		MediaMeta*	MediaMetaResourse;
	};

	void InitRespondMediaMenuPullMessage(RespondMediaMenuPullMessage* message)
	{
		message->type	= 0x21;
		message->length = 0x00;
		message->tid	= 0x00;

		message->MediaMetaResourse = nullptr;
	}

	bool SendRespondMediaMenuPullMessage(int Socket, const RespondMediaMenuPullMessage* message)
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

			if (message->length < 0)
			{
				dzlog_error("message->length: %d", message->length);
				return false;
			}
		}

		int length	= sizeof(RespondMediaMenuPullMessage) - sizeof(MediaMeta*);
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

		for (uint32_t i = 0; i < message->length; ++i)
		{
			once = 0;
			uint32_t name_length = message->MediaMetaResourse[i].VideoNameLength;

			for (uint32_t j = 0; j < name_length; j += once)
			{
				once = send(Socket, (const uint8_t*)message->MediaMetaResourse[i].VideoName + j, name_length - j, MSG_CONFIRM);

				if (once == -1)
				{
					dzlog_error("error, errno: %d", errno);
					return false;
				}
			}
		}

		dzlog_info("success for socket: %d", Socket);
		return true;
	}

	int SetMediaMetaData(RespondMediaMenuPullMessage* message, const std::vector<MediaMeta>& medias)
	{
		unsigned int size = medias.size();

		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return 1;
			}

			if (message->MediaMetaResourse != nullptr)
			{
				dzlog_error("message->MediaMetaResourse != nullptr");
				return 2;
			}

			// size == 0 并不是一种异常, 可以没有视频资源
			if (size == 0) return 0;

			for (unsigned int i = 0; i < size; ++i)
			{
				if (medias[i].VideoName == nullptr)
				{
					dzlog_error("medias[%d].VideoName == nullptr", i);
					return -i;
				}
			}
		}

		message->MediaMetaResourse	= new MediaMeta[size];
		message->length = size;

		for (unsigned int i = 0; i < size; ++i)
		{
			message->MediaMetaResourse[i].VideoNameLength	= medias[i].VideoNameLength;
			message->MediaMetaResourse[i].VideoName			= new uint8_t[medias[i].VideoNameLength];

			memcpy(message->MediaMetaResourse[i].VideoName, medias[i].VideoName, medias[i].VideoNameLength);
		}

		return 0;
	}

	int ClearRespondMediaMenuPullMessage(RespondMediaMenuPullMessage* const message)
	{
		{
			if (message == nullptr)
			{
				dzlog_error("message == nullptr");
				return -1;
			}

			if (message->MediaMetaResourse == nullptr)
			{
				dzlog_error("message->MediaMetaResourse == nullptr");
				return -2;
			}
		}

		for (uint32_t i = 0; i < message->length; ++i)
		{
			if (message->MediaMetaResourse[i].VideoName != nullptr)
				delete[] message->MediaMetaResourse[i].VideoName;

			message->MediaMetaResourse[i].VideoName = nullptr;
		}

		delete[] message->MediaMetaResourse;
		message->MediaMetaResourse = nullptr;

		return 0;
	}
}

#endif 
