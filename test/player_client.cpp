#include <zlog.h>
#include <thread>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "stream_render.h"

int main(int argc, char* argv[]) 
{
	dzlog_init("../zlog.config", "client");

	struct in_addr a;
	inet_aton("127.0.0.1", &a);

	std::string video("../resource/videos/test_video.mp4");
	stream_render test(video, 1280, 720);
	test.triggering(444, 444, a.s_addr, 10216);

	while (true)
		std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}
