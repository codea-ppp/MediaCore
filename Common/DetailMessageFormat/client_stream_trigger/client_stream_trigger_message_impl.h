#ifndef CLIENT_STREAM_TRIGGER_MESSAGE_IMPL_H_
#define CLIENT_STREAM_TRIGGER_MESSAGE_IMPL_H_

#include <stdint.h>
#include "MessageType.h"
#include "MessageInerface.h"

namespace media_core_message
{
	class client_stream_trigger_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint32_t ssrc);
		int give_me_data(uint32_t& tid, uint32_t& ssrc);

		void print_data();
		void init();
		void clear();

		client_stream_trigger_message_impl();
		~client_stream_trigger_message_impl();

	private:
		uint32_t _ssrc;
		uint32_t _tid;
	};
}

#endif 
