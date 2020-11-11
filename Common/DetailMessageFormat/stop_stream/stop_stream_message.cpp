#include "stop_stream_message.h"
#include "stop_stream_message_impl.h"

namespace media_core_message
{
	int stop_stream_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int stop_stream_message::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int stop_stream_message::full_data_direct(uint32_t tid, uint32_t ssrc)
	{
		return impl->full_data_direct(tid, ssrc);
	}

	int stop_stream_message::give_me_data(uint32_t& tid, uint32_t& ssrc)
	{
		return impl->give_me_data(tid, ssrc);
	}

	void stop_stream_message::print_data()
	{
		impl->print_data();
	}

	stop_stream_message::stop_stream_message()
	{
		impl = std::make_shared<stop_stream_message_impl>();
	}

	stop_stream_message::~stop_stream_message()
	{
		impl->clear();
	}

	void stop_stream_message::init()
	{
		impl->init();
	}

	void stop_stream_message::clear()
	{
		impl->clear();
	}
}
