#ifndef LOADBALANCE_PULL_MEDIA_STREAM_MESSAGE_IMPL_H_
#define LOADBALANCE_PULL_MEDIA_STREAM_MESSAGE_IMPL_H_

#include <string>
#include <stdint.h>

namespace media_core_message
{
	class loadbalance_pull_media_stream_message_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint16_t client_recv_port, uint32_t ssrc, const std::string& video_name);
		int give_me_data(uint32_t& tid, uint16_t& client_recv_port, uint32_t& ssrc, std::string& video_name);

		void print_data();
		void init();
		void clear();

		loadbalance_pull_media_stream_message_impl();
		~loadbalance_pull_media_stream_message_impl();

	private:
		std::string _video_name;
		uint16_t _client_receive_port;
		uint32_t _ssrc;
		uint32_t _tid;
	};
}

#endif 
