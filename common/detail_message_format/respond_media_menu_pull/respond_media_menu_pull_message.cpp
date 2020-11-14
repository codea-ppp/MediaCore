#include "respond_media_menu_pull_message.h"
#include "respond_media_menu_pull_message_impl.h"

namespace media_core_message
{
	int respond_media_pull_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int respond_media_pull_message::full_data_remote(int sockfd, uint32_t tid, uint32_t length)
	{
		return impl->full_data_remote(sockfd, tid, length);
	}

	int respond_media_pull_message::full_data_direct(uint32_t tid, const std::vector<std::string>& video_names)
	{
		return impl->full_data_direct(tid, video_names);
	}

	int respond_media_pull_message::give_me_data(uint32_t& tid, std::vector<std::string>& video_names)
	{
		return impl->give_me_data(tid, video_names);
	}

	int respond_media_pull_message::tell_me_type()
	{
		return MSGTYPE_RESPONDMEDIAMENU;
	}

	void respond_media_pull_message::print_data()
	{
		impl->print_data();
	}

	respond_media_pull_message::respond_media_pull_message()
	{
		impl = std::make_shared<respond_media_pull_message_impl>();
	}

	respond_media_pull_message::~respond_media_pull_message()
	{
		impl->clear();
	}

	void respond_media_pull_message::init()
	{
		impl->init();
	}

	void respond_media_pull_message::clear()
	{
		impl->clear();
	}
}
