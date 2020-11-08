#ifndef RESPOND_MEDIA_MENU_PULL_MESSAGE_H_
#define RESPOND_MEDIA_MENU_PULL_MESSAGE_H_

#include <vector>
#include <string>
#include "MessageInerface.h"

namespace media_core_message
{
	class respond_media_pull_message_impl;

	class respond_media_pull_message: public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, const std::vector<std::string>& video_names);
		int give_me_data(uint32_t& tid, std::vector<std::string>& video_names);

		void print_data();

		respond_media_pull_message();
		~respond_media_pull_message();

	private:
		void init();
		void clear();

	private:
		std::shared_ptr<respond_media_pull_message_impl> impl;
	};
}

#endif 
