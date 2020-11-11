#include "loadbalance_pull_media_stream_message.h"
#include "loadbalance_pull_media_stream_message_impl.h"

namespace media_core_message
{
	int loadbalance_pull_media_stream_message::send_data_to(int sockfd)
	{
		return impl->send_data_to(sockfd);
	}

	int loadbalance_pull_media_stream_message::full_data_remote(int sockfd, uint32_t tid, uint32_t)
	{
		return impl->full_data_remote(sockfd, tid);
	}

	int loadbalance_pull_media_stream_message::full_data_direct(uint32_t tid, uint16_t client_recv_port, uint32_t ssrc, const std::string& video_name)
	{
		return impl->full_data_direct(tid, client_recv_port, ssrc, video_name);
	}

	int loadbalance_pull_media_stream_message::give_me_data(uint32_t& tid, uint16_t& client_recv_port, uint32_t& ssrc, std::string& video_name)
	{
		return impl->give_me_data(tid, client_recv_port, ssrc, video_name);
	}

	void loadbalance_pull_media_stream_message::print_data()
	{
		impl->print_data();
	}

	void loadbalance_pull_media_stream_message::init()
	{
		impl->init();
	}

	void loadbalance_pull_media_stream_message::clear()
	{
		impl->clear();
	}

	loadbalance_pull_media_stream_message::loadbalance_pull_media_stream_message()
	{
		impl = std::make_shared<loadbalance_pull_media_stream_message_impl>();
	}
}
