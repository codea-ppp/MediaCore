#include <thread>
#include <chrono>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message_headers.h"
#include "net_message_listener.h"

using namespace media_core_message;

void print_message(const connection conn, uint32_t message_type, std::shared_ptr<media_core_message::message> mess)
{
	dzlog_info("get message from %s:%d", conn.show_ip(), conn.show_port());
	mess->print_data();
}

#define PORT 27423

int main(int argc, char* argv[]) 
{
	dzlog_init("../zlog.config", "test");

	net_message_listener::get_instance()->set_callback(&print_message);
	net_message_listener::get_instance()->listening(PORT);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(20));
	}

	return 0;
}
