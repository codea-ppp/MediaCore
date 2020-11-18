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

		uint8_t buffer[19];
		((uint32_t*)&buffer)[0] = MSGTYPE_KEEPALIVE;
		((uint32_t*)&buffer)[1] = 7;
		((uint32_t*)&buffer)[2] = _tid;
		((uint32_t*)&buffer)[3] = _sid;
		((uint16_t*)&buffer)[8] = _listening_port;
		buffer[18] = _count;

		int once = 0;
		for (int i = 0; i != 19; i += once)
		{
			once = send(sockfd, buffer + i, 19 - i, 0);

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

		uint8_t buffer[7] = { 0 };

		int once = recv(sockfd, buffer, 7, MSG_WAITALL);
		if (once != 7)
		{
			clear();
			
			dzlog_error("recv failed: %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_sid				= ((uint32_t*)buffer)[0];
		_listening_port		= ((uint16_t*)buffer)[2];
		_count				= buffer[6];

		return 0;
	}

	int keepalive_message_impl::full_data_direct(uint32_t tid,  uint32_t sid, uint16_t listening_port, uint8_t count)
	{
		_tid				= tid;
		_sid				= sid;
		_listening_port		= listening_port;
		_count				= count;

		return 0;
	}

	int keepalive_message_impl::give_me_data(uint32_t& tid, uint32_t& sid, uint16_t& listening_port, uint8_t& count)
	{
		tid					= _tid;
		sid					= _sid;
		listening_port		= _listening_port;
		count				= _count;

		return 0;
	}
	
	void keepalive_message_impl::print_data()
	{
		dzlog_info("tid: %d, sid: %d, listening port: %d, count: %d", _tid, _sid, _listening_port, _count);
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
