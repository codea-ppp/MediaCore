#ifndef PULL_MEDIA_MENU_MESSAGE_H_
#define PULL_MEDIA_MENU_MESSAGE_H_

#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class pull_media_menu_message_impl;

	class pull_media_menu_message : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid);
		int give_me_data(uint32_t& tid);
		int tell_me_type();
		void print_data();

		pull_media_menu_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<pull_media_menu_message_impl> impl;
	};
}

#endif 
