#ifndef RESOURCE_SERVER_RESPOND_PULL_MESSAGE_IMPL_H_
#define RESOURCE_SERVER_RESPOND_PULL_MESSAGE_IMPL_H_

#include <stdint.h>
#include "message_inerface.h"

namespace media_core_message
{
	class resource_server_respond_media_menu_pull_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint32_t length, uint32_t width, uint16_t server_send_port);
		int give_me_data(uint32_t& tid, uint32_t& length, uint32_t& width, uint16_t& server_send_port);

		void print_data();
		void init();
		void clear();

		resource_server_respond_media_menu_pull_message_impl();
		~resource_server_respond_media_menu_pull_message_impl();

	private:
		uint32_t _tid;
		uint32_t _length;
		uint32_t _width;
		uint16_t _server_send_port;
	};
}

#endif 
