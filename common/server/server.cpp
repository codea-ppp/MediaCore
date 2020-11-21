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

int server::deal_message(const connection, std::shared_ptr<resource_server_respond_media_pull_message> mess)
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
