#include "MessageType.h"
#include "client_stream_trigger_message.h"
#include "client_stream_trigger_message_impl.h"

namespace media_core_message
{
	int client_stream_trigger_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int client_stream_trigger_message::full_data_remote(int sockfd, uint32_t tid)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int client_stream_trigger_message::full_data_direct(uint32_t tid, uint32_t ssrc)
	{
		return impl->full_data_direct(tid, ssrc);
	}

	int client_stream_trigger_message::give_me_data(uint32_t& tid, uint32_t& ssrc)
	{
		return impl->give_me_data(tid, ssrc);
	}
	
	void client_stream_trigger_message::print_data()
	{
		impl->print_data();
	}

	client_stream_trigger_message::client_stream_trigger_message()
	{
		impl = std::make_shared<client_stream_trigger_message_impl>();
	}

	void client_stream_trigger_message::init()
	{
		impl->init();
	}

	void client_stream_trigger_message::clear()
	{
		impl->clear();
	}
}
