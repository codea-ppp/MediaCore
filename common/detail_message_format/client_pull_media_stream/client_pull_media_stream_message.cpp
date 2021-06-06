#include "client_pull_media_stream_message.h"

#include "client_pull_media_stream_message_impl.h"

namespace media_core_message
{
	int client_pull_media_stream_message::send_data_to(int sockfd) 
	{ 
		return impl->send_data_to(sockfd); 
	}

	int client_pull_media_stream_message::full_data_remote(int sockfd, uint32_t tid, uint32_t) 
	{ 
		return impl->full_data_remote(sockfd, tid); 
	}

	int client_pull_media_stream_message::full_data_direct(uint32_t tid, const std::string& media2play, uint16_t receive_port) 
	{ 
		return impl->full_data_direct(tid, media2play, receive_port); 
	}

	int client_pull_media_stream_message::give_me_data(uint32_t& tid, std::string& video_name, uint16_t& receive_port)
	{
		return impl->give_me_data(tid, video_name, receive_port);
	}

	int client_pull_media_stream_message::tell_me_type()
	{
		return MSGTYPE_CLIENTPULLMEDIASTREAM;
	}

	void client_pull_media_stream_message::print_data() 
	{ 
		impl->print_data(); 
	}

	client_pull_media_stream_message::client_pull_media_stream_message()
	{
		impl = std::make_shared<client_pull_media_stream_message_impl>();
	}

	void client_pull_media_stream_message::clear() 
	{ 
		impl->clear(); 
	}

	void client_pull_media_stream_message::init() 
	{ 
		impl->init(); 
	}
}
