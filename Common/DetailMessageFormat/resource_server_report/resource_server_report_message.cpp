#include "resource_server_report_message.h"
#include "resource_server_report_message_impl.h"

namespace media_core_message
{
	int resource_server_report::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int resource_server_report::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int resource_server_report::full_data_direct(uint32_t tid, uint8_t error)
	{
		return impl->full_data_direct(tid, error);
	}

	int resource_server_report::give_me_data(uint32_t& tid, uint8_t& error)
	{
		return impl->give_me_data(tid, error);
	}

	void resource_server_report::print_data()
	{
		impl->print_data();
	}

	resource_server_report::resource_server_report()
	{
		impl = std::make_shared<resource_server_report_impl>();
	}

	void resource_server_report::init()
	{
		impl->init();
	}

	void resource_server_report::clear()
	{
		impl->clear();
	}
}
