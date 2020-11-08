#include <vector>
#include <zlog.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MessageType.h"
#include "loadbalance_pull_media_stream_message_impl.h"

namespace media_core_message
{
	int loadbalance_pull_media_stream_message_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0)
		{
			dzlog_error("sockfd < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		if (_client_receive_port == 0)
		{
			dzlog_error("client receive port not set");
			return ERR_CLIENT_RECV_PORT_NO_SET;
		}

		if (_ssrc == 0)
		{
			dzlog_error("ssrc not set");
			return ERR_SSRC_NOT_SET;
		}

		if (_video_name.empty())
		{
			dzlog_error("video not set");
			return ERR_VIDEO_NOT_SET;
		}

		if (_tid == 0)
		{
			dzlog_error("tid not set");
			return ERR_TID_NOT_SET;
		}

		uint32_t head_buffer[5] = { 0 };
		head_buffer[0] = MSGTYPE_LOADBALANCEPULLMEDIASTREAM;
		head_buffer[1] = 8 + _video_name.size();
		head_buffer[2] = _tid;
		head_buffer[3] = _ssrc;
		head_buffer[4] = (_client_receive_port << 16) + _video_name.size();

		int once = 0;
		for (int i = 0; i != 20; i += once)
		{
			once = send(sockfd, (uint8_t*)head_buffer + i, 20 - i, 0);
			if (once == -1)
			{
				dzlog_error("send error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		once = 0;
		unsigned int name_length = _video_name.size();
		for (unsigned int i = 0; i != name_length; i += once)
		{
			once = send(sockfd, _video_name.c_str() + i, name_length - i, 0);
			if (once == -1)
			{
				dzlog_error("error, errno: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int loadbalance_pull_media_stream_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		if (sockfd < 0)
		{
			dzlog_error("sockfd < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		uint32_t head_buffer[2];

		int once = recv(sockfd, (void*)head_buffer, 8, MSG_WAITALL);
		if (once != 8)
		{
			clear();

			dzlog_error("recv data error: %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_tid = tid;
		_ssrc = head_buffer[0];
		_client_receive_port = head_buffer[1] >> 16;

		uint16_t video_name_length = head_buffer[1] & 0x0000FFFF;
		uint8_t* video_name = new uint8_t[video_name_length];

		once = recv(sockfd, video_name, video_name_length, MSG_WAITALL);
		if (once != video_name_length)
		{
			dzlog_error("recv error: %d", errno);

			clear();

			delete[] video_name;
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_video_name = (char*)video_name;

		dzlog_info("get video: %s", (char*)video_name);
		delete[] video_name;
		return 0;
	}

	int loadbalance_pull_media_stream_message_impl::full_data_direct(uint32_t tid, uint16_t client_recv_port, uint32_t ssrc, const std::string& media2play)
	{
		unsigned int size = media2play.size();
		if (size == 0)
		{
			dzlog_error("try to play a video without name");
			return ERR_VIDEO_NO_NAME;
		}

		_client_receive_port = client_recv_port;
		_video_name = media2play;
		_ssrc 	= ssrc;
		_tid 	= tid;

		return 0;
	}
	
	int loadbalance_pull_media_stream_message_impl::give_me_data(uint32_t& tid, uint16_t& client_recv_port, uint32_t& ssrc, std::string& video_name)
	{
		video_name			= _video_name;
		client_recv_port	= _client_receive_port;
		
		ssrc	= _ssrc;
		tid		=  _tid;

		return 0;
	}

	void loadbalance_pull_media_stream_message_impl::print_data()
	{
		dzlog_info("ssrc: %d, client receive port: %d, video: %s", _ssrc, _client_receive_port, _video_name.c_str());
	}

	void loadbalance_pull_media_stream_message_impl::init()
	{
		clear();
	}

	void loadbalance_pull_media_stream_message_impl::clear()
	{
		_client_receive_port = 0;
		_ssrc = 0;
		_tid = 0;
	}

	loadbalance_pull_media_stream_message_impl::loadbalance_pull_media_stream_message_impl()
	{
		clear();
	}

	loadbalance_pull_media_stream_message_impl::~loadbalance_pull_media_stream_message_impl()
	{
		clear();
	}
}
