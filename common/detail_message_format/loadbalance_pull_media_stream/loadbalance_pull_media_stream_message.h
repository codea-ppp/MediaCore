#ifndef LOADBALANCE_PULL_MEDIA_STREAM_MESSAGE_H_
#define LOADBALANCE_PULL_MEDIA_STREAM_MESSAGE_H_

#include <memory>
#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class loadbalance_pull_media_stream_message_impl;

	class loadbalance_pull_media_stream_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, uint16_t client_recv_port, uint32_t ssrc, const std::string& video_name);
		int give_me_data(uint32_t& tid, uint16_t& client_recv_port, uint32_t& ssrc, std::string& video_name);
		int tell_me_type();
		void print_data();

		loadbalance_pull_media_stream_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<loadbalance_pull_media_stream_message_impl> impl;
	};
}

#endif 
