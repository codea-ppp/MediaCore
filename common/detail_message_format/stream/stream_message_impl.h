#ifndef STREAM_MESSAGE_IMPL_H_
#define STREAM_MESSAGE_IMPL_H_

#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class stream_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint8_t* YUV[3], uint32_t YUV_length[3], uint32_t height);
		int give_me_data(uint32_t& tid, uint8_t* must_be_nullptr[3], uint32_t YUV_length[3]);

		void print_data();
		void init();
		void clear();

		stream_message_impl();
		~stream_message_impl();

	private:
		uint32_t _tid;
		uint32_t _height;
		uint32_t _y_length;
		uint32_t _u_length;
		uint32_t _v_length;
		uint8_t* _y;
		uint8_t* _u;
		uint8_t* _v;
	};
}

#endif 
