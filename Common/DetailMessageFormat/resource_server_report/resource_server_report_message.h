#ifndef RESOURCE_SERVER_REPORT_H_
#define RESOURCE_SERVER_REPORT_H_

#include "MessageInerface.h"

namespace media_core_message
{
	class resource_server_report_impl;

	class resource_server_report : public message
	{
	public:
		int send_data_to(int sockfd);
		int full_data_remote(int sockfd, uint32_t tid);
		int full_data_direct(uint32_t tid, uint8_t error);
		int give_me_data(uint32_t& tid, uint8_t& error);

		void print_data();

		resource_server_report();

	private:
		void init();
		void clear();
		
	private:
		std::shared_ptr<resource_server_report_impl> impl;
	};
}

#endif 
