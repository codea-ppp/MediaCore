#include <thread>
#include <chrono>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message_headers.h"
#include "threadpool_instance.h"
#include "net_message_listener.h"

using namespace media_core_message;

#define PORT 27423

void send_keepalive(int sock)
{
	keepalive_message mess;
	mess.full_data_direct(555, 234, 3366, 2);
	mess.send_data_to(sock);
}

void send_pull_other_loadbalance(int sock)
{
	pull_other_loadbalance_message mess;
	mess.full_data_direct(332);
	mess.send_data_to(sock);
}

void send_respond_loadbalance_pulling(int sock)
{
	std::vector<uint32_t> sids;
	std::vector<uint32_t> ip;
	std::vector<uint16_t> port;

	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.1")); port.push_back(htons(25564));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.2")); port.push_back(htons(25565));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.3")); port.push_back(htons(25566));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.4")); port.push_back(htons(25567));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.5")); port.push_back(htons(25568));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.6")); port.push_back(htons(25560));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.7")); port.push_back(htons(25544));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.8")); port.push_back(htons(25584));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.9")); port.push_back(htons(28564));

	respond_loadbalance_pull_message mess;
	mess.full_data_direct(1323, sids, ip, port);
	mess.send_data_to(sock);
}

void send_push_loadbalance(int sock)
{
	std::vector<uint32_t> sids;
	std::vector<uint32_t> ip;
	std::vector<uint16_t> port;

	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.1")); port.push_back(htons(25564));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.2")); port.push_back(htons(25565));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.3")); port.push_back(htons(25566));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.4")); port.push_back(htons(25567));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.5")); port.push_back(htons(25568));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.6")); port.push_back(htons(25560));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.7")); port.push_back(htons(25544));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.8")); port.push_back(htons(25584));
	sids.push_back(5545); ip.push_back(inet_addr("127.0.0.9")); port.push_back(htons(28564));

	push_loadbalance_pull_message mess;
	mess.full_data_direct(1234, sids, ip, port);
	mess.send_data_to(sock);
}

void send_pull_media_menu(int sock)
{
	pull_media_menu_message mess;
	mess.full_data_direct(134234);
	mess.send_data_to(sock);
}

void send_respond_media_pull(int sock) 
{
	std::vector<std::string> medias;
	for (int i = 0; i < 10; ++i)
	{
		std::string temp("测试实验ddjfalsjf2313&*)){{{{<JIweier}}}} dd;d");
		temp.append(std::to_string(i));
		medias.push_back(temp);
	}

	respond_media_menu_pull_message mess;
	mess.full_data_direct(233423, medias);
	mess.send_data_to(sock);
}

void send_push_media(int sock)
{
	std::vector<std::string> medias;
	for (int i = 0; i < 10; ++i)
	{
		std::string temp("242342测试实验ddjfalsjf2313&*)){{{{<JIweier}}}} dd;d");
		temp.append(std::to_string(i));
		medias.push_back(temp);
	}

	push_media_pull_message mess;
	mess.full_data_direct(2343, medias);
	mess.send_data_to(sock);
}

void send_client_pull_media_stream(int sock)
{
	client_pull_media_stream_message mess;
	mess.full_data_direct(334, "测试视频但是看了房价是的罚款集散地 软3.mp4", 3423);
	mess.send_data_to(sock);
}

void send_loadbalance_pull_media_stream(int sock)
{
	loadbalance_pull_media_stream_message mess;
	mess.full_data_direct(789, 4234, 898, "测试大力开发撒娇的分量卡kj;w");
	mess.send_data_to(sock);
}

void send_resource_respond_media_pull(int sock)
{
	resource_server_respond_media_menu_pull_message mess;
	mess.full_data_direct(32432, 1024, 720, 12342);
	mess.send_data_to(sock);
}

void send_loadbalance_respond_media_pull(int sock)
{
	struct in_addr addr;
	inet_aton("192.168.55.21", &addr);

	loadbalance_respond_media_menu_pull_message mess;
	mess.full_data_direct(234, 9999, 1024, 720, addr.s_addr, 48);
	mess.send_data_to(sock);
}

void send_client_trigger(int sock)
{
	client_stream_trigger_message mess;
	mess.full_data_direct(234, 5555);
	mess.send_data_to(sock);
}

void send_stop_stream(int sock)
{
	stop_stream_message mess;
	mess.full_data_direct(23, 23334);
	mess.send_data_to(sock);
}

void send_resource_report(int sock)
{
	resource_server_report_message mess;
	mess.full_data_direct(9955, 234);
	mess.send_data_to(sock);
}

void send_something()
{
	while (true)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		struct sockaddr_in addr;
		addr.sin_family	= AF_INET;
		addr.sin_port	= htons(PORT);

		if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
		{
			dzlog_error("failed to convern ip");
			return;
		}
		
		if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
		{
			dzlog_error("connect failed, errno: %d", errno);
			return;
		}

		dzlog_info("alloc a socket %d", sockfd);

		while (true)
		{
//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_keepalive(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_pull_other_loadbalance(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_respond_loadbalance_pulling(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_push_loadbalance(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_pull_media_menu(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_respond_media_pull(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_push_media(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_client_pull_media_stream(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_loadbalance_pull_media_stream(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_resource_respond_media_pull(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_loadbalance_respond_media_pull(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_client_trigger(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
//			send_stream(sockfd);	// stream 用 player_client 测试

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_stop_stream(sockfd);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
			send_resource_report(sockfd);

			std::this_thread::sleep_for(std::chrono::milliseconds(80)); 
		}

		dzlog_info("close socket %d from break", sockfd);
		close(sockfd);
	}

}

int main(int argc, char* argv[]) 
{
	dzlog_init("../zlog.config", "test");

	threadpool_instance::get_instance()->schedule(&send_something);
	threadpool_instance::get_instance()->schedule(&send_something);
	threadpool_instance::get_instance()->schedule(&send_something);
	threadpool_instance::get_instance()->schedule(&send_something);

	while (true)
	{
//		send_something();
		std::this_thread::sleep_for(std::chrono::seconds(20));
	}

	return 0;
}
