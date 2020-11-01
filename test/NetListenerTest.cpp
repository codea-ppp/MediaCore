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

void print_message(const connection& conn, void* message)
{
	dzlog_info("get message from %d@%s:%d", conn.sockfd, conn.ip.c_str(), conn.port);

	uint32_t* _message = (uint32_t*)message;

	uint32_t type	= _message[0];
	uint32_t length = _message[1];
	uint32_t tid	= _message[2];

	switch (type)
	{
	case MSGTYPE_KEEPALIVE: 
		PrintMessage((KeepAliveMessage*)message); 
		delete (KeepAliveMessage*)message;
		break;

	case MSGTYPE_PULLOTHERLOADBALANCE:
		PrintMessage((PullOtherLoadBalanceMessage*)message); 
		delete (PullOtherLoadBalanceMessage*)message;
		break;

	case MSGTYPE_RESPONDLOADBALANCEPULL:
		PrintMessage((RespondLoadBalancePullMessage*)message);
		ClearRespondLoadBalancePullMessage((RespondLoadBalancePullMessage*)message);
		delete (RespondLoadBalancePullMessage*)message;
		break;

	case MSGTYPE_PUSHLOADBANLANCE:
		PrintMessage((PushLoadBalanceMessage*)message);
		ClearRespondLoadBalancePullMessage((PushLoadBalanceMessage*)message);
		delete (PushLoadBalanceMessage*)message;
		break;

	case MSGTYPE_PULLMEDIAMENU:
		PrintMessage((PullMediaMenuMessage*)message);
		delete (PullMediaMenuMessage*)message;
		break;

	case MSGTYPE_RESPONDMEDIAMENU:
		PrintMessage((RespondMediaMenuPullMessage*)message);
		ClearRespondMediaMenuPullMessage((RespondMediaMenuPullMessage*)message);
		delete (RespondMediaMenuPullMessage*)message;
		break;

	case MSGTYPE_PUSHMEDIAMENU:
		PrintMessage((PushMediaMenuMessage*)message);
		ClearRespondMediaMenuPullMessage((PushMediaMenuMessage*)message);
		delete (PushMediaMenuMessage*)message;
		break;

	case MSGTYPE_CLIENTPULLMEDIASTREAM:
		PrintMessage((ClientPullMediaStreamMessage*)message);
		ClearClientPullMediaStreamMessage((ClientPullMediaStreamMessage*)message);
		delete (ClientPullMediaStreamMessage*)message;
		break;

	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM:
		PrintMessage((LoadBalancePullMediaStreamMessage*)message);
		ClearLoadBalancePullMediaStreamMessage((LoadBalancePullMediaStreamMessage*)message);
		delete (LoadBalancePullMediaStreamMessage*)message;
		break;

	case MSGTYPE_RESOURCERESPONDMEDIAPULL:
		PrintMessage((ResourceServerRespondMediaPullMessage*)message);
		delete (ResourceServerRespondMediaPullMessage*)message;
		break;

	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:
		PrintMessage((LoadBalanceRespondMediaPullMessage*)message);
		delete (LoadBalanceRespondMediaPullMessage*)message;
		break;

	case MSGTYPE_CLIENTSTREAMTRIGGER:
		PrintMessage((ClientStreamTriggerMessage*)message);
		delete (ClientStreamTriggerMessage*)message;
		break;

	case MSGTYPE_STREAMFRAME:
		PrintMessage((StreamMessage*)message);
		delete (StreamMessage*)message;
		break;

	case MSGTYPE_STOPSTREAM:
		PrintMessage((StopStreamMessage*)message);
		delete (StopStreamMessage*)message;
		break;

	case MSGTYPE_RESOURCESERVERREPORT:
		PrintMessage((ResourceServerReportMessage*)message);
		delete (ResourceServerReportMessage*)message;
		break;

	default: 
		dzlog_info("recv unknown mesage");
		break;
	}
}

void send_something()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family	= AF_INET;
	addr.sin_port	= htons(26120);
	if (inet_pton(AF_INET, "192.168.10.135", &addr.sin_addr) <= 0)
	{
		dzlog_error("failed to convern ip");
		return;
	}

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		dzlog_error("connect failed, errno: %d", errno);
		return;
	}

	{
		KeepAliveMessage message;
		InitKeepAliveMessage(&message);
		SetKeepAliveData(&message, 33, 45);
		SendKeepAliveMessage(sockfd, &message);
	}
	{
		PullOtherLoadBalanceMessage message;
		InitPullOtherLoadBalanceMessage(&message);
		SendPullOtherLoadBalanceMessage(sockfd, &message);
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
		SendRespondLoadBalancePullMessage(sockfd, &message);
		ClearRespondLoadBalancePullMessage(&message);
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
		SendPushLoadBalanceMessage(sockfd, &message);
		ClearPushLoadBalanceMessage(&message);
	}
	{
		PullMediaMenuMessage message;
		InitPullMediaMenuMessage(&message);
		SendPullMediaMenuMessage(sockfd, &message);
	}
	{
		std::vector<MediaMeta> medias;
		for (int i = 0; i < 20; ++i)
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
		SendRespondMediaMenuPullMessage(sockfd, &message);
		ClearRespondMediaMenuPullMessage(&message);
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
		SendPushMediaMenuMessage(sockfd, &message);
		ClearPushMediaMenuMessage(&message);
	}
	{
		ClientPullMediaStreamMessage message;
		InitClientPullMediaStreamMessage(&message);
		SetClientPullMediaStreamData(&message, "测试视频-sdl.mp4", 12026);
		SendClientPullMediaStreamMessage(sockfd, &message);
		ClearClientPullMediaStreamMessage(&message);
	}
	{
		LoadBalancePullMediaStreamMessage message;
		InitLoadBalancePullMediaStreamMessage(&message);
		SetLoadBalancePullMediaStreamData(&message, "测试视频ddd, 图形处理dsfdfa.avi", 602, 5562);
		SendLoadBalancePullMediaStreamMessage(sockfd, &message);
		ClearLoadBalancePullMediaStreamMessage(&message);
	}
	{
		ResourceServerRespondMediaPullMessage message;
		InitResourceServerRespondMediaPullMessage(&message);
		SetResourceServerRespondMediaPullData(&message, 9023);
		SendResourceServerRespondMediaPullMessage(sockfd, &message);
	}
	{
		LoadBalanceRespondMediaPullMessage message;
		InitLoadBalanceRespondMediaPullMessage(&message);
		SetLoadBalanceRespondMediaPullData(&message, 230, 3245);
		SendLoadBalanceRespondMediaPullMessage(sockfd, &message);
	}
	{
		ClientStreamTriggerMessage message;
		InitClientStreamTriggerMessage(&message);
		SetClientStreamTriggerData(&message, 32453);
		SendClientStreamTriggerMessage(sockfd, &message);
	}
	{
		uint32_t length[3] = { 1024, 720, 720 };
		uint8_t* value[3];
		value[0] = new uint8_t[1024];	memset(value[0], 33, 1024);
		value[1] = new uint8_t[720];	memset(value[1], 34, 720);
		value[2] = new uint8_t[720];	memset(value[2], 35, 720);

		StreamMessage message;
		InitStreamMessage(&message);
		SetStreamData(&message, value, length);
		SendStreamMessage(sockfd, &message);
		ClearStreamMessage(&message);

		delete value[0];
		delete value[1];
		delete value[2];
	}
	{
		StopStreamMessage message;
		InitStopStreamMessage(&message);
		SetStopStreamData(&message, 2391);
		SendStopStreamMessage(sockfd, &message);
	}
	{
		ResourceServerReportMessage message;
		InitResourceServerReportMessage(&message);
		SetResourceServerReportData(&message, 76);
		SendResourceServerReportMessage(sockfd, &message);
	}

	dzlog_info("close socket %d", sockfd);
	close(sockfd);
}

int main(int argc, char* argv[]) 
{
	dzlog_init("../config/zlog.config", "test");

	NetMessageListener::GetInstance()->SetCallback(&print_message);
	NetMessageListener::GetInstance()->listening(26120);

	while (true)
	{
		send_something();
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	return 0;
}
