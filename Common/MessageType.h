#ifndef MESSAGE_TYPE_H_
#define MESSAGE_TYPE_H_

#include <string>
#include <chrono>

#define ERR_NEED_2_CLOSE_SOCKET				-1
#define ERR_VIDEO_NAME_NOT_SET				-2
#define ERR_RECV_PORT_NOT_SET				-3
#define ERR_TID_NOT_SET						-4
#define ERR_SOCKET_FD_NEGATIVE				-5
#define ERR_VIDEO_NAME_TOO_LONG				-6
#define ERR_SSRC_NOT_SET					-7
#define ERR_SID_NOT_SET						-8
#define ERR_CLIENT_RECV_PORT_NO_SET			-9
#define ERR_VIDEO_NOT_SET					-10
#define ERR_VIDEO_NO_NAME					-11
#define ERR_SERVER_SEND_PORT_NO_SET			-12
#define ERR_ERRNO_NOT_SET					-13
#define ERR_LENGTH_NEGATIVE					-14
#define ERR_IP_PORT_SIZE_MISMATCH			-15
#define ERR_NO_IPS							-16
#define ERR_PTR_IS_NOT_NULLPTR				-17
#define ERR_LENGTH_EQU_0_BUT_PTR_NOT		-18
#define ERR_FRAME_ALL_0						-19
#define ERR_SET_NO_VIDEO_NAMES				-20

#define MSGTYPE_KEEPALIVE					0X01
#define MSGTYPE_PULLOTHERLOADBALANCE		0x02
#define MSGTYPE_RESPONDLOADBALANCEPULL		0x03
#define MSGTYPE_PUSHLOADBANLANCE			0x04

#define MSGTYPE_PULLMEDIAMENU				0x20
#define MSGTYPE_RESPONDMEDIAMENU			0x21
#define MSGTYPE_PUSHMEDIAMENU				0x22

#define MSGTYPE_CLIENTPULLMEDIASTREAM		0x30
#define MSGTYPE_LOADBALANCEPULLMEDIASTREAM	0x31
#define MSGTYPE_RESOURCERESPONDMEDIAPULL	0x32
#define	MSGTYPE_LOADBALANCERESPONDMEDIAPULL	0x33
#define	MSGTYPE_CLIENTSTREAMTRIGGER			0x34
#define MSGTYPE_STREAMFRAME					0x35
#define	MSGTYPE_STOPSTREAM					0x36
#define MSGTYPE_RESOURCESERVERREPORT		0x37

struct connection
{
	std::string ip;
	uint16_t port;
	uint16_t sockfd;
};

struct media_chain
{
	connection client;
	connection resource;
	uint16_t client_recv_port;
	uint16_t server_send_port;
	uint32_t ssrc;

	bool is_stream;
	std::chrono::time_point<std::chrono::steady_clock> last_flesh;
};

struct peer
{
	connection conn;
	int	type;
	uint16_t count;

	std::chrono::time_point<std::chrono::steady_clock> last_flesh;
};

#endif 
