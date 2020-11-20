#include <zlog.h>
#include <thread>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "stream_pusher_impl.h"

int main(int argc, char* argv[]) 
{
	std::string video("../resource/videos/test_video.mp4");

	dzlog_init("../zlog.config", "resource");

	stream_pusher_impl test;
	test.set_video(444, 666, video);
	test.listening(htons(5523));
	
	while (true)
		std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}
