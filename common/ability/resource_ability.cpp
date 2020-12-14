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
#include "resource_ability.h"
#include "threadpool_instance.h"
#include "net_message_listener.h"

int resource_ability::fresh_or_insert_resource(const connection conn, const uint32_t sid, const uint16_t listening_port, const uint32_t load)
{
	const int sock = conn.show_sockfd();
	std::scoped_lock lk(_resource_map_lock);

	if (_resource_map.count(sid))
	{
		dzlog_info("fresh socket %d@%s:%d", sock, conn.show_ip(), conn.show_port());

		_resource_map[sid]->fresh();
		_resource_map[sid]->set_load(load);
		return 0;
	}

	if (1 == insert_resource_sock_sid(sock, sid))
	{
		// 这里就算是失败也不要返回
		dzlog_info("mapping %d->%d already exist", sock, sid);
	}

	std::shared_ptr<peer> p = std::make_shared<peer>(conn, sid, listening_port);
	p->set_load(load);
	_resource_map[sid] = p;

	return 1;
}

int resource_ability::fresh_resource(int sock)
{
	uint32_t sid = resource_sockfd_2_sid(sock);

	{
		std::scoped_lock lk(_resource_map_lock);

		if (!_resource_map.count(sid))
		{
			dzlog_error("fresh resource %d not exist", sock);
			return -1;
		}
	}

	_resource_map[sid]->fresh();
	return 0;
}

void resource_ability::shoting_resource()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, _self_port, get_load());

	std::scoped_lock lk(_resource_map_lock);
	dzlog_info("shoting %ld resource", _resource_map.size());

	for (auto i = _resource_map.begin(); i != _resource_map.end(); )
	{
		std::shared_ptr<peer> p = i->second;
		if (p->is_expires()) 
		{
			dzlog_info("peer %s:%d expires", p->get_connection().show_ip(), p->get_connection().show_port());

			// 这里不清除其他资源, 因为死锁 在其他地方查到没有的时候, 清除
			_resource_map.erase(i++);
		}
		else
		{
			p->get_connection().send_message(mess);
			++i;
		}
	}
}

void resource_ability::set_resource_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(ip, port);
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	std::scoped_lock lk(_resource_2_is_connect_map_lock);
	_resource_2_is_connect_map[k] = v;
}

void resource_ability::set_new_resource_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(ip, port);
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	for (auto i = _self_ip.begin(); i != _self_ip.end(); ++i)
	{
		if (htons(_self_port) != port) break;
		if (*i == ip) return;
	}

	std::scoped_lock lk(_resource_2_is_connect_map_lock);
	if (!_resource_2_is_connect_map.count(k)) return;
	_resource_2_is_connect_map[k] = v;
}

void resource_ability::erase_resource_map(uint32_t ip, uint16_t port)
{
	std::pair<uint32_t, uint16_t> k = std::make_pair(ip, port);

	std::scoped_lock lk(_resource_2_is_connect_map_lock);
	if (!_resource_2_is_connect_map.count(k))
		return;

	_resource_2_is_connect_map .erase(k);
}

void resource_ability::rolling_resource_map()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, _self_port, get_load());

	{
		std::scoped_lock lk(_resource_2_is_connect_map_lock);

		for (auto i = _resource_2_is_connect_map.begin(); i != _resource_2_is_connect_map.end(); ++i)
		{
			std::pair<uint32_t, uint16_t> ip_port = i->first;
			std::pair<bool, uint32_t> connected_sid = i->second;

			uint32_t ip		= i->first.first;
			uint32_t port	= i->first.second;

			if (ip_port.first) 
			{
				_resource_sids.push_back(connected_sid.second);
				continue;
			}

			bool is_skip		= false;
			char ip_buffer[16]	= { 0 }; 
			snprintf(ip_buffer, 16, "%u.%u.%u.%u", (ip & 0x000000ff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);

			if (connected_sid.first) continue;

			for (auto i = _resource_sids.begin(); i != _resource_sids.end(); ++i)
			{
				if (*i == connected_sid.second) 
				{
					dzlog_info("skip %s:%d", ip_buffer, ntohs(port));
					is_skip = true;
					break;
				}
			}

			if (is_skip) continue;

			if (net_message_listener::get_instance()->connect_with_message(ip, port, mess))
			{
				char ip_buffer[16] = { 0 }; 
				snprintf(ip_buffer, 16, "%u.%u.%u.%u", (ip & 0x000000ff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);

				dzlog_error("failed to connect to %s:%d", ip_buffer, ntohs(port));
			}

			_resource_sids.push_back(connected_sid.second);
			dzlog_info("sid %d connect success", connected_sid.second);
		}
	}
}

int resource_ability::resource_sockfd_2_sid(int sock)
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

int resource_ability::insert_resource_sock_sid(int sock, uint32_t sid)
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

int resource_ability::remove_resource_sock_sid(int sock)
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

