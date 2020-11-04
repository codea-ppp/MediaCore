#ifndef MESSAGE_TYPE_H_
#define MESSAGE_TYPE_H_

#include <string>

#define NEED_2_CLOSE_SOCKET_ERROR			-2

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

template <typename T>
void doouble_memory(T** head, long size)
{
	T* temp = new T[size * 2];
	memcpy(temp, *head, size * sizeof(T));
	delete[] head;
	*head = temp;
}

#endif 
