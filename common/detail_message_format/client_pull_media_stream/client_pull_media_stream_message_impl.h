#ifndef CLIENT_PULL_MEDIA_STREAM_MESSAGE_IMPL_H_
#define CLIENT_PULL_MEDIA_STREAM_MESSAGE_IMPL_H_

#include <string>
#include <stdint.h>

namespace media_core_message
{
	class client_pull_media_stream_mesage_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, const std::string& media2play, uint16_t receive_port);
		int give_me_data(uint32_t& tid, std::string& video_name, uint16_t& receive_port);
		void print_data();
		void init();
		void clear();

		client_pull_media_stream_mesage_impl();
		~client_pull_media_stream_mesage_impl();

	private:
		std::string _video_name;
		uint16_t _receive_port;
		uint32_t _tid;
	};
}

#endif 
