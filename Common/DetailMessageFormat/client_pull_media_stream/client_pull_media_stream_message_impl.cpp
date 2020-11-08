#include <zlog.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "MessageType.h"
#include "client_pull_media_stream_message_impl.h"

namespace media_core_message
{
	int client_pull_media_stream_mesage_impl::send_data_to(int sockfd)
	{
		if (_video_name.size() == 0)
		{
			dzlog_error("no video name");
			return ERR_VIDEO_NAME_NOT_SET;
		}

		if (_receive_port == 0)
		{
			dzlog_error("receive_port not set");
			return ERR_RECV_PORT_NOT_SET;	
		}

		if (_tid == 0)
		{
			dzlog_error("tid not set");
			return ERR_TID_NOT_SET;
		}

		uint8_t head_buffer[16]		 = { 0 };
		((uint32_t*)&head_buffer)[0] = MSGTYPE_CLIENTPULLMEDIASTREAM;
		((uint32_t*)&head_buffer)[1] = _video_name.size() + 4;
		((uint32_t*)&head_buffer)[2] = _tid;
		((uint32_t*)&head_buffer)[3] = (_receive_port << 16) + _video_name.size();
 
		int once = 0;
		for (int i = 0; i < 16; i += once)
		{
			once = send(sockfd, head_buffer + i, 16 - i, 0);
			if (once == -1)
			{
				clear();
			
				dzlog_error("send data error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		once = 0;
		for (unsigned int i = 0; i < _video_name.size(); i += once)
		{
			once = send(sockfd, _video_name.c_str() + i, _video_name.size() - i, 0);
			if (once == -1)
			{
				clear();
			
				dzlog_error("send data error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int client_pull_media_stream_mesage_impl::full_data_remote(int sockfd, uint32_t tid) 
	{
		if (sockfd < 0) 
		{
			clear();

			dzlog_error("socketfd: %d", sockfd);
			return ERR_SOCKET_FD_NEGATIVE;
		}

		_tid = tid;

		uint32_t buffer;

		int once = 0;
		once = recv(sockfd, (void*)&buffer, 4, MSG_WAITALL);
		if (once == -1)
		{
			clear();
			
			dzlog_error("error, errno: %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		uint16_t receive_port		= buffer >> 16;
		uint16_t video_name_length	= buffer & 0x0000FFFF;

		if (video_name_length > 512)
		{
			clear();
			
			dzlog_error("video name too long: %d", video_name_length);
			return ERR_VIDEO_NAME_TOO_LONG;
		}
		
		char* video_name_buffer = new char[video_name_length + 1];
		video_name_buffer[video_name_length] = 0;

		once = recv(sockfd, video_name_buffer, video_name_length, MSG_WAITALL);
		if (once == -1 || once < video_name_length)
		{
			clear();
			dzlog_error("error, errno: %d", errno);
			
			delete[] video_name_buffer;
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		dzlog_info("get video: %s", video_name_buffer);

		_video_name.resize(video_name_length);
		_video_name		= video_name_buffer;
		_receive_port	= receive_port;

		delete[] video_name_buffer;
		return 0;
	}

	int client_pull_media_stream_mesage_impl::full_data_direct(unsigned tid, const std::string& media2play, uint16_t receive_port)
	{
		_receive_port	= receive_port;
		_video_name		= media2play;
		_tid			= tid;

		dzlog_info("set pull video: %s:%d", media2play.c_str(), receive_port);
		return 0;
	}

	int client_pull_media_stream_mesage_impl::give_me_data(uint32_t& tid, std::string& video_name, uint16_t& receive_port)
	{
		receive_port	= _receive_port;
		video_name		= _video_name;
		tid				= _tid;

		return 0;

	}
	
	void client_pull_media_stream_mesage_impl::print_data()
	{
		dzlog_info("pull video: %s:%d", _video_name.c_str(), _receive_port);
	}

	client_pull_media_stream_mesage_impl::client_pull_media_stream_mesage_impl()
	{
		clear();
	}

	client_pull_media_stream_mesage_impl::~client_pull_media_stream_mesage_impl()
	{
		clear();
	}
	
	void client_pull_media_stream_mesage_impl::init()
	{
		clear();
	}

	void client_pull_media_stream_mesage_impl::clear()
	{
		_video_name.clear();
		_receive_port = 0;
		_tid = 0;
		
		dzlog_info("clear message");
	}
}
