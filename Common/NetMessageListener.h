#ifndef NET_MESSAGE_LISTENER_H_
#define NET_MESSAGE_LISTENER_H_

#include <mutex>
#include "MessageType.h"
#include "MessageFormat.h"
#include "ThreadpoolInstance.h"

class NetMessageListener
{
public:
	static NetMessageListener* const GetInstance();

	int listening(uint16_t port);
	void SetCallback(void (*Message2Go)(const connection& conn, void* MessageHeadPtr));

private:
	void _listening();
	void _rolling(const connection conn);
	void* construct_message(const connection& conn, uint8_t* buffer);

private:
	uint16_t _port;
	int	_status;

	void (*_Message2Go)(const connection& conn, void* ptr);

	NetMessageListener();
	~NetMessageListener();

	NetMessageListener(const NetMessageListener&)	= delete;
	NetMessageListener(const NetMessageListener&&)	= delete;
	const NetMessageListener& operator=(const NetMessageListener&)	= delete;
	const NetMessageListener& operator=(const NetMessageListener&&)	= delete;
};

#endif 
