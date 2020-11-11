#ifndef CLIENT_STREAM_TRIGGER_MESSAGE_H_
#define CLIENT_STREAM_TRIGGER_MESSAGE_H_

#include <memory>
#include <stdint.h>
#include "MessageType.h"
#include "MessageInerface.h"

namespace media_core_message
{
	class client_stream_trigger_message_impl;

	class client_stream_trigger_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, uint32_t ssrc);
		int give_me_data(uint32_t& tid, uint32_t& ssrc);

		void print_data();

		client_stream_trigger_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<client_stream_trigger_message_impl> impl;
	};
}

#endif 
