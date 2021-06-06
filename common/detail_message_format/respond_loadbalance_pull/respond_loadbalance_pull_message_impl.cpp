#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "message_type.h"
#include "respond_loadbalance_pull_message_impl.h"

namespace media_core_message
{
	int respond_loadbalance_pull_message_impl::send_data_to(int sockfd)
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

		if (_sids.size() != _ips.size() || _ips.size() != _ports.size())
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
		unsigned int all = 10 * size + 12;

		uint8_t* buffer = new uint8_t[all];
		memset(buffer, 0, all);

		uint32_t* head_buffer = (uint32_t*)buffer;
		head_buffer[0] = MSGTYPE_RESPONDLOADBALANCEPULL;
		head_buffer[1] = 10 * size;
		head_buffer[2] = _tid;

		uint32_t* sid_buffer	= (uint32_t*)(buffer + 12);
		for (unsigned int i = 0; i < size; ++i) sid_buffer[i]	= _sids[i];

		uint32_t* ip_buffer		= (uint32_t*)(buffer + 12 + size * 4);
		for (unsigned int i = 0; i < size; ++i) ip_buffer[i]	= _ips[i];

		uint16_t* port_buffer	= (uint16_t*)(buffer + 12 + size * 4 * 2);
		for (unsigned int i = 0; i < size; ++i) port_buffer[i]	= _ports[i];

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

	int respond_loadbalance_pull_message_impl::full_data_remote(int sockfd, uint32_t tid, uint32_t length)
	{
		if (sockfd < 0)
		{
			dzlog_error("socket < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		if (length < 0 || length % 10 != 0)
		{
			dzlog_error("length %d", length);
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

		uint32_t count = length / 10;

		uint32_t* sids_buffer	= (uint32_t*)buffer;
		for (unsigned int i = 0; i < count; ++i) _sids.push_back(sids_buffer[i]);

		uint32_t* ips_buffer	= (uint32_t*)(buffer + count * sizeof(uint32_t)); 
		for (unsigned int i = 0; i < count; ++i) _ips.push_back(ips_buffer[i]);

		uint16_t* ports_buffer	= (uint16_t*)(buffer + count * sizeof(uint32_t) * 2);
		for (unsigned int i = 0; i < count; ++i) _ports.push_back(ports_buffer[i]);

		delete[] buffer;
		return 0;
	}

	int respond_loadbalance_pull_message_impl::full_data_direct(uint32_t tid, const std::vector<uint32_t>& sids, const std::vector<uint32_t>& ips, const std::vector<uint16_t>& ports)
	{
		_tid	= tid;
		_sids	= sids;
		_ips	= ips;
		_ports	= ports;

		return 0;
	}

	int respond_loadbalance_pull_message_impl::give_me_data(uint32_t& tid, std::vector<uint32_t>& sids, std::vector<uint32_t>& ips, std::vector<uint16_t>& ports)
	{
		tid		= _tid;
		sids	= _sids;
		ips		= _ips;
		ports	= _ports;

		return 0;
	}

	void respond_loadbalance_pull_message_impl::print_data()
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

			dzlog_info("ip:port %d@%s:%d", _sids[i], ip_buffer, ntohs(_ports[i]));
		}
	}

	void respond_loadbalance_pull_message_impl::init()
	{
		clear();
	}

	void respond_loadbalance_pull_message_impl::clear()
	{
		_ips.clear();
		_ports.clear();
		_tid = 0;
	}

	respond_loadbalance_pull_message_impl::respond_loadbalance_pull_message_impl()
	{
		clear();
	}

	respond_loadbalance_pull_message_impl::~respond_loadbalance_pull_message_impl()
	{
		clear();
	}
}
