#include "loadbalance_respond_media_pull_message.h"
#include "loadbalance_respond_media_pull_message_impl.h"

namespace media_core_message
{
	int loadbalance_respond_media_pull_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int loadbalance_respond_media_pull_message::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int loadbalance_respond_media_pull_message::full_data_direct(uint32_t tid, uint32_t ssrc, uint16_t server_send_port)
	{
		return impl->full_data_direct(tid, ssrc, server_send_port);
	}

	int loadbalance_respond_media_pull_message::give_me_data(uint32_t& tid, uint32_t& ssrc, uint16_t& server_send_port)
	{
		return impl->give_me_data(tid, ssrc, server_send_port);
	}

	int loadbalance_respond_media_pull_message::tell_me_type()
	{
		return MSGTYPE_LOADBALANCERESPONDMEDIAPULL;
	}

	void loadbalance_respond_media_pull_message::print_data()
	{
		impl->print_data();
	}

	void loadbalance_respond_media_pull_message::init()
	{
		impl->init();
	}

	void loadbalance_respond_media_pull_message::clear()
	{
		impl->clear();
	}
	
	loadbalance_respond_media_pull_message::loadbalance_respond_media_pull_message()
	{
		impl = std::make_shared<loadbalance_respond_media_pull_message_impl>();
	}
}

