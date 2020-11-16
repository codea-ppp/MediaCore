#include <thread>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include <ifaddrs.h>
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

	_status = 0;

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

int server::get_ssrc(int which, int id, uint32_t tid)
{
	std::pair<int, int> key = std::make_pair(id, tid);

	std::lock_guard<std::mutex> lk(_id_tid_2_ssrc_lock[which]);
	if (!_id_tid_2_ssrc[which].count(key))
		return -1;

	return _id_tid_2_ssrc[which][key];
}

int server::insert_id_tid_2_ssrc(int which, int id, uint32_t tid, uint32_t ssrc)
{
	std::pair<int, int> key = std::make_pair(id, tid);

	std::lock_guard<std::mutex> lk(_id_tid_2_ssrc_lock[which]);
	if (_id_tid_2_ssrc[which].count(key))
	{
		dzlog_error("[id, tid](%d, %d) already exist", id, tid);
		return -1;
	}

	_id_tid_2_ssrc[which][key] = ssrc;
	return 0;
}

int server::remove_id_tid_2_ssrc(int which, int id, uint32_t tid)
{
	std::pair<int, int> key = std::make_pair(id, tid);

	std::lock_guard<std::mutex> lk(_id_tid_2_ssrc_lock[which]);
	if (!_id_tid_2_ssrc[which].count(key))
	{
		dzlog_info("[id, tid](%d, %d) not exist", id, tid);
		return 0;
	}

	_id_tid_2_ssrc[which].erase(key);

	return 0;
}

void server::set_lb_map(uint32_t ip, uint16_t port, bool status)
{
	std::lock_guard<std::mutex> lk(_lb_ip_port_2_is_connect_lock);
	_lb_ip_port_2_is_connect[std::make_pair(ip, port)] = status;
}

void server::set_new_lb_map(uint32_t ip, uint16_t port, bool status)
{
	for (auto i = _self_ip.begin(); i != _self_ip.end(); ++i)
	{
		if (_self_port != port)
			break;

		if (*i == ip)
			return;
	}

	std::pair<uint32_t, uint16_t> key = std::make_pair(ip, port);

	std::lock_guard<std::mutex> lk(_lb_ip_port_2_is_connect_lock);

	if (!_lb_ip_port_2_is_connect.count(key)) return;

	_lb_ip_port_2_is_connect[key] = status;
}

void server::erase_lb_map(uint32_t ip, uint16_t port)
{
	std::pair<uint32_t, uint16_t> key = std::make_pair(ip, port);

	std::lock_guard<std::mutex> lk(_lb_ip_port_2_is_connect_lock);
	if (!_lb_ip_port_2_is_connect.count(key))
		return;
	
	_lb_ip_port_2_is_connect.erase(key);
}

void server::rolling_lb_map()
{
	std::shared_ptr<keepalive_message> mess = std::make_shared<keepalive_message>();
	mess->full_data_direct(get_tid(), _sid, get_load());

	{
		std::lock_guard<std::mutex> lk(_lb_ip_port_2_is_connect_lock);

		for (auto i = _lb_ip_port_2_is_connect.begin(); i != _lb_ip_port_2_is_connect.end(); ++i)
		{
			if (i->second) continue;

			uint32_t ip		= i->first.first;
			uint32_t port	= i->first.second;

			net_message_listener::get_instance()->connect_with_message(ip, port, mess);
		}
	}
	
	std::this_thread::sleep_for(std::chrono::seconds(60));
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
