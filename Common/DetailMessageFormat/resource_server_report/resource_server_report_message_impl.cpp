#include "MessageType.h"
#include "resource_server_report_message_impl.h"

namespace media_core_message
{
	int resource_server_report_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0) 
		{
			dzlog_error("socket < 0");
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		if (_tid == 0)
		{
			dzlog_error("tid not set");
			return ERR_TID_NOT_SET;
		}

		if (_error == 1)
		{
			dzlog_error("error not set");
			return ERR_ERRNO_NOT_SET;
		}

		uint8_t buffer[13];
		((uint32_t*)buffer)[0] = MSGTYPE_RESOURCESERVERREPORT;
		((uint32_t*)buffer)[1] = 1;
		((uint32_t*)buffer)[2] = _tid;
		buffer[12] = _error;

		int once = 0;
		for (int i = 0; i != 13; i += once)
		{
			once = send(sockfd, (const uint8_t*)buffer + i, 13 - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("errno: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int resource_server_report_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		if (sockfd < 0)
		{
			dzlog_error("socket < 0");
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_tid = tid;
		
		uint8_t buffer;

		int once = recv(sockfd, &buffer, 1, MSG_WAITALL);
		if (once != 1)
		{
			clear();

			dzlog_error("not enough data");
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_error = buffer;
		dzlog_info("get error: %d", buffer);

		return 0;
	}

	int resource_server_report_impl::full_data_direct(uint32_t tid, uint8_t error)
	{
		_tid	= tid;
		_error	= error;

		return 0;
	}

	int resource_server_report_impl::give_me_data(uint32_t& tid, uint8_t& error)
	{
		tid		= _tid;
		error	= _error;

		return 0;
	}

	void resource_server_report_impl::print_data()
	{
		dzlog_info("tid: %d, error: %d", _tid, _error);
	}

	void resource_server_report_impl::init()
	{
		clear();
	}

	void resource_server_report_impl::clear()
	{
		_tid	= 0;
		_error	= 1;
	}

	resource_server_report_impl::resource_server_report_impl()
	{
		clear();	
	}

	resource_server_report_impl::~resource_server_report_impl()
	{
		clear();	
	}
}
