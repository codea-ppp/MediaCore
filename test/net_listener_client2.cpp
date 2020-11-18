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

void send_keepalive(connection conn)
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(555, 234, 2);
	conn.send_message(mess);
}

void send_pull_other_loadbalance(connection conn)
{
	std::shared_ptr<pull_other_loadbalance_message> mess = std::make_shared<pull_other_loadbalance_message>();
	mess->full_data_direct(332);
	conn.send_message(mess);
}

void send_respond_loadbalance_pulling(connection conn)
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

	std::shared_ptr<respond_loadbalance_pull_message> mess = std::make_shared<respond_loadbalance_pull_message>();
	mess->full_data_direct(1323, sids, ip, port);
	conn.send_message(mess);
}

void send_push_loadbalance(connection conn)
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

	std::shared_ptr<push_loadbalance_pull_message> mess = std::make_shared<push_loadbalance_pull_message>();
	mess->full_data_direct(1234, sids, ip, port);
	conn.send_message(mess);
}

void send_pull_media_menu(connection conn)
{
	std::shared_ptr<pull_media_menu_message> mess = std::make_shared<pull_media_menu_message>();
	mess->full_data_direct(134234);
	conn.send_message(mess);
}

void send_respond_media_pull(connection conn) 
{
	std::vector<std::string> medias;
	for (int i = 0; i < 10; ++i)
	{
		std::string temp("测试实验ddjfalsjf2313&*)){{{{<JIweier}}}} dd;d");
		temp.append(std::to_string(i));
		medias.push_back(temp);
	}

	std::shared_ptr<respond_media_menu_pull_message> mess = std::make_shared<respond_media_menu_pull_message>();
	mess->full_data_direct(233423, medias);
	conn.send_message(mess);
}

void send_push_media(connection conn)
{
	std::vector<std::string> medias;
	for (int i = 0; i < 10; ++i)
	{
		std::string temp("242342测试实验ddjfalsjf2313&*)){{{{<JIweier}}}} dd;d");
		temp.append(std::to_string(i));
		medias.push_back(temp);
	}

	std::shared_ptr<push_media_pull_message> mess = std::make_shared<push_media_pull_message>();
	mess->full_data_direct(2343, medias);
	conn.send_message(mess);
}

void send_client_pull_media_stream(connection conn)
{
	std::shared_ptr<client_pull_media_stream_message> mess = std::make_shared<client_pull_media_stream_message>();
	mess->full_data_direct(334, "测试视频但是看了房价是的罚款集散地 软3.mp4", 3423);
	conn.send_message(mess);
}

void send_loadbalance_pull_media_stream(connection conn)
{
	std::shared_ptr<loadbalance_pull_media_stream_message> mess = std::make_shared<loadbalance_pull_media_stream_message>();
	mess->full_data_direct(789, 4234, 898, "测试大力开发撒娇的分量卡kj;w");
	conn.send_message(mess);
}

void send_resource_respond_media_pull(connection conn)
{
	std::shared_ptr<resource_server_respond_media_menu_pull_message> mess = std::make_shared<resource_server_respond_media_menu_pull_message>();
	mess->full_data_direct(32432, 1024, 720, 12342);
	conn.send_message(mess);
}

void send_loadbalance_respond_media_pull(connection conn)
{
	struct in_addr addr;
	inet_aton("192.168.55.21", &addr);

	std::shared_ptr<loadbalance_respond_media_menu_pull_message> mess = std::make_shared<loadbalance_respond_media_menu_pull_message>();
	mess->full_data_direct(234, 9999, 1024, 720, addr.s_addr, 48);
	conn.send_message(mess);
}

void send_client_trigger(connection conn)
{
	std::shared_ptr<client_stream_trigger_message> mess = std::make_shared<client_stream_trigger_message>();
	mess->full_data_direct(234, 5555);
	conn.send_message(mess);
}

void send_stream(connection conn)
{
	static uint8_t buffer[1024][3];
	uint32_t length[3] = { 1024, 1024, 1024 };

	uint8_t* value[3];

	value[0] = buffer[0];	memset(value[0], 33, 1024);
	value[1] = buffer[1];	memset(value[1], 34, 1024);
	value[2] = buffer[2];	memset(value[2], 35, 1024);

	std::shared_ptr<stream_message> mess = std::make_shared<stream_message>();
	mess->full_data_direct(234234, value, length);
	conn.send_message(mess);
}

void send_stop_stream(connection conn)
{
	std::shared_ptr<stop_stream_message> mess = std::make_shared<stop_stream_message>();
	mess->full_data_direct(23, 23334);
	conn.send_message(mess);
}

void send_resource_report(connection conn)
{
	std::shared_ptr<resource_server_report_message> mess = std::make_shared<resource_server_report_message>();
	mess->full_data_direct(9955, 234);
	conn.send_message(mess);
}

void send_something(connection conn)
{
	while (true)
	{
//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_keepalive(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_pull_other_loadbalance(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_respond_loadbalance_pulling(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_push_loadbalance(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_pull_media_menu(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_respond_media_pull(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_push_media(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_client_pull_media_stream(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_loadbalance_pull_media_stream(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_resource_respond_media_pull(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_loadbalance_respond_media_pull(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_client_trigger(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_stream(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_stop_stream(conn);

//			std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
		send_resource_report(conn);

		std::this_thread::sleep_for(std::chrono::milliseconds(80)); 
	}
}

int main(int argc, char* argv[]) 
{
	dzlog_init("../zlog.config", "test");

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family	= AF_INET;
	addr.sin_port	= htons(PORT);

	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
	{
		dzlog_error("failed to convern ip");
		return 0 ;
	}
	
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		dzlog_error("connect failed, errno: %d", errno);
		return 0;
	}

	connection conn(sockfd, addr.sin_addr.s_addr, addr.sin_port);

	dzlog_info("alloc a socket %d", sockfd);

	threadpool_instance::get_instance()->schedule(std::bind(&send_something, conn));
	threadpool_instance::get_instance()->schedule(std::bind(&send_something, conn));

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(20));
	}

	close(sockfd);
	return 0;
}
