#include "resource_server_report_message.h"
#include "resource_server_report_message_impl.h"

namespace media_core_message
{
	int resource_server_report_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int resource_server_report_message::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int resource_server_report_message::full_data_direct(uint32_t tid, uint8_t error)
	{
		return impl->full_data_direct(tid, error);
	}

	int resource_server_report_message::give_me_data(uint32_t& tid, uint8_t& error)
	{
		return impl->give_me_data(tid, error);
	}

	int resource_server_report_message::tell_me_type()
	{
		return MSGTYPE_RESOURCESERVERREPORT;
	}

	void resource_server_report_message::print_data()
	{
		impl->print_data();
	}

	resource_server_report_message::resource_server_report_message()
	{
		impl = std::make_shared<resource_server_report_message_impl>();
	}

	void resource_server_report_message::init()
	{
		impl->init();
	}

	void resource_server_report_message::clear()
	{
		impl->clear();
	}
}
