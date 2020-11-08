#ifndef STREAM_MESSAGE_H_
#define STREAM_MESSAGE_H_

#include "MessageType.h"
#include "MessageInerface.h"

namespace media_core_message
{
	class stream_message_impl;

	class stream_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint8_t* YUV[3], uint32_t YUV_length[3]);
		int give_me_data(uint32_t& tid, uint8_t* must_be_nullptr[3], uint32_t YUV_length[3]);

		void print_data();

		stream_message();
		~stream_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<stream_message_impl> impl;
	};
}

#endif 
