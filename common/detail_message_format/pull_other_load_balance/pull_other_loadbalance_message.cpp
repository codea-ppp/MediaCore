#include "pull_other_loadbalance_message.h"
#include "pull_other_loadbalance_message_impl.h"

namespace media_core_message
{
	int pull_other_loadbalance_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int pull_other_loadbalance_message::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int pull_other_loadbalance_message::full_data_direct(uint32_t tid)
	{
		return impl->full_data_direct(tid);
	}

	int pull_other_loadbalance_message::give_me_data(uint32_t& tid)
	{
		return impl->give_me_data(tid);
	}

	int pull_other_loadbalance_message::tell_me_type()
	{
		return MSGTYPE_PULLOTHERLOADBALANCE;
	}

	void pull_other_loadbalance_message::print_data()
	{
		impl->print_data();
	}

	void pull_other_loadbalance_message::init()
	{
		impl->init();
	}

	void pull_other_loadbalance_message::clear()
	{
		impl->clear();
	}

	pull_other_loadbalance_message::pull_other_loadbalance_message()
	{
		impl = std::make_shared<pull_other_loadbalance_message_impl>();
	}
}
