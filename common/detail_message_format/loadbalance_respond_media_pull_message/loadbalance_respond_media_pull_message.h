#ifndef LOADBALANCE_RESPOND_MEDIA_PULLING_MESSAGE_H_
#define LOADBALANCE_RESPOND_MEDIA_PULLING_MESSAGE_H_

#include <memory>
#include <zlog.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class loadbalance_respond_media_pull_message_impl;

	class loadbalance_respond_media_pull_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, uint32_t ssrc, uint32_t length, uint32_t width, uint16_t server_send_port);
		int give_me_data(uint32_t& tid, uint32_t& ssrc, uint32_t& length, uint32_t& width, uint16_t& server_send_port);
		int tell_me_type();
		void print_data();

		loadbalance_respond_media_pull_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<loadbalance_respond_media_pull_message_impl> impl;
	};
}

#endif 
