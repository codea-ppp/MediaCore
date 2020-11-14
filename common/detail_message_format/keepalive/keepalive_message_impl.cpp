#include <zlog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "keepalive_message_impl.h"

namespace media_core_message
{
	int keepalive_message_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0) 
		{
			dzlog_error("sockfd < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		if (_tid == 0)
		{
			dzlog_error("tid not set");
			return ERR_TID_NOT_SET;
		}

		if (_sid == 0)
		{
			dzlog_error("sid not set");
			return ERR_SID_NOT_SET;
		}

		uint8_t buffer[17];
		((uint32_t*)&buffer)[0] = MSGTYPE_KEEPALIVE;
		((uint32_t*)&buffer)[1] = 5;
		((uint32_t*)&buffer)[2] = _tid;
		((uint32_t*)&buffer)[3] = _sid;
		buffer[16] = _count;

		int once = 0;
		for (int i = 0; i != 17; i += once)
		{
			once = send(sockfd, buffer + i, 17 - i, 0);

			if (once == -1)
			{
				dzlog_error("send error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int keepalive_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		_tid = tid;

		uint8_t buffer[5] = { 0 };

		int once = recv(sockfd, buffer, 5, MSG_WAITALL);
		if (once != 5 || once == -1)
		{
			clear();
			
			dzlog_error("recv failed: %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_sid	= ((uint32_t*)buffer)[0];
		_count	= buffer[4];

		return 0;
	}

	int keepalive_message_impl::full_data_direct(uint32_t tid,  uint32_t sid, uint8_t count)
	{
		_tid	= tid;
		_sid	= sid;
		_count	= count;

		return 0;
	}

	int keepalive_message_impl::give_me_data(uint32_t& tid, uint32_t& sid, uint8_t& count)
	{
		tid		= _tid;
		sid		= _sid;
		count	= _count;

		return 0;
	}
	
	void keepalive_message_impl::print_data()
	{
		dzlog_info("tid: %d, sid: %d, count: %d", _tid, _sid, _count);
	}

	void keepalive_message_impl::init()
	{
		clear();
	}

	void keepalive_message_impl::clear()
	{
		_tid	= 0x00;
		_sid	= 0x00;
		_count	= 0x00;
	}

	keepalive_message_impl::keepalive_message_impl()
	{
		clear();
	}

	keepalive_message_impl::~keepalive_message_impl()
	{
		clear();
	}
}
