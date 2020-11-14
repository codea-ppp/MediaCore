#include "message_type.h"
#include "keepalive_message.h"
#include "keepalive_message_impl.h"

namespace media_core_message
{
	int keepalive_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int keepalive_message::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int keepalive_message::full_data_direct(uint32_t tid, uint32_t sid, uint8_t count)
	{
		return impl->full_data_direct(tid, sid, count);
	}

	int keepalive_message::give_me_data(uint32_t& tid, uint32_t& sid, uint8_t& count)
	{
		return impl->give_me_data(tid, sid, count);
	}
	
	int keepalive_message::tell_me_type()
	{
		return MSGTYPE_KEEPALIVE;
	}

	void keepalive_message::print_data()
	{
		impl->print_data();
	}

	keepalive_message::keepalive_message()
	{
		impl = std::make_shared<keepalive_message_impl>();
	}

	void keepalive_message::init()
	{
		impl->init();
	}

	void keepalive_message::clear()
	{
		impl->clear();
	}
}
