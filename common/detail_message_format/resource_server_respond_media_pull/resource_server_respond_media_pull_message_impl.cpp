#include "message_type.h"
#include "resource_server_respond_media_pull_message_impl.h"

namespace media_core_message
{
	int resource_server_respond_media_menu_pull_message_impl::send_data_to(int sockfd)
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

		if (_length == 0)
		{
			dzlog_error("video length not set");
			return ERR_VIDEO_LENGTH_NOT_SET;
		}

		if (_width == 0)
		{
			dzlog_error("video width not set");
			return ERR_VIDEO_WIDTH_NOT_SET;
		}

		if (_server_send_port == 0)
		{
			dzlog_error("server send port not set");
			return ERR_SERVER_SEND_PORT_NO_SET;
		}

		uint16_t buffer[11];
		((uint32_t*)buffer)[0] = MSGTYPE_RESOURCERESPONDMEDIAPULL;
		((uint32_t*)buffer)[1] = 2;
		((uint32_t*)buffer)[2] = _tid;
		((uint32_t*)buffer)[3] = _length;
		((uint32_t*)buffer)[4] = _width;
		buffer[10] = _server_send_port;

		int once = 0;
		for (int i = 0; i != 22; i += once)
		{
			once = send(sockfd, (uint8_t*)buffer + i, 22 - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("send error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int resource_server_respond_media_menu_pull_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		if (sockfd < 0)
		{
			dzlog_error("sockfd < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		_tid = tid;
		
		uint16_t buffer[5];

		int once = recv(sockfd, buffer, 10, MSG_WAITALL);
		if (once != 10)
		{
			clear();

			dzlog_error("not enough data");
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_length = ((uint32_t*)buffer)[0];
		_width	= ((uint32_t*)buffer)[1];
		_server_send_port = buffer[4];
		return 0;
	}

	int resource_server_respond_media_menu_pull_message_impl::full_data_direct(uint32_t tid, uint32_t length, uint32_t width, uint16_t server_send_port)
	{
		_tid = tid;
		_length = length;
		_width = width;
		_server_send_port = server_send_port;

		return 0;
	}

	int resource_server_respond_media_menu_pull_message_impl::give_me_data(uint32_t& tid, uint32_t& length, uint32_t& width, uint16_t& server_send_port)
	{
		tid = _tid;
		length = _length;
		width = _width;
		server_send_port = _server_send_port;

		return 0;
	}

	void resource_server_respond_media_menu_pull_message_impl::print_data()
	{
		dzlog_info("tid: %d, size(%d, %d), server send port: %d", _tid, _length, _width, _server_send_port);
	}

	void resource_server_respond_media_menu_pull_message_impl::init()
	{
		clear();
	}

	void resource_server_respond_media_menu_pull_message_impl::clear()
	{
		_tid = _length = _width = _server_send_port = 0;
	}

	resource_server_respond_media_menu_pull_message_impl::resource_server_respond_media_menu_pull_message_impl()
	{
		clear();
	}

	resource_server_respond_media_menu_pull_message_impl::~resource_server_respond_media_menu_pull_message_impl()
	{
		clear();
	}
}

