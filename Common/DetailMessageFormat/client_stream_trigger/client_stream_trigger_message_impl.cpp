#include <zlog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "client_stream_trigger_message_impl.h"

namespace media_core_message
{
	int client_stream_trigger_message_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0) 
		{
			dzlog_error("socketfd: %d", sockfd);
			return ERR_SOCKET_FD_NEGATIVE;
		}

		if (_tid == 0)
		{
			dzlog_error("tid not set");
			return ERR_TID_NOT_SET;
		}

		if (_ssrc == 0)
		{
			dzlog_error("ssrc not set");
			return ERR_SSRC_NOT_SET;
		}

		uint32_t buffer[4] = { 0 };
		buffer[1] = MSGTYPE_CLIENTSTREAMTRIGGER;	
		buffer[2] =	4;
		buffer[3] = _tid;
		buffer[4] = _ssrc;

		int once = 0;
		for (int i = 0; i != 16; i += once)
		{
			once = send(sockfd, (const uint8_t*)buffer + i, 16 - i, 0);

			if (once == -1)
			{
				dzlog_error("send error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int client_stream_trigger_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		_tid = tid;

		int once = recv(sockfd, &_ssrc, 4, MSG_WAITALL);
		if (once != 4)
		{
			clear();
			
			dzlog_error("no enought data");
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		return 0;
	}

	int client_stream_trigger_message_impl::full_data_direct(uint32_t tid, uint32_t ssrc)
	{
		_ssrc	= ssrc;
		_tid	= tid;

		return 0;
	}

	void client_stream_trigger_message_impl::print_data()
	{
		dzlog_info("tid: %d, ssrc: %d", _tid, _ssrc);
	}

	void client_stream_trigger_message_impl::init()
	{
		clear();
	}

	void client_stream_trigger_message_impl::clear()
	{
		_tid	= 0x00;
		_ssrc	= 0x00;
	}

	client_stream_trigger_message_impl::client_stream_trigger_message_impl()
	{
		clear();
	}

	client_stream_trigger_message_impl::~client_stream_trigger_message_impl()
	{
		clear();
	}
}
