#ifndef PUSH_MEDIA_MENU_PULL_MESSAGE_H_
#define PUSH_MEDIA_MENU_PULL_MESSAGE_H_

#include <vector>
#include <string>
#include "message_type.h"
#include "message_inerface.h"

namespace media_core_message
{
	class push_media_pull_message_impl;

	class push_media_pull_message: public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, const std::vector<std::string>& video_names);
		int give_me_data(uint32_t& tid, std::vector<std::string>& video_names);
		int tell_me_type();
		void print_data();

		push_media_pull_message();
		~push_media_pull_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<push_media_pull_message_impl> impl;
	};
}

#endif 
