#include "message_type.h"
#include "pull_other_loadbalance_message_impl.h"

namespace media_core_message
{
	int pull_other_loadbalance_message_impl::send_data_to(int sockfd)
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

		uint32_t head_buffer[3];
		head_buffer[0] = MSGTYPE_PULLMEDIAMENU;
		head_buffer[1] = 0;
		head_buffer[2] = _tid;

		int once = 0;
		for (int i = 0; i != 12; i += once)
		{
			once = send(sockfd, (uint8_t*)head_buffer + i, 12 - i, 0);
			if (once == -1)
			{
				dzlog_error("send error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int pull_other_loadbalance_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		_tid = tid;
		return 0;
	}

	int pull_other_loadbalance_message_impl::full_data_direct(uint32_t tid)
	{
		_tid = tid;
		return 0;
	}

	int pull_other_loadbalance_message_impl::give_me_data(uint32_t& tid)
	{
		tid = _tid;
		return 0;
	}

	void pull_other_loadbalance_message_impl::print_data()
	{
		dzlog_info("tid: %d", _tid);
	}
	
	void pull_other_loadbalance_message_impl::init()
	{
		clear();
	}

	void pull_other_loadbalance_message_impl::clear()
	{
		_tid = 0;
	}

	pull_other_loadbalance_message_impl::pull_other_loadbalance_message_impl()
	{
		clear();
	}

	pull_other_loadbalance_message_impl::~pull_other_loadbalance_message_impl()
	{
		clear();
	}
}

