#include "stream_message.h"
#include "stream_message_impl.h"

namespace media_core_message
{
	int stream_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int stream_message::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int stream_message::full_data_direct(uint32_t tid, uint8_t* YUV[3], uint32_t YUV_length[3], uint32_t height)
	{
		return impl->full_data_direct(tid, YUV, YUV_length, height);
	}

	int stream_message::give_me_data(uint32_t& tid, uint8_t* must_be_nullptr[3], uint32_t YUV_length[3])
	{
		return impl->give_me_data(tid, must_be_nullptr, YUV_length);
	}

	int stream_message::tell_me_type()
	{
		return MSGTYPE_STREAMFRAME;
	}

	void stream_message::print_data()
	{
		impl->print_data();
	}

	stream_message::stream_message()
	{
		impl = std::make_shared<stream_message_impl>();
	}

	stream_message::~stream_message()
	{
		clear();
	}

	void stream_message::init()
	{
		impl->clear();
	}

	void stream_message::clear()
	{
		impl->clear();
	}
}
