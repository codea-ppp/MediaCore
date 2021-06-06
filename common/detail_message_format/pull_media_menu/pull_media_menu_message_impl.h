#ifndef PULL_MEDIA_MENU_MESSAGE_IMPL_H_
#define PULL_MEDIA_MENU_MESSAGE_IMPL_H_

#include "message_inerface.h"

namespace media_core_message
{
	class pull_media_menu_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid);
		int give_me_data(uint32_t& tid);

		void print_data();
		void init();
		void clear();

		pull_media_menu_message_impl();
		~pull_media_menu_message_impl();

	private:
		uint32_t _tid;
	};
}

#endif 
