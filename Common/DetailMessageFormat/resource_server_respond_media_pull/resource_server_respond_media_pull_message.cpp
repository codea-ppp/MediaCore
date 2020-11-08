#include "resource_server_respond_media_pull_message.h"
#include "resource_server_respond_media_pull_message_impl.h"

namespace media_core_message
{
	int resource_server_respond_media_pull_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int resource_server_respond_media_pull_message::full_data_remote(int sockfd, uint32_t tid)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int resource_server_respond_media_pull_message::full_data_direct(uint32_t tid, uint16_t server_send_port)
	{
		return impl->full_data_direct(tid, server_send_port);
	}

	int resource_server_respond_media_pull_message::give_me_data(uint32_t& tid, uint16_t& server_send_port)
	{
		return impl->give_me_data(tid, server_send_port);
	}

	void resource_server_respond_media_pull_message::print_data()
	{
		impl->print_data();
	}

	void resource_server_respond_media_pull_message::init()
	{
		impl->init();
	}

	void resource_server_respond_media_pull_message::clear()
	{
		impl->clear();
	}

	resource_server_respond_media_pull_message::resource_server_respond_media_pull_message()
	{
		impl = std::make_shared<resource_server_respond_media_pull_message_impl>();
	}

	resource_server_respond_media_pull_message::~resource_server_respond_media_pull_message()
	{
	}
}
