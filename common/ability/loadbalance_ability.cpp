#include <thread>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include <ifaddrs.h>
#include <algorithm>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "loadbalance.h"
#include "message_headers.h"
#include "threadpool_instance.h"
#include "net_message_listener.h"
#include "loadbalance_ability.h"

int loadbalance_ability::fresh_or_insert_loadbalance(const connection conn, const uint32_t sid, const uint16_t listening_port, const uint32_t load)
{
	const int sock = conn.show_sockfd();

	std::scoped_lock lk(_loadbalance_map_lock);

	if (_loadbalance_map.count(sid))
	{
		dzlog_info("fresh socket %d@%s:%d", sock, conn.show_ip(), conn.show_port());

		_loadbalance_map[sid]->fresh();
		_loadbalance_map[sid]->set_load(load);
		return 0;
	}

	if (insert_loadbalance_sock_sid(sock, sid))
	{
		// 这里就算是失败也不要返回
		dzlog_info("mapping %d->%d already exist", sock, sid);
	}

	std::shared_ptr<peer> p = std::make_shared<peer>(conn, sid, listening_port);
	p->set_load(load);
	_loadbalance_map[sid] = p;

	return 1;
}

int loadbalance_ability::fresh_loadbalance(int sock)
{
	int sid = loadbalance_sockfd_2_sid(sock);
	if (sid == -1)
		return -1;

	{
		std::scoped_lock lk(_loadbalance_map_lock);

		if (!_loadbalance_map.count(sid))
		{
			dzlog_error("cannot find lb by sid: %d", sid);
			return -2;
		}
	}

	_loadbalance_map[sid]->fresh();
	return 0;
}

void loadbalance_ability::shoting_loadbalance()
{
	std::scoped_lock lk(_loadbalance_map_lock);

	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, _self_port, get_load());

	std::shared_ptr<pull_other_loadbalance_message> lb_mess = std::make_shared<pull_other_loadbalance_message>();
	lb_mess->full_data_direct(get_tid());

	dzlog_info("shoting %ld loadbalance", _loadbalance_map.size());

	for (auto i = _loadbalance_map.begin(); i != _loadbalance_map.end(); )
	{
		std::shared_ptr<peer> p = i->second;

		if (p->is_expires()) 
		{
			dzlog_info("peer %s:%d expires", p->get_connection().show_ip(), p->get_connection().show_port());

			// 这里不清除其他资源, 因为死锁 在其他地方查到没有的时候, 清除
			_loadbalance_map.erase(i++);
		}
		else
		{
			p->get_connection().send_message(mess);
			p->get_connection().send_message(lb_mess);

			++i;
		}
	}
}

void loadbalance_ability::set_loadbalance_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(ip, port);
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	std::scoped_lock lk(_loadbalance_2_is_connect_map_lock);
	_loadbalance_2_is_connect_map[k] = v;
}

void loadbalance_ability::set_loadbalance_map(std::vector<uint32_t> sids, std::vector<uint32_t> ips, std::vector<uint16_t> ports, bool is_connect)
{
	std::scoped_lock lk(_loadbalance_2_is_connect_map_lock);

	for (unsigned int i = 0; i < sids.size(); ++i)
	{
		uint32_t sid	= sids[i];
		uint32_t ip		= ips[i];
		uint16_t port	= ports[i];

		std::pair<uint32_t, uint16_t>	k = std::make_pair(ip, port);
		std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

		_loadbalance_2_is_connect_map[k] = v;

		dzlog_info("get lb %d", sid);
	}
}

void loadbalance_ability::set_new_loadbalance_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect)
{
	if (sid == _sid) return;

	std::pair<uint32_t, uint16_t>	k = std::make_pair(ip, htons(port));
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	for (auto i = _self_ip.begin(); i != _self_ip.end(); ++i)
	{
		if (htons(_self_port) != port) break;
		if (*i == ip) return;
	}

	std::scoped_lock lk(_loadbalance_2_is_connect_map_lock);
	if (_loadbalance_2_is_connect_map.count(k)) return;
	_loadbalance_2_is_connect_map[k] = v;
}

void loadbalance_ability::set_new_loadbalance_map(std::vector<uint32_t> sids, std::vector<uint32_t> ips, std::vector<uint16_t> ports, bool is_connect)
{
	std::scoped_lock lk(_loadbalance_2_is_connect_map_lock);

	for (unsigned int i = 0; i < sids.size(); ++i)
	{
		bool same_one	= false;

		uint32_t sid	= sids[i];
		uint32_t ip		= ips[i];
		uint16_t port	= ports[i];

		if (sid == _sid) continue;

		std::pair<uint32_t, uint16_t>	k = std::make_pair(ip, htons(port));
		std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

		for (auto j = _self_ip.begin(); j != _self_ip.end(); ++j)
		{
			if (htons(_self_port) != port) break;
			if (*j == ip)
			{
				same_one = true;
				break;
			}
		}

		if (same_one) continue;

		if (_loadbalance_2_is_connect_map.count(k)) continue;
		_loadbalance_2_is_connect_map[k] = v;
	}
}

void loadbalance_ability::erase_loadbalance_map(uint32_t ip, uint16_t port)
{
	std::pair<uint32_t, uint16_t> k = std::make_pair(ip, port);

	std::scoped_lock lk(_loadbalance_2_is_connect_map_lock);
	if (!_loadbalance_2_is_connect_map.count(k))
		return;

	_loadbalance_2_is_connect_map .erase(k);
}

void loadbalance_ability::rolling_loadbalance_map()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, _self_port, get_load());

	{
		std::scoped_lock lk(_loadbalance_2_is_connect_map_lock);

		for (auto i = _loadbalance_2_is_connect_map.begin(); i != _loadbalance_2_is_connect_map.end(); ++i)
		{
			std::pair<uint32_t, uint16_t> ip_port = i->first;
			std::pair<bool, uint32_t> connected_sid = i->second;

			uint32_t ip		= ip_port.first;
			uint32_t port	= ip_port.second;

			bool is_skip		= false;
			char ip_buffer[16]	= { 0 }; 
			snprintf(ip_buffer, 16, "%u.%u.%u.%u", (ip & 0x000000ff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);

			if (connected_sid.first) continue;

			for (auto i = _loadbalance_sids.begin(); i != _loadbalance_sids.end(); ++i)
			{
				if (*i == connected_sid.second) 
				{
					dzlog_info("skip %s:%d", ip_buffer, ntohs(port));
					is_skip = true;
					break;
				}
			}

			if (is_skip) continue;

			int sock = net_message_listener::get_instance()->connect_with_message(ip, port, mess);
			if (-1 == sock)
			{
				dzlog_error("failed to connect to %s:%d", ip_buffer, ntohs(port));
				continue;
			}

			insert_loadbalance_sock_sid(sock, connected_sid.second);
			_loadbalance_sids.push_back(connected_sid.second);

			dzlog_info("sid %d connect success", connected_sid.second);
			continue;
		}
	}
}

int loadbalance_ability::loadbalance_sockfd_2_sid(int sock)
{
	{
		std::scoped_lock lk(_sock_2_sid_lock);

		if (!_sock_2_sid.count(sock))
		{
			dzlog_error("search sock %d not exist", sock);
			return -1;
		}
	}

	return _sock_2_sid[sock];
}

int loadbalance_ability::loadbalance_sockfd_2_sid(std::vector<uint32_t>& lbsids, const std::vector<uint32_t>& lbfds)
{
	std::scoped_lock lk(_sock_2_sid_lock);
	for (auto i = lbfds.begin(); i != lbfds.end(); ++i)
	{
		if (!_sock_2_sid.count(*i))
		{
			dzlog_error("cannot find sid by socket %d", *i);
			continue;
		}

		lbsids.push_back(_sock_2_sid[*i]);
	}

	return 0;
}

int loadbalance_ability::insert_loadbalance_sock_sid(int sock, uint32_t sid)
{
	{
		std::scoped_lock lk(_sock_2_sid_lock);

		if (_sock_2_sid.count(sock))
		{
			dzlog_error("insert a sock %d already exist", sock);
			return 1;
		}

		_sock_2_sid[sock] = sid;
	}

	dzlog_info("mapping %d->%d", sock, sid);
	return 0;
}

int loadbalance_ability::remove_loadbalance_sock_sid(int sock)
{
	{
		std::scoped_lock lk(_sock_2_sid_lock);

		if (!_sock_2_sid.count(sock))
		{
			dzlog_error("try to erase socket %d not exist", sock);
			return -1;
		}

		_sock_2_sid.erase(sock);
	}

	dzlog_info("erase socket %d", sock);
	return 0;
}

