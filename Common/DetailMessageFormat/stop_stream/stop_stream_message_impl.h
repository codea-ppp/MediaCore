#ifndef STOP_STREAM_MESSAGE_IMPL_H_
#define STOP_STREAM_MESSAGE_IMPL_H_

#include "MessageType.h"
#include "MessageInerface.h"

namespace media_core_message
{
	class stop_stream_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint32_t ssrc);
		int give_me_data(uint32_t& tid, uint32_t& ssrc);

		void print_data();
		void init();
		void clear();

		stop_stream_message_impl();
		~stop_stream_message_impl();

	private:
		uint32_t _tid;
		uint32_t _ssrc;
	};
}


#endif 
