#ifndef RESPOND_LOADBALANCE_PULL_MESSAGE_IMPL_H_
#define RESPOND_LOADBALANCE_PULL_MESSAGE_IMPL_H_

#include <vector>
#include "message_inerface.h"

namespace media_core_message
{
	class respond_loadbalance_pull_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, const std::vector<uint32_t>& sids, const std::vector<uint32_t>& ips, const std::vector<uint16_t>& ports);
		int give_me_data(uint32_t& tid, std::vector<uint32_t>& sids, std::vector<uint32_t>& ips, std::vector<uint16_t>& ports);

		void print_data();
		void init();
		void clear();

		respond_loadbalance_pull_message_impl();
		~respond_loadbalance_pull_message_impl();

	private:
		uint32_t _tid;

		std::vector<uint32_t> _sids;
		std::vector<uint32_t> _ips;
		std::vector<uint16_t> _ports;
	};
}

#endif 
