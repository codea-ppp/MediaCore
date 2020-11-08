#include "client_pull_media_stream_message.h"
#include "client_pull_media_stream_message_impl.h"

namespace media_core_message
{
	int client_pull_media_stream_mesage::send_data_to(int sockfd) 
	{ 
		return impl->send_data_to(sockfd); 
	}

	int client_pull_media_stream_mesage::full_data_remote(int sockfd, uint32_t tid) 
	{ 
		return impl->full_data_remote(sockfd, tid); 
	}

	int client_pull_media_stream_mesage::full_data_direct(uint32_t tid, const std::string& media2play, uint16_t receive_port) 
	{ 
		return impl->full_data_direct(tid, media2play, receive_port); 
	}

	int client_pull_media_stream_mesage::give_me_data(uint32_t& tid, std::string& video_name, uint16_t& receive_port)
	{
		return impl->give_me_data(tid, video_name, receive_port);
	}

	void client_pull_media_stream_mesage::print_data() 
	{ 
		impl->print_data(); 
	}

	client_pull_media_stream_mesage::client_pull_media_stream_mesage()
	{
		impl = std::make_shared<client_pull_media_stream_mesage_impl>();
	}

	void client_pull_media_stream_mesage::clear() 
	{ 
		impl->clear(); 
	}

	void client_pull_media_stream_mesage::init() 
	{ 
		impl->init(); 
	}
}
