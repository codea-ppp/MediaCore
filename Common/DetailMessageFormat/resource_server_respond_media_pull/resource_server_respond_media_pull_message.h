#ifndef RESOURCE_SERVER_RESPOND_PULL_MESSAGE_H_
#define RESOURCE_SERVER_RESPOND_PULL_MESSAGE_H_

#include "MessageInerface.h"

namespace media_core_message
{
	class resource_server_respond_media_pull_message_impl;

	class resource_server_respond_media_pull_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint16_t server_send_port);
		int give_me_data(uint32_t& tid, uint16_t& server_send_port);

		void print_data();

	private:
		void init();
		void clear();

		resource_server_respond_media_pull_message();
		~resource_server_respond_media_pull_message();

	private:
		std::shared_ptr<resource_server_respond_media_pull_message_impl> impl;
	};
}

#endif 
