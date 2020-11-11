#include "respond_loadbalance_pull_message.h"
#include "respond_loadbalance_pull_message_impl.h"

namespace media_core_message
{
	int respond_loadbalance_pull_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int respond_loadbalance_pull_message::full_data_remote(int sockfd, uint32_t tid, uint32_t length)
	{
		return impl->full_data_remote(sockfd, tid, length);
	}

	int respond_loadbalance_pull_message::full_data_direct(uint32_t tid, const std::vector<uint32_t>& ips, const std::vector<uint16_t>& ports)
	{
		return impl->full_data_direct(tid, ips, ports);
	}

	int respond_loadbalance_pull_message::give_me_data(uint32_t& tid, std::vector<uint32_t>& ips, std::vector<uint16_t>& ports)
	{
		return impl->give_me_data(tid, ips, ports);
	}

	void respond_loadbalance_pull_message::print_data()
	{
		impl->print_data();
	}

	void respond_loadbalance_pull_message::init()
	{
		impl->init();
	}

	void respond_loadbalance_pull_message::clear()
	{
		impl->clear();
	}

	respond_loadbalance_pull_message::respond_loadbalance_pull_message()
	{
		impl = std::make_shared<respond_loadbalance_pull_message_impl>();
	}

	respond_loadbalance_pull_message::~respond_loadbalance_pull_message()
	{
	}
}
