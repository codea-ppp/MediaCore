#include "MessageType.h"
#include "respond_media_menu_pull_message_impl.h"

namespace media_core_message
{
	int respond_media_pull_message_impl::send_data_to(int sockfd)
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
			dzlog_error("length not set");
			return ERR_LENGTH_NEGATIVE;
		}

		uint32_t head_length = 12 + _video_names.size() * 4;	// for uint8_t
		uint32_t* head_buffer = new uint32_t[head_length / 4];

		head_buffer[0] = MSGTYPE_RESPONDMEDIAMENU;
		head_buffer[1] = _video_names.size();
		head_buffer[2] = _tid;

		for (unsigned int i = 0; i < _video_names.size(); ++i)
		{
			head_buffer[3 + i] = _video_names[i].size();
		}

		int once = 0;
		for (unsigned int i = 0; i != head_length; i += once)
		{
			once = send(sockfd, (uint8_t*)head_buffer, head_length, 0);
			if (once == -1)
			{
				clear();

				delete[] head_buffer;

				dzlog_error("send failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		delete[] head_buffer;

		for (unsigned int i = 0; i < _video_names.size(); ++i)
		{
			once = 0;
			for (unsigned int j = 0; j != _video_names[i].size(); ++j)
			{
				once = send(sockfd, _video_names[i].c_str() + j, _video_names[i].size() - j, 0);
				if (once == -1)
				{
					clear();

					dzlog_error("send failed: %d", errno);
					return ERR_NEED_2_CLOSE_SOCKET;
				}
			}
		}

		return 0;
	}

	int respond_media_pull_message_impl::full_data_remote(int sockfd, uint32_t tid, uint32_t length)
	{
		if (sockfd < 0)
		{
			dzlog_error("sockfd < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		if (length < 0)
		{
			clear();

			dzlog_error("length < 0");
			return ERR_LENGTH_NEGATIVE;
		}

		_tid		= tid;
		_length		= length;

		uint32_t* video_name_length_buffer = new uint32_t[length];
		unsigned int once = recv(sockfd, (uint8_t*)video_name_length_buffer, length * 4, MSG_WAITALL);
		if (once != length * 4)
		{
			delete[] video_name_length_buffer;

			clear();
			
			dzlog_error("recv failed; %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		uint32_t video_names_total_length = 0;
		for (unsigned int i = 0; i < length; ++i)
		{
			dzlog_info("get a video of %d length", video_name_length_buffer[i]);
			video_names_total_length += video_name_length_buffer[i];
		}

		dzlog_info("video total length; %d", video_names_total_length);

		uint8_t* video_names_buffer = new uint8_t[video_names_total_length];
		once = recv(sockfd, video_names_buffer, video_names_total_length, MSG_WAITALL);
		if (once != video_names_total_length)
		{
			delete[] video_names_buffer;
			delete[] video_name_length_buffer;

			clear();

			dzlog_error("recv failed: %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		uint32_t pos = 0;
		std::string temp;

		for (unsigned int i = 0; i < length; ++i)
		{
			uint32_t this_time = video_name_length_buffer[i];

			temp.clear();
			temp.assign((char*)video_names_buffer, pos, this_time);

			pos += this_time;

			dzlog_info("get a video: %s", temp.c_str());
			_video_names.push_back(temp);
		}

		delete[] video_names_buffer;
		delete[] video_name_length_buffer;
		return 0;
	}

	int respond_media_pull_message_impl::full_data_direct(uint32_t tid, const std::vector<std::string>& video_names)
	{
		if (!video_names.size())
		{
			dzlog_error("set not video names");
			return ERR_SET_NO_VIDEO_NAMES;
		}

		_tid = tid;
		_video_names = video_names;

		return 0;
	}

	int respond_media_pull_message_impl::give_me_data(uint32_t& tid, std::vector<std::string>& video_names)
	{
		tid = _tid;
		video_names = _video_names;

		return 0;
	}

	void respond_media_pull_message_impl::print_data()
	{
		dzlog_info("tid: %d, length: %d", _tid, _length);

		for (unsigned int i = 0; i < _video_names.size(); ++i)
		{
			dzlog_info("video: %s", _video_names[i].c_str());
		}
	}

	respond_media_pull_message_impl::respond_media_pull_message_impl()
	{
		clear();
	}

	respond_media_pull_message_impl::~respond_media_pull_message_impl()
	{
		clear();
	}

	void respond_media_pull_message_impl::init()
	{
		clear();
	}

	void respond_media_pull_message_impl::clear()
	{
		_tid = _length = 0;
		_video_names.clear();
	}
}
