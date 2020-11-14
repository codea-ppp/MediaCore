#ifndef KEEP_ALIVE_MESSAGE_IMPL_H_
#define KEEP_ALIVE_MESSAGE_IMPL_H_

#include <stdint.h>
#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class keepalive_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint32_t sid, uint8_t count);
		int give_me_data(uint32_t& tid, uint32_t& sid, uint8_t& count);

		void print_data();
		void init();
		void clear();

		keepalive_message_impl();
		~keepalive_message_impl();

	private:
		uint32_t _tid;
		uint32_t _sid;
		uint8_t _count;
	};
}

#endif 
