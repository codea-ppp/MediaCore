#include <zlog.h>
#include <thread>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "stream_pusher.h"

void end(uint32_t)
{

}

int main(int argc, char* argv[]) 
{
	std::string video("../resource/videos/test_video.mp4");

	dzlog_init("../zlog.config", "resource");

	uint16_t port;

	stream_pusher test;
	test.set_video(57, 444, video, &end);
	test.listening(port);

	dzlog_info("alloc %d", port);
	
	while (true)
		std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}
