#include <thread>
#include <chrono>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "NetMessageListener.h"

using namespace MediaCoreMessageFormat;

#define PORT 27423

void send_something()
{
	while (true)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		struct sockaddr_in addr;
		addr.sin_family	= AF_INET;
		addr.sin_port	= htons(PORT);

		if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
		{
			dzlog_error("connect failed, errno: %d", errno);
			return;
		}

		if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
		{
			dzlog_error("failed to convern ip");
			return;
		}

		while (true)
		{
			{
				KeepAliveMessage message;
				InitKeepAliveMessage(&message);
				SetKeepAliveData(&message, 33, 45);
				if (!SendKeepAliveMessage(sockfd, &message))
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				PullOtherLoadBalanceMessage message;
				InitPullOtherLoadBalanceMessage(&message);
				if (!SendPullOtherLoadBalanceMessage(sockfd, &message))
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				std::vector<uint32_t> ip;
				std::vector<uint16_t> port;
				ip.push_back(inet_addr("127.0.0.1")); port.push_back(htons(25564));
				ip.push_back(inet_addr("127.0.0.2")); port.push_back(htons(25565));
				ip.push_back(inet_addr("127.0.0.3")); port.push_back(htons(25566));
				ip.push_back(inet_addr("127.0.0.4")); port.push_back(htons(25567));
				ip.push_back(inet_addr("127.0.0.5")); port.push_back(htons(25568));
				ip.push_back(inet_addr("127.0.0.6")); port.push_back(htons(25560));
				ip.push_back(inet_addr("127.0.0.7")); port.push_back(htons(25544));
				ip.push_back(inet_addr("127.0.0.8")); port.push_back(htons(25584));
				ip.push_back(inet_addr("127.0.0.9")); port.push_back(htons(28564));

				RespondLoadBalancePullMessage message;
				InitRespondLoadBalancePullMessage(&message);
				SetOtherLoadBalanceData(&message, ip, port);
				if (!SendRespondLoadBalancePullMessage(sockfd, &message))
				{
					ClearRespondLoadBalancePullMessage(&message);
					break;
				}

				ClearRespondLoadBalancePullMessage(&message);
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				std::vector<uint32_t> ip;
				std::vector<uint16_t> port;
				ip.push_back(inet_addr("127.1.1.1")); port.push_back(htons(25564));
				ip.push_back(inet_addr("127.1.1.2")); port.push_back(htons(25565));
				ip.push_back(inet_addr("127.1.1.3")); port.push_back(htons(25566));
				ip.push_back(inet_addr("127.1.1.4")); port.push_back(htons(25567));
				ip.push_back(inet_addr("127.1.1.5")); port.push_back(htons(25568));
				ip.push_back(inet_addr("127.1.1.6")); port.push_back(htons(25561));
				ip.push_back(inet_addr("127.1.1.7")); port.push_back(htons(25544));
				ip.push_back(inet_addr("127.1.1.8")); port.push_back(htons(25584));
				ip.push_back(inet_addr("127.1.1.9")); port.push_back(htons(28564));

				PushLoadBalanceMessage message;
				InitPushLoadBalanceMessage(&message);
				SetOtherLoadBalanceData(&message, ip, port);
				if (!SendPushLoadBalanceMessage(sockfd, &message))
				{
					ClearPushLoadBalanceMessage(&message);
					break;
				}

				ClearPushLoadBalanceMessage(&message);
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				PullMediaMenuMessage message;
				InitPullMediaMenuMessage(&message);
				if (!SendPullMediaMenuMessage(sockfd, &message))
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				std::vector<MediaMeta> medias;
				for (int i = 0; i < 30; ++i)
				{
					MediaMeta temp;
					temp.VideoNameLength = 32;
					temp.VideoName = new uint8_t[32];
					memset(temp.VideoName, 0, 32);
					snprintf((char*)temp.VideoName, 32, "%s", std::to_string(i).c_str());

					medias.push_back(temp);
				}

				RespondMediaMenuPullMessage message;
				InitRespondMediaMenuPullMessage(&message);
				SetMediaMetaData(&message, medias);
				if (!SendRespondMediaMenuPullMessage(sockfd, &message))
				{
					ClearRespondMediaMenuPullMessage(&message);
					break;
				}

				ClearRespondMediaMenuPullMessage(&message);
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				std::vector<MediaMeta> medias;
				for (int i = 50; i > 0; --i)
				{
					MediaMeta temp;
					temp.VideoNameLength = 32;
					temp.VideoName = new uint8_t[32];
					memset(temp.VideoName, 0, 32);
					snprintf((char*)temp.VideoName, 32, "%s", std::to_string(i).c_str());

					medias.push_back(temp);
				}

				PushMediaMenuMessage message;
				InitPushLoadBalanceMessage(&message);
				SetMediaMetaData(&message, medias);
				if (!SendPushMediaMenuMessage(sockfd, &message))
				{
					ClearPushMediaMenuMessage(&message);
					break;
				}

				ClearPushMediaMenuMessage(&message);
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				ClientPullMediaStreamMessage message;
				InitClientPullMediaStreamMessage(&message);
				SetClientPullMediaStreamData(&message, "测试视频-sdl.mp4", 13026);
				if (!SendClientPullMediaStreamMessage(sockfd, &message))
				{
					ClearClientPullMediaStreamMessage(&message);
					break;
				}

				ClearClientPullMediaStreamMessage(&message);
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				LoadBalancePullMediaStreamMessage message;
				InitLoadBalancePullMediaStreamMessage(&message);
				SetLoadBalancePullMediaStreamData(&message, "测试视频ddd, 图形处理dsfdfa.avi", 602, 5562);
				if (!SendLoadBalancePullMediaStreamMessage(sockfd, &message))
				{
					ClearLoadBalancePullMediaStreamMessage(&message);
					break;
				}
				 
				ClearLoadBalancePullMediaStreamMessage(&message);
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				ResourceServerRespondMediaPullMessage message;
				InitResourceServerRespondMediaPullMessage(&message);
				SetResourceServerRespondMediaPullData(&message, 9023);
				if (!SendResourceServerRespondMediaPullMessage(sockfd, &message))
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				LoadBalanceRespondMediaPullMessage message;
				InitLoadBalanceRespondMediaPullMessage(&message);
				SetLoadBalanceRespondMediaPullData(&message, 230, 3245);
				if (!SendLoadBalanceRespondMediaPullMessage(sockfd, &message))
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				ClientStreamTriggerMessage message;
				InitClientStreamTriggerMessage(&message);
				SetClientStreamTriggerData(&message, 32453);
				if (!SendClientStreamTriggerMessage(sockfd, &message))
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				uint32_t length[3] = { 1024, 730, 730 };
				uint8_t* value[3];
				value[0] = new uint8_t[1024];	memset(value[0], 33, 1024);
				value[1] = new uint8_t[730];	memset(value[1], 34, 730);
				value[2] = new uint8_t[730];	memset(value[2], 35, 730);

				StreamMessage message;
				InitStreamMessage(&message);
				SetStreamData(&message, value, length);
				if (!SendStreamMessage(sockfd, &message))
				{
					ClearStreamMessage(&message);
					delete value[0];
					delete value[1];
					delete value[2];
					break;
				}

				ClearStreamMessage(&message);

				delete value[0];
				delete value[1];
				delete value[2];
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				StopStreamMessage message;
				InitStopStreamMessage(&message);
				SetStopStreamData(&message, 2391);
				if (!SendStopStreamMessage(sockfd, &message))
					break;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
			{
				ResourceServerReportMessage message;
				InitResourceServerReportMessage(&message);
				SetResourceServerReportData(&message, 76);
				if (!SendResourceServerReportMessage(sockfd, &message))
					break;

				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		dzlog_info("close socket %d from break", sockfd);
		close(sockfd);
	}

}

int main(int argc, char* argv[]) 
{
	dzlog_init("../config/zlog.config", "test");

	ThreadpoolInstance::GetInstance()->schedule(&send_something);
	ThreadpoolInstance::GetInstance()->schedule(&send_something);
	ThreadpoolInstance::GetInstance()->schedule(&send_something);
	ThreadpoolInstance::GetInstance()->schedule(&send_something);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(20));
	}

	return 0;
}
