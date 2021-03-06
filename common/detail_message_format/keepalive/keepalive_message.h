#ifndef KEEP_ALIVE_MESSAGE_H_
#define KEEP_ALIVE_MESSAGE_H_

#include <memory>
#include <stdint.h>
#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class keepalive_message_impl;

	class keepalive_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, uint32_t sid, uint16_t listening_port, uint8_t count);
		int give_me_data(uint32_t& tid, uint32_t& sid, uint16_t& listening_port, uint8_t& count);
		int tell_me_type();
		void print_data();

		keepalive_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<keepalive_message_impl> impl;
	};
}

#endif 
