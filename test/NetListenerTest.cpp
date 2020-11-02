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

#define PORT 27423

int main(int argc, char* argv[]) 
{
	dzlog_init("../config/zlog.config", "test");

	NetMessageListener::GetInstance()->SetCallback(&print_message);
	NetMessageListener::GetInstance()->listening(PORT);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(20));
	}

	return 0;
}
