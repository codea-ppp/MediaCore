#ifndef RESOURCE_SERVER_REPORT_IMPL_H_
#define RESOURCE_SERVER_REPORT_IMPL_H_

#include "message_inerface.h"

namespace media_core_message
{
	class resource_server_report_impl
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint8_t error);
		int give_me_data(uint32_t& tid, uint8_t& error);

		void print_data();
		void init();
		void clear();

		resource_server_report_impl();
		~resource_server_report_impl();

	private:
		uint32_t _tid;
		uint8_t _error;
	};
}

#endif 
