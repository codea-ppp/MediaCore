#include <zlog.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "message_type.h"
#include "message_headers.h"
#include "connection_impl.h"

int connection_impl::send_message(std::shared_ptr<media_core_message::message> mess)
{
	if (mess == nullptr)
	{
		dzlog_error("message == nullptr");
		return ERR_MESSAGE_EQU_NULLPTR;
	}

	if (!mess->send_data_to(_sockfd))	
	{
		dzlog_error("send message failed: %d", errno);
		return ERR_MESSAGE_SEND_FAILED;
	}

	return 0;
}

int connection_impl::give_message(std::shared_ptr<media_core_message::message>& mess)
{
	using namespace media_core_message;
	
	uint8_t buffer[12] = { 0 };

	int temp = recv(_sockfd, buffer, 12, MSG_WAITALL);
	if (temp != 12)
	{
		dzlog_error("failed to recv message head socket: %d, get %d, errno: %d", _sockfd, temp, errno);
		return ERR_CANNOT_RECV_MESSAGE_HEAD;
	}

	uint32_t type	= ((uint32_t*)buffer)[0];
	uint32_t length	= ((uint32_t*)buffer)[1];
	uint32_t tid	= ((uint32_t*)buffer)[2];

	dzlog_info("get message head %d, %d, %d", type, length, tid);

	switch (type)
	{
	case MSGTYPE_KEEPALIVE:
		mess = std::make_shared<keepalive_message>(); break;
	case MSGTYPE_PULLOTHERLOADBALANCE:
		mess = std::make_shared<pull_other_loadbalance_message>(); break;
	case MSGTYPE_RESPONDLOADBALANCEPULL:
		mess = std::make_shared<respond_loadbalance_pull_message>(); break;
	case MSGTYPE_PUSHLOADBANLANCE:
		mess = std::make_shared<push_loadbalance_pull_message>(); break;
	case MSGTYPE_PULLMEDIAMENU:
		mess = std::make_shared<pull_media_menu_message>(); break;
	case MSGTYPE_RESPONDMEDIAMENU:
		mess = std::make_shared<respond_media_pull_message>(); break;
	case MSGTYPE_PUSHMEDIAMENU:
		mess = std::make_shared<push_media_pull_message>(); break;
	case MSGTYPE_CLIENTPULLMEDIASTREAM:
		mess = std::make_shared<client_pull_media_stream_message>(); break;
	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM:
		mess = std::make_shared<loadbalance_pull_media_stream_message>(); break;
	case MSGTYPE_RESOURCERESPONDMEDIAPULL:
		mess = std::make_shared<resource_server_respond_media_pull_message>(); break;
	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:
		mess = std::make_shared<loadbalance_respond_media_pull_message>(); break;
	case MSGTYPE_CLIENTSTREAMTRIGGER:
		mess = std::make_shared<client_stream_trigger_message>(); break;
	case MSGTYPE_STREAMFRAME:
		mess = std::make_shared<stream_message>(); break;
	case MSGTYPE_STOPSTREAM:
		mess = std::make_shared<stop_stream_message>(); break;
	case MSGTYPE_RESOURCESERVERREPORT:
		mess = std::make_shared<resource_server_report_message>(); break;

	default: return ERR_UNKNOWN_MESSAGE_TYPE;
	}

	if (mess->full_data_remote(_sockfd, tid, length))
	{
		dzlog_error("full message failed: %d", errno);
		return ERR_MESSAGE_BODY_RECV_FAILED;
	}

	return 0;
}

const int connection_impl::show_sockfd()
{
	return _sockfd;
}

const char* connection_impl::show_ip()
{
	static char ip_buffer[16];
	memset(ip_buffer, 0, 16);

	snprintf(ip_buffer, 16, "%u.%u.%u.%u", (_ip & 0x000000ff), (_ip & 0x0000ff00), (_ip & 0x00ff0000), (_ip & 0xff000000) >> 24);
	return ip_buffer;
}

const uint16_t connection_impl::show_port()
{
	return _port;
}

connection_impl::connection_impl(const int sockfd, const uint32_t ip, const uint16_t port)
	: _sockfd(sockfd) , _ip(ip) , _port(port) 
{
}

connection_impl::~connection_impl()
{
	close(_sockfd);
}
