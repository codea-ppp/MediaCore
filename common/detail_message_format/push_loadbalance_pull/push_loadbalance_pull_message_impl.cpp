#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "message_type.h"
#include "push_loadbalance_pull_message_impl.h"

namespace media_core_message
{
	int push_loadbalance_pull_message_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0)
		{
			dzlog_error("socket < 0");
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		if (_tid == 0)
		{
			dzlog_error("tid not set");
			return ERR_TID_NOT_SET;
		}

		if (_ips.size() != _ports.size())
		{
			dzlog_error("ip size != ports size");
			return ERR_IP_PORT_SIZE_MISMATCH;
		}

		if (!_ips.size())
		{
			dzlog_error("ip size == 0");
			return ERR_NO_IPS;
		}

		unsigned int size = _ips.size();
		unsigned int all = 6 * size + 12;

		uint8_t* buffer = new uint8_t[all];
		memset(buffer, 0, all);

		uint32_t* head_buffer = (uint32_t*)buffer;
		head_buffer[0] = MSGTYPE_PUSHLOADBANLANCE;
		head_buffer[1] = 6 * size;
		head_buffer[2] = _tid;

		uint32_t* ip_buffer = (uint32_t*)(buffer + 12);
		for (unsigned int i = 0; i < size; ++i)
		{
			ip_buffer[i] = _ips[i];
		}

		uint16_t* port_buffer = (uint16_t*)(buffer + 12 + size * 4);
		for (unsigned int i = 0; i < size; ++i)
		{
			port_buffer[i] = _ports[i];
		}

		int once = 0;
		for (unsigned int i = 0; i < all; i += once)
		{
			once = send(sockfd, buffer + i, all - i, 0);
			if (once == -1)
			{

				clear();

				dzlog_error("send error: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int push_loadbalance_pull_message_impl::full_data_remote(int sockfd, uint32_t tid, uint32_t length)
	{
		if (sockfd < 0)
		{
			dzlog_error("socket < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		if (length < 0 || length % 6 != 0)
		{
			dzlog_error("length < 0");
			return ERR_LENGTH_NEGATIVE;
		}

		clear();

		_tid = tid;

		uint8_t* buffer = new uint8_t[length];

		unsigned int once = recv(sockfd, buffer, length, MSG_WAITALL);
		if (once != length)
		{
			dzlog_error("recv failed: %d", errno);
			delete[] buffer;
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		uint32_t count = length / 6;

		uint32_t* ips_buffer = (uint32_t*)buffer;
		for (unsigned int i = 0; i < count; ++i)
		{
			_ips.push_back(ips_buffer[i]);
		}

		uint16_t* ports_buffer = (uint16_t*)(buffer + count * sizeof(uint32_t));
		for (unsigned int i = 0; i < count; ++i)
		{
			_ports.push_back(ports_buffer[i]);
		}

		delete[] buffer;
		return 0;
	}

	int push_loadbalance_pull_message_impl::full_data_direct(uint32_t tid, const std::vector<uint32_t>& ips, const std::vector<uint16_t>& ports)
	{
		_tid	= tid;
		_ips	= ips;
		_ports	= ports;

		return 0;
	}

	int push_loadbalance_pull_message_impl::give_me_data(uint32_t& tid, std::vector<uint32_t>& ips, std::vector<uint16_t>& ports)
	{
		tid		= _tid;
		ips		= _ips;
		ports	= _ports;

		return 0;
	}

	void push_loadbalance_pull_message_impl::print_data()
	{
		dzlog_info("tid: %d", _tid);

		if (_ips.size() != _ports.size())
		{
			dzlog_error("ips size != ports size");
			clear();
			return;
		}

		int size = _ips.size();
		for (int i = 0; i < size; ++i)
		{
			char ip_buffer[16] = { 0 };
			snprintf(ip_buffer, 16, "%u.%u.%u.%u", (_ips[i] & 0x000000ff), (_ips[i] & 0x0000ff00) >> 8, (_ips[i] & 0x00ff0000) >> 16, (_ips[i] & 0xff000000) >> 24);

			dzlog_info("ip:port %s:%d", ip_buffer, ntohs(_ports[i]));
		}
	}

	void push_loadbalance_pull_message_impl::init()
	{
		clear();
	}

	void push_loadbalance_pull_message_impl::clear()
	{
		_ips.clear();
		_ports.clear();
		_tid = 0;
	}

	push_loadbalance_pull_message_impl::push_loadbalance_pull_message_impl()
	{
		clear();
	}

	push_loadbalance_pull_message_impl::~push_loadbalance_pull_message_impl()
	{
		clear();
	}
}
