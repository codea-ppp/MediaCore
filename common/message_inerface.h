#ifndef MESSAGE_INERFACE_H_
#define MESSAGE_INERFACE_H_

#include <memory>
#include <zlog.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace media_core_message
{
	class message
	{
	public:
		virtual int send_data_to(int sockfd) = 0;
		virtual int full_data_remote(int sockfd, uint32_t tid, uint32_t length) = 0;
		virtual int tell_me_type() = 0;
		virtual void print_data()  = 0;

	private:
		virtual void init()		= 0;
		virtual void clear()	= 0;
	};
}

#endif 
