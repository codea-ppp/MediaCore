#include "stop_stream_message_impl.h"

namespace media_core_message
{
	int stop_stream_message_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0)
		{
			dzlog_info("sockfd < 0");
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

		uint32_t buffer[4];
		buffer[0] = MSGTYPE_STOPSTREAM;
		buffer[1] = 4;
		buffer[2] = _tid;
		buffer[3] = _ssrc;

		int once = 0;
		for (int i = 0; i < 16; i += once)
		{
			once = send(sockfd, (uint8_t*)buffer + i, 16 - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("send failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int stop_stream_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		if (sockfd < 0)
		{
			dzlog_error("sockfd < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		if (_ssrc == 0)
		{
			dzlog_error("ssrc not set");
			return ERR_SSRC_NOT_SET;
		}

		uint32_t ssrc;
		int once = recv(sockfd, (uint8_t*)&ssrc, 4, MSG_WAITALL);
		if (once != 4)
		{
			clear();

			dzlog_error("recv failed: %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_tid	= tid;
		_ssrc	= ssrc;

		dzlog_info("get stop stream: %d", ssrc);
		return 0;
	}

	int stop_stream_message_impl::full_data_direct(uint32_t tid, uint32_t ssrc)
	{
		_tid	= tid;
		_ssrc	= ssrc;

		return 0;
	}

	int stop_stream_message_impl::give_me_data(uint32_t& tid, uint32_t& ssrc)
	{
		tid		= _tid;
		ssrc	= _ssrc;

		return 0;
	}

	void stop_stream_message_impl::print_data()
	{
		dzlog_info("tidL %d, ssrc: %d", _tid, _ssrc);
	}

	void stop_stream_message_impl::init()
	{
		clear();
	}

	void stop_stream_message_impl::clear()
	{
		_tid = _ssrc = 0;
	}

	stop_stream_message_impl::stop_stream_message_impl()
	{
		clear();
	}

	stop_stream_message_impl::~stop_stream_message_impl()
	{
		clear();
	}
}
