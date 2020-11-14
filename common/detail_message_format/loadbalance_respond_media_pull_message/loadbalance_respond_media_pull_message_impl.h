#ifndef LOADBALANCE_RESPOND_MEDIA_PULL_MESSAGE_IMPL_H_
#define LOADBALANCE_RESPOND_MEDIA_PULL_MESSAGE_IMPL_H_

#include <stdint.h>

namespace media_core_message
{
	class loadbalance_respond_media_pull_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint32_t ssrc, uint16_t server_send_port);
		int give_me_data(uint32_t& tid, uint32_t& ssrc, uint16_t& server_send_port);

		void print_data();
		void init();
		void clear();

		loadbalance_respond_media_pull_message_impl();
		~loadbalance_respond_media_pull_message_impl();

	private:
		uint32_t _tid;
		uint32_t _ssrc;
		uint16_t _server_send_port;
	};
}

#endif 
