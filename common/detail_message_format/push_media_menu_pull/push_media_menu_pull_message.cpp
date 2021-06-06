#include "push_media_menu_pull_message.h"
#include "push_media_menu_pull_message_impl.h"

namespace media_core_message
{
	int push_media_pull_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int push_media_pull_message::full_data_remote(int sockfd, uint32_t tid, uint32_t length)
	{
		return impl->full_data_remote(sockfd, tid, length);
	}

	int push_media_pull_message::full_data_direct(uint32_t tid, const std::vector<std::string>& video_names)
	{
		return impl->full_data_direct(tid, video_names);
	}

	int push_media_pull_message::give_me_data(uint32_t& tid, std::vector<std::string>& video_names)
	{
		return impl->give_me_data(tid, video_names);
	}

	int push_media_pull_message::tell_me_type()
	{
		return MSGTYPE_PUSHMEDIAMENU;
	}

	void push_media_pull_message::print_data()
	{
		impl->print_data();
	}

	push_media_pull_message::push_media_pull_message()
	{
		impl = std::make_shared<push_media_pull_message_impl>();
	}

	push_media_pull_message::~push_media_pull_message()
	{
		impl->clear();
	}

	void push_media_pull_message::init()
	{
		impl->init();
	}

	void push_media_pull_message::clear()
	{
		impl->clear();
	}
}
