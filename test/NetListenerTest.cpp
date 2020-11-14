#include <thread>
#include <chrono>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message_headers.h"
#include "net_message_listener_impl.h"

using namespace media_core_message;

void print_message(const connection conn, uint32_t message_type, std::shared_ptr<media_core_message::message> mess)
{
	dzlog_info("get message from %s:%d", conn.show_ip(), conn.show_port());
	mess->print_data();

	/*
	switch (mess->tell_me_type())
	{
	case MSGTYPE_KEEPALIVE: 
		PrintMessage((KeepAliveMessage*)message); 
		break;

	case MSGTYPE_PULLOTHERLOADBALANCE:
		PrintMessage((PullOtherLoadBalanceMessage*)message); 
		break;

	case MSGTYPE_RESPONDLOADBALANCEPULL:
		PrintMessage((RespondLoadBalancePullMessage*)message);
		break;

	case MSGTYPE_PUSHLOADBANLANCE:
		PrintMessage((PushLoadBalanceMessage*)message);
		break;

	case MSGTYPE_PULLMEDIAMENU:
		PrintMessage((PullMediaMenuMessage*)message);
		break;

	case MSGTYPE_RESPONDMEDIAMENU:
		PrintMessage((RespondMediaMenuPullMessage*)message);
		break;

	case MSGTYPE_PUSHMEDIAMENU:
		PrintMessage((PushMediaMenuMessage*)message);
		break;

	case MSGTYPE_CLIENTPULLMEDIASTREAM:
		PrintMessage((client_pull_media_stream_message*)message);
		break;

	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM:
		PrintMessage((LoadBalancePullMediaStreamMessage*)message);
		break;

	case MSGTYPE_RESOURCERESPONDMEDIAPULL:
		PrintMessage((ResourceServerRespondMediaPullMessage*)message);
		break;

	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:
		PrintMessage((LoadBalanceRespondMediaPullMessage*)message);
		break;

	case MSGTYPE_CLIENTSTREAMTRIGGER:
		PrintMessage((ClientStreamTriggerMessage*)message);
		break;

	case MSGTYPE_STREAMFRAME:
		PrintMessage((StreamMessage*)message);
		break;

	case MSGTYPE_STOPSTREAM:
		PrintMessage((StopStreamMessage*)message);
		break;

	case MSGTYPE_RESOURCESERVERREPORT:
		PrintMessage((ResourceServerReportMessage*)message);
		break;

	default: 
		dzlog_info("recv unknown mesage");
		break;
	}
	*/
}

#define PORT 27423

int main(int argc, char* argv[]) 
{
	dzlog_init("../zlog.config", "test");

	net_message_listener_impl::get_instance()->set_callback(&print_message);
	net_message_listener_impl::get_instance()->listening(PORT);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(20));
	}

	return 0;
}
