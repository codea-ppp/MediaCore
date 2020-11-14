#ifndef CLIENT_PULL_MEDIA_STREAM_MESSAGE_H_
#define CLIENT_PULL_MEDIA_STREAM_MESSAGE_H_

#include <memory>
#include <string>
#include <stdint.h>
#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class client_pull_media_stream_mesage_impl;

	class client_pull_media_stream_mesage : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, const std::string& media2play, uint16_t receive_port);
		int give_me_data(uint32_t& tid, std::string& video_name, uint16_t& receive_port);
		int tell_me_type();
		void print_data();

		client_pull_media_stream_mesage();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<client_pull_media_stream_mesage_impl> impl;
	};
}

#endif 
