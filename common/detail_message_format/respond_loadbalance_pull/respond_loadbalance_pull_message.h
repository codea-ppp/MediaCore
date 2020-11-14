#ifndef RESPOND_LOADBALANCE_PULL_MESSAGE_H_
#define RESPOND_LOADBALANCE_PULL_MESSAGE_H_

#include <vector>
#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class respond_loadbalance_pull_message_impl;

	class respond_loadbalance_pull_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, const std::vector<uint32_t>& ips, const std::vector<uint16_t>& ports);
		int give_me_data(uint32_t& tid, std::vector<uint32_t>& ips, std::vector<uint16_t>& ports);
		int tell_me_type();
		void print_data();
		void init();
		void clear();

		respond_loadbalance_pull_message();
		~respond_loadbalance_pull_message();

	private:
		std::shared_ptr<respond_loadbalance_pull_message_impl> impl;
	};
}

#endif 
