#ifndef PUSH_MEDIA_MENU_PULL_MESSAGE_IMPL_H_
#define PUSH_MEDIA_MENU_PULL_MESSAGE_IMPL_H_

#include <vector>
#include <string>
#include "MessageInerface.h"

namespace media_core_message
{
	class push_media_pull_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid, uint32_t length);
		int full_data_direct(uint32_t tid, const std::vector<std::string>& video_names);
		int give_me_data(uint32_t& tid, std::vector<std::string>& video_names);

		void print_data();

		push_media_pull_message_impl();
		~push_media_pull_message_impl();

		void init();
		void clear();

	private:
		uint32_t _tid;
		uint32_t _length;

		std::vector<std::string> _video_names;
	};
}

#endif 
