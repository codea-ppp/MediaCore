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

server::server()
{
	load[0] = load[1] = load_index = _sid = 0;

	_self_port	= 0;
	_status		= 0;

	struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa	= nullptr;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) 
	{
        if (!ifa->ifa_addr) 
			continue;

        if (ifa->ifa_addr->sa_family == AF_INET) 
            _self_ip.push_back((uint32_t)((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr);
    }

    if (ifAddrStruct != NULL) 
		freeifaddrs(ifAddrStruct);
}

int server::tell_me_type(uint32_t sid)
{
	return (sid & 0xff000000) >> 24;
}

uint32_t server::get_load()
{
	// !load_index 已经是采集的负载, load_index 是当前正在采集的
	return load[!load_index];
}

void server::inc_load()
{
	++load[load_index];
}

void server::swi_load()
{
	load_index = !load_index;
	load[load_index] = 0;
}

uint32_t server::get_tid()
{
	if (_tid_boundary < 100)
		_tid_boundary = 100;

	return ++_tid_boundary;
}

int server::get_ssrc(int which, int sid, uint32_t tid)
{
	std::pair<int, int> key = std::make_pair(sid, tid);

	std::lock_guard<std::mutex> lk(_sid_tid_2_ssrc_lock[which]);
	if (!_sid_tid_2_ssrc[which].count(key))
		return -1;

	return _sid_tid_2_ssrc[which][key];
}

int server::insert_id_tid_2_ssrc(int which, int sid, uint32_t tid, uint32_t ssrc)
{
	std::pair<int, int> key = std::make_pair(sid, tid);

	std::lock_guard<std::mutex> lk(_sid_tid_2_ssrc_lock[which]);
	if (_sid_tid_2_ssrc[which].count(key))
	{
		dzlog_error("[sid, tid](%d, %d) already exist", sid, tid);
		return -1;
	}

	_sid_tid_2_ssrc[which][key] = ssrc;
	return 0;
}

int server::remove_id_tid_2_ssrc(int which, int sid, uint32_t tid)
{
	std::pair<int, int> key = std::make_pair(sid, tid);

	std::lock_guard<std::mutex> lk(_sid_tid_2_ssrc_lock[which]);
	if (!_sid_tid_2_ssrc[which].count(key))
	{
		dzlog_info("[sid, tid](%d, %d) not exist", sid, tid);
		return 0;
	}

	_sid_tid_2_ssrc[which].erase(key);

	return 0;
}

void server::set_client_map(uint32_t sid, const connection conn, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	std::lock_guard<std::mutex> lk(_client_2_is_connect_map_lock);
	_client_2_is_connect_map[k] = v;
}

void server::set_new_client_map(uint32_t sid, const connection conn, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	for (auto i = _self_ip.begin(); i != _self_ip.end(); ++i)
	{
		if (_self_port != conn.show_port_raw()) break;
		if (*i == conn.show_ip_raw())			return;

		dzlog_info("set a new lb %s:%d", conn.show_ip(), conn.show_port());
	}

	std::lock_guard<std::mutex> lk(_client_2_is_connect_map_lock);
	if (!_client_2_is_connect_map.count(k)) return;
	_client_2_is_connect_map[k] = v;
}

void server::erase_client_map(const connection conn)
{
	std::pair<uint32_t, uint16_t> k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());

	std::lock_guard<std::mutex> lk(_client_2_is_connect_map_lock);
	if (!_client_2_is_connect_map.count(k))
		return;

	_client_2_is_connect_map .erase(k);
}

void server::rolling_client_map()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, get_load());

	int n;

	{
		std::lock_guard<std::mutex> lk(_client_2_is_connect_map_lock);

		for (auto i = _client_2_is_connect_map.begin(); i != _client_2_is_connect_map.end(); ++i)
		{
			std::pair<uint32_t, uint16_t> ip_port = i->first;
			std::pair<bool, uint32_t> connected_sid = i->second;

			if (ip_port.first) 
			{
				_client_sids.push_back(connected_sid.second);
				continue;
			}

			if (_client_sids.end() != std::find(_client_sids.begin(), _client_sids.end(), n)) continue;

			uint32_t ip		= i->first.first;
			uint32_t port	= i->first.second;

			if (net_message_listener::get_instance()->connect_with_message(ip, port, mess))
			{
				char ip_buffer[16] = { 0 }; 
				snprintf(ip_buffer, 16, "%u.%u.%u.%u", (ip & 0x000000ff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);

				dzlog_error("failed to connect to %s:%d", ip_buffer, ntohs(port));
			}

			_client_sids.push_back(connected_sid.second);
			dzlog_info("sid %d connect success", connected_sid.second);
		}
	}
}

void server::set_loadbalance_map(uint32_t sid, const connection conn, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	std::lock_guard<std::mutex> lk(_loadbalance_2_is_connect_map_lock);
	_loadbalance_2_is_connect_map[k] = v;
}

void server::set_new_loadbalance_map(uint32_t sid, const connection conn, bool is_connect)
{
	if (sid == _sid) return;

	std::pair<uint32_t, uint16_t>	k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	for (auto i = _self_ip.begin(); i != _self_ip.end(); ++i)
	{
		if (_self_port != conn.show_port_raw()) break;
		if (*i == conn.show_ip_raw())			return;

		dzlog_info("set a new lb %s:%d", conn.show_ip(), conn.show_port());
	}

	std::lock_guard<std::mutex> lk(_loadbalance_2_is_connect_map_lock);
	if (!_loadbalance_2_is_connect_map.count(k)) return;
	_loadbalance_2_is_connect_map[k] = v;
}

void server::erase_loadbalance_map(const connection conn)
{
	std::pair<uint32_t, uint16_t> k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());

	std::lock_guard<std::mutex> lk(_loadbalance_2_is_connect_map_lock);
	if (!_loadbalance_2_is_connect_map.count(k))
		return;

	_loadbalance_2_is_connect_map .erase(k);
}

void server::rolling_loadbalance_map()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, get_load());

	{
		std::lock_guard<std::mutex> lk(_loadbalance_2_is_connect_map_lock);

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
				}
			}

			if (is_skip) continue;

			int sock = net_message_listener::get_instance()->connect_with_message(ip, port, mess);
			if (-1 == sock)
			{
				dzlog_error("failed to connect to %s:%d", ip_buffer, ntohs(port));
				continue;
			}

			insert_sock_sid(sock, connected_sid.second);
			_loadbalance_sids.push_back(connected_sid.second);

			dzlog_info("sid %d connect success", connected_sid.second);
			continue;
		}
	}
}

void server::set_resource_map(uint32_t sid, const connection conn, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	std::lock_guard<std::mutex> lk(_resource_2_is_connect_map_lock);
	_resource_2_is_connect_map[k] = v;
}

void server::set_new_resource_map(uint32_t sid, const connection conn, bool is_connect)
{
	std::pair<uint32_t, uint16_t>	k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());
	std::pair<bool, uint32_t>		v = std::make_pair(is_connect, sid);

	for (auto i = _self_ip.begin(); i != _self_ip.end(); ++i)
	{
		if (_self_port != conn.show_port_raw()) break;
		if (*i == conn.show_ip_raw())			return;

		dzlog_info("set a new lb %s:%d", conn.show_ip(), conn.show_port());
	}

	std::lock_guard<std::mutex> lk(_resource_2_is_connect_map_lock);
	if (!_resource_2_is_connect_map.count(k)) return;
	_resource_2_is_connect_map[k] = v;
}

void server::erase_resource_map(const connection conn)
{
	std::pair<uint32_t, uint16_t> k = std::make_pair(conn.show_ip_raw(), conn.show_port_raw());

	std::lock_guard<std::mutex> lk(_resource_2_is_connect_map_lock);
	if (!_resource_2_is_connect_map.count(k))
		return;

	_resource_2_is_connect_map .erase(k);
}

void server::rolling_resource_map()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, get_load());

	int n;

	{
		std::lock_guard<std::mutex> lk(_resource_2_is_connect_map_lock);

		for (auto i = _resource_2_is_connect_map.begin(); i != _resource_2_is_connect_map.end(); ++i)
		{
			std::pair<uint32_t, uint16_t> ip_port = i->first;
			std::pair<bool, uint32_t> connected_sid = i->second;

			if (ip_port.first) 
			{
				_resource_sids.push_back(connected_sid.second);
				continue;
			}

			if (_resource_sids.end() != std::find(_resource_sids.begin(), _resource_sids.end(), n)) continue;

			uint32_t ip		= i->first.first;
			uint32_t port	= i->first.second;

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

void server::rolling_map()
{
	while (_status)
	{
		threadpool_instance::get_instance()->schedule(std::bind(&server::rolling_client_map, this));
		threadpool_instance::get_instance()->schedule(std::bind(&server::rolling_loadbalance_map, this));
		threadpool_instance::get_instance()->schedule(std::bind(&server::rolling_resource_map, this));
		
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

	dzlog_info("map rolling end");
}

int server::sockfd_2_sid(int sock)
{
	{
		std::lock_guard<std::mutex> lk(_sock_2_sid_lock);

		if (!_sock_2_sid.count(sock))
		{
			dzlog_error("search sock %d not exist", sock);
			return -1;
		}
	}

	return _sock_2_sid[sock];
}

int server::insert_sock_sid(int sock, uint32_t sid)
{
	{
		std::lock_guard<std::mutex> lk(_sock_2_sid_lock);

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

int server::remove_sock_sid(int sock)
{
	{
		std::lock_guard<std::mutex> lk(_sock_2_sid_lock);

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

int server::deal_message(const connection, std::shared_ptr<client_pull_media_stream_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<client_stream_trigger_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<keepalive_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<loadbalance_pull_media_stream_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<loadbalance_respond_media_menu_pull_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<pull_media_menu_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<pull_other_loadbalance_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<push_loadbalance_pull_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<push_media_pull_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<resource_server_report_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<resource_server_respond_media_menu_pull_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<respond_loadbalance_pull_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<respond_media_menu_pull_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<stop_stream_message> mess)
{
	mess->print_data();
	return 0;
}

int server::deal_message(const connection, std::shared_ptr<stream_message> mess)
{
	mess->print_data();
	return 0;
}
