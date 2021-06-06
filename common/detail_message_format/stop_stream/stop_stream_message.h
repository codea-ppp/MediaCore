#ifndef STOP_STREAM_H_
#define STOP_STREAM_H_

#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class stop_stream_message_impl;

	class stop_stream_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, uint32_t ssrc);
		int give_me_data(uint32_t& tid, uint32_t& ssrc);
		int tell_me_type();
		void print_data();

		stop_stream_message();
		~stop_stream_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<stop_stream_message_impl> impl;
	};
}

#endif 
