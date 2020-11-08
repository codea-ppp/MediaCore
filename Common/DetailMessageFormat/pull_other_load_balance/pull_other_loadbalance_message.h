#ifndef PULL_OTHER_LOADBALANCE_MESSAGE_H_
#define PULL_OTHER_LOADBALANCE_MESSAGE_H_

#include "MessageInerface.h"

namespace media_core_message
{
	class pull_other_loadbalance_message_impl;

	class pull_other_loadbalance_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid);
		int give_me_data(uint32_t& tid);

		void print_data();

	private:
		void init();
		void clear();

		pull_other_loadbalance_message();

	private:
		std::shared_ptr<pull_other_loadbalance_message_impl> impl;
	};
}

#endif 
