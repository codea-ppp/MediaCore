#include "pull_media_menu_message.h"
#include "pull_media_menu_message_impl.h"

namespace media_core_message
{
	int pull_media_menu_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int pull_media_menu_message::full_data_remote(int sockfd, uint32_t tid)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int pull_media_menu_message::full_data_direct(uint32_t tid)
	{
		return impl->full_data_direct(tid);
	}

	int pull_media_menu_message::give_me_data(uint32_t& tid)
	{
		return impl->give_me_data(tid);
	}

	void pull_media_menu_message::print_data()
	{
		impl->print_data();
	}

	void pull_media_menu_message::init()
	{
		impl->init();
	}

	void pull_media_menu_message::clear()
	{
		impl->clear();
	}

	pull_media_menu_message::pull_media_menu_message()
	{
		impl = std::make_shared<pull_media_menu_message_impl>();
	}
}
