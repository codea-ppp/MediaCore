#include <zlog.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "message_type.h"
#include "loadbalance_respond_media_pull_message_impl.h"

namespace media_core_message
{
	int loadbalance_respond_media_pull_message_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0) 
		{
			dzlog_error("socket < 0");
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
		
		if (_server_send_port == 0)
		{
			dzlog_error("server send port not set");
			return ERR_SERVER_SEND_PORT_NO_SET;
		}

		uint16_t buffer[13];
		((uint32_t*)buffer)[0] = MSGTYPE_LOADBALANCERESPONDMEDIAPULL; 
		((uint32_t*)buffer)[1] = 6;
		((uint32_t*)buffer)[2] = _tid;
		((uint32_t*)buffer)[3] = _ssrc;
		((uint32_t*)buffer)[4] = _length;
		((uint32_t*)buffer)[5] = _width;
		buffer[12] = _server_send_port;

		int once = 0;
		for (int i = 0; i != 26; i += once)
		{
			once = send(sockfd, (uint8_t*)buffer + i, 26 - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("errno: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int loadbalance_respond_media_pull_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		uint16_t buffer[7];
		int once = recv(sockfd, (uint8_t*)buffer, 14, MSG_WAITALL);
		if (once != 14)
		{
			dzlog_error("no enough data");

			clear();

			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_ssrc	= ((uint32_t*)buffer)[0];
		_length = ((uint32_t*)buffer)[1];
		_width	= ((uint32_t*)buffer)[2];
		_server_send_port = buffer[6];

		_tid = tid;

		return 0;
	}

	int loadbalance_respond_media_pull_message_impl::full_data_direct(uint32_t tid, uint32_t ssrc, uint32_t length, uint32_t width, uint16_t server_send_port)
	{
		_tid	= tid;
		_ssrc	= ssrc;
		_length = length;
		_width	= width;

		_server_send_port = server_send_port;

		return 0;
	}

	int loadbalance_respond_media_pull_message_impl::give_me_data(uint32_t& tid, uint32_t& ssrc, uint32_t& length, uint32_t& width, uint16_t& server_send_port)
	{
		tid		= _tid;
		ssrc	= _ssrc;
		length	= _length;
		width	= _width;

		server_send_port = _server_send_port;

		return 0;
	}

	void loadbalance_respond_media_pull_message_impl::print_data()
	{
		dzlog_info("tid: %d, ssrc: %d, size(%d, %d), server send port: %d", _tid, _ssrc, _length, _width, _server_send_port);
	}

	void loadbalance_respond_media_pull_message_impl::init()
	{
		clear();
	}

	void loadbalance_respond_media_pull_message_impl::clear()
	{
		_tid	= 0;
		_ssrc	= 0;
		_length = 0;
		_width	= 0;

		_server_send_port = 0;
	}

	loadbalance_respond_media_pull_message_impl::loadbalance_respond_media_pull_message_impl()
	{
		clear();
	}

	loadbalance_respond_media_pull_message_impl::~loadbalance_respond_media_pull_message_impl()
	{
		clear();
	}
}
