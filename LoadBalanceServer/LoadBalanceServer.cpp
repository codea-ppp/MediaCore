#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include <linux/if_link.h>
#include "LoadBalanceServer.h"

void NetMessageListenerCallBack(const connection& conn, void* message)
{
	LoadBalanceServer::GetInstance()->GetCallBack(conn, message);
}

void LoadBalanceServer::GetCallBack(const connection conn, void* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	dzlog_info("get message from %d@%s:%d", conn.sockfd, conn.ip.c_str(), conn.port);
	inc_load();

	uint32_t* _message = (uint32_t*)message;

	uint32_t type = _message[0];

	// 这里是接受到的信令
	switch (type)
	{
	case MSGTYPE_KEEPALIVE: 
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_keepalive, 
			this, conn, (KeepAliveMessage*)message));

		break;

	case MSGTYPE_PULLOTHERLOADBALANCE:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_pullotherloadbalance, 
			this, conn, (PullOtherLoadBalanceMessage*)message));

		break;

	case MSGTYPE_RESPONDLOADBALANCEPULL:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_respondpullotherloadbalance, 
			this, conn, (RespondLoadBalancePullMessage*)message));

		break;

	case MSGTYPE_PUSHMEDIAMENU:	
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_pushmediamenu,
			this, conn, (PushMediaMenuMessage*)message));

		break;

	case MSGTYPE_PULLMEDIAMENU:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_pullmediamenu, 
			this, conn, (PullMediaMenuMessage*)message));

		break;

	case MSGTYPE_RESPONDMEDIAMENU:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_respondpullmediamenu, 
			this, conn, (RespondMediaMenuPullMessage*)message));

		break;

	case MSGTYPE_CLIENTPULLMEDIASTREAM:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_clientpullstream, 
			this, conn, (ClientPullMediaStreamMessage*)message));

		break;

	case MSGTYPE_RESOURCERESPONDMEDIAPULL:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_resourcerespondpullstream, 
			this, conn, (ResourceServerRespondMediaPullMessage*)message));

		break;

	case MSGTYPE_STOPSTREAM:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_stopstream, 
			this, conn, (StopStreamMessage*)message));

		break;

	case MSGTYPE_RESOURCESERVERREPORT:
		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::deal_resourcereport, 
			this, conn, (ResourceServerReportMessage*)message));

		break;

	case MSGTYPE_PUSHLOADBANLANCE: 
		ClearRespondLoadBalancePullMessage((PushLoadBalanceMessage*)message); 
		delete (PushLoadBalanceMessage*)message; 
		break;

	case MSGTYPE_LOADBALANCEPULLMEDIASTREAM: 
		ClearLoadBalancePullMediaStreamMessage((LoadBalancePullMediaStreamMessage*)message);
		delete (LoadBalancePullMediaStreamMessage*)message;
		break;

	case MSGTYPE_LOADBALANCERESPONDMEDIAPULL:
		delete (LoadBalanceRespondMediaPullMessage*)message;
		break;

	case MSGTYPE_CLIENTSTREAMTRIGGER:
		delete (ClientStreamTriggerMessage*)message;
		break;
		
	case MSGTYPE_STREAMFRAME:
		ClearStreamMessage((StreamMessage*)message);
		delete (StreamMessage*)message;
		break;

	default: 
		dzlog_info("recv a message should not send to loadbalance, type: %d", type);
		break;
	}
}

void LoadBalanceServer::deal_pushmediamenu(const connection conn, PushMediaMenuMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	std::vector<std::string> names;

	for (uint32_t i = 0; i < message->length; ++i)
	{
		names.push_back((char*)message->MediaMetaResourse[i].VideoName);
	}

	{
		std::lock_guard<std::mutex> lk(video_names_lock);

		for (auto i = names.begin(); i != names.end(); ++i)
		{
			if (video_name_map.count(*i) && video_name_map[*i].count(conn.sockfd)) 
				continue;

			video_name_map[*i][conn.sockfd] = conn;
		}
	}
	
	ClearPushMediaMenuMessage(message);
	delete message;
}

void LoadBalanceServer::deal_resourcereport(const connection conn, ResourceServerReportMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		delete message;
		return;
	}

	uint32_t tid = message->tid;
	uint32_t err = message->error;

	connection client;

	{
		std::lock_guard<std::mutex> lk(media_chain_lock);

		if (!media_chain_map.count(tid))
		{
			dzlog_error("unknown tid %d", tid);
			ThreadpoolInstance::GetInstance()->schedule(
				std::bind(&LoadBalanceServer::send_report, 
				this, conn, tid, 6));

			delete message;
			return;
		}

		client = media_chain_map[tid].client;
		media_chain_map.erase(tid);
	}

	ThreadpoolInstance::GetInstance()->schedule(
		std::bind(&LoadBalanceServer::send_report, 
		this, client, tid, err));

	delete message;
}

void LoadBalanceServer::deal_stopstream(const connection conn, StopStreamMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	uint32_t tid	= message->tid;
	uint32_t ssrc	= message->ssrc;

	connection resource;

	{
		std::lock_guard<std::mutex> lk(media_chain_lock);
		if (!media_chain_map.count(tid))
		{
			dzlog_error("stop tid not exist");
			delete message;
			return;
		}

		resource = media_chain_map[tid].resource;
		media_chain_map.erase(tid);
	}

	ThreadpoolInstance::GetInstance()->schedule(
		std::bind(&LoadBalanceServer::send_stopstream, 
		this, resource, tid, ssrc));

	delete message;
}

void LoadBalanceServer::send_stopstream(const connection conn, uint32_t tid, uint32_t ssrc)
{
	StopStreamMessage message;
	InitStopStreamMessage(&message);
	SetStopStreamData(&message, ssrc);
	SetTransactionId(&message, tid);

	if (!SendStopStreamMessage(conn.sockfd, &message))
	{
		dzlog_error("send stop stream to %d@%s:%d failed", conn.sockfd, conn.ip.c_str(), conn.port);

		peer p;
		p.conn = conn;
		p.type = RESOURCE_MAP;

		delete_peer(p);
		delete_shot(p);
	}
}

void LoadBalanceServer::deal_resourcerespondpullstream(const connection conn, ResourceServerRespondMediaPullMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	uint16_t send_port = message->send_port;
	uint32_t tid = message->tid;
	uint32_t ssrc;

	connection client;

	{
		std::lock_guard<std::mutex> lk(media_chain_lock);

		if (!media_chain_map.count(tid))
		{
			dzlog_error("no such media chain %d", tid);

			ThreadpoolInstance::GetInstance()->schedule(
				std::bind(&LoadBalanceServer::send_report, 
				this, conn, tid, 6));

			delete message;
			return;
		}

		media_chain_map[tid].is_stream = true;
		media_chain_map[tid].server_send_port = message->send_port;

		ssrc	= media_chain_map[tid].ssrc;
		client	= media_chain_map[tid].client;
	}

	ThreadpoolInstance::GetInstance()->schedule(
		std::bind(&LoadBalanceServer::send_loadbalancerespondpullstream, 
		this, client, tid, ssrc, send_port));

	delete message;
}

void LoadBalanceServer::send_loadbalancerespondpullstream(const connection conn, uint32_t tid, uint32_t ssrc, uint16_t port)
{
	LoadBalanceRespondMediaPullMessage message;
	InitLoadBalanceRespondMediaPullMessage(&message);
	SetLoadBalanceRespondMediaPullData(&message, ssrc, port);
	SetTransactionId(&message, tid);

	if (!SendLoadBalanceRespondMediaPullMessage(conn.sockfd, &message))
	{
		peer p;
		p.type = CLIENT_MAP;	
		p.conn = conn;

		delete_peer(p);
		delete_shot(p);
	}
}

void LoadBalanceServer::deal_clientpullstream(const connection conn, ClientPullMediaStreamMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	media_chain chain;
	chain.server_send_port = 0;
	chain.client_recv_port = message->receive_port;
	chain.is_stream	= false;
	chain.client	= conn;
	chain.ssrc		= rand();

	std::vector<int> able_resources;
	std::string video_name((char*)message->video_name);
	uint32_t tid = message->tid;

	{
		std::lock_guard<std::mutex> lk(video_names_lock);
		if (!video_name_map.count(video_name))
		{
			ThreadpoolInstance::GetInstance()->schedule(
				std::bind(&LoadBalanceServer::send_report, 
				this, conn, tid, 1));

			ClearClientPullMediaStreamMessage(message);
			delete message;
			return;
		}

		for (auto i = video_name_map[video_name].begin(); i != video_name_map[video_name].end(); ++i)
		{
			able_resources.push_back(i->first);
		}
	}

	int current_count	= INT_MAX;
	int current_fd		= 0;

	{
		std::lock_guard<std::mutex> lk(server_lock[RESOURCE_MAP]);

		for (auto i = able_resources.begin(); i != able_resources.end(); ++i)
		{
			if (server[RESOURCE_MAP][*i].count < current_count)
			{
				current_count = server[RESOURCE_MAP][*i].count;
				current_fd = *i;
			}
		}

		if (!current_fd)
		{
			ThreadpoolInstance::GetInstance()->schedule(
				std::bind(&LoadBalanceServer::send_report, 
				this, conn, tid, 3));

			ClearClientPullMediaStreamMessage(message);
			delete message;
			return;
		}

		chain.resource = server[RESOURCE_MAP][current_fd].conn;
	}

	chain.last_flesh = std::chrono::steady_clock::now();

	{
		std::lock_guard<std::mutex> lk(media_chain_lock);

		if (media_chain_map.count(tid))
		{
			dzlog_error("client %d@%s:%d pull stream with same tid: %d", conn.sockfd, conn.ip.c_str(), conn.port, tid);

			ThreadpoolInstance::GetInstance()->schedule(
				std::bind(&LoadBalanceServer::send_report,
				this, conn, tid, 2));

			ClearClientPullMediaStreamMessage(message);
			delete message;
			return;
		}

		media_chain_map[tid] = chain;
	}

	LoadBalancePullMediaStreamMessage _message;
	InitLoadBalancePullMediaStreamMessage(&_message);
	SetTransactionId(&_message, tid);
	SetLoadBalancePullMediaStreamData(&_message, video_name, chain.ssrc, chain.client_recv_port);

	if (!SendLoadBalancePullMediaStreamMessage(current_fd, &_message))
	{
		peer p;
		p.conn = chain.resource;
		p.type = RESOURCE_MAP;

		delete_peer(p);
		delete_shot(p);

		std::lock_guard<std::mutex> lk(media_chain_lock);
		media_chain_map.erase(tid);
	}
	
	ClearClientPullMediaStreamMessage(message);
	delete message;
}

void LoadBalanceServer::send_report(const connection conn, uint32_t tid, int err)
{
	ResourceServerReportMessage message;
	InitResourceServerReportMessage(&message);
	SetTransactionId(&message, tid);
	SetResourceServerReportData(&message, err);
	SendResourceServerReportMessage(conn.sockfd, &message);
}

void LoadBalanceServer::deal_respondpullmediamenu(const connection conn, RespondMediaMenuPullMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}
	
	std::vector<std::string> video_names;
	for (uint32_t i = 0; i < message->length; ++i)
	{
		video_names.push_back(std::string((char*)message->MediaMetaResourse[i].VideoName));
	}

	{
		std::lock_guard<std::mutex> lk(video_names_lock);

		for (auto i = video_names.begin(); i != video_names.end(); ++i)
		{
			if (video_name_map.count(*i) && video_name_map[*i].count(conn.sockfd))
				continue;

			video_name_map[*i][conn.sockfd] = conn;
		}
	}

	ClearRespondMediaMenuPullMessage(message);
	delete message;
}

void LoadBalanceServer::deal_pullmediamenu(const connection conn, PullMediaMenuMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message ==  nullptr");
		return;
	}

	{
		std::lock_guard<std::mutex> lk(server_lock[CLIENT_MAP]);
		if (server[CLIENT_MAP].count(conn.sockfd))
		{
			dzlog_error("recv a PullMediaMenuMessage not from a client");
			delete message;
			return;
		}
	}

	send_mediamenu(conn);
	delete message;
}

void LoadBalanceServer::send_mediamenu(const connection conn)
{
	RespondMediaMenuPullMessage message;
	InitRespondMediaMenuPullMessage(&message);
	SetTransactionId(&message, rand());

	std::vector<MediaMeta> medias;

	{
		MediaMeta temp;

		std::lock_guard<std::mutex> lk(video_names_lock);
		for (auto i = video_name_map.begin(); i != video_name_map.end(); ++i)
		{
			std::string video_name = i->first;

			temp.VideoNameLength = video_name.size();
			temp.VideoName = new uint8_t[temp.VideoNameLength];

			memcpy(temp.VideoName, video_name.c_str(), temp.VideoNameLength);

			medias.push_back(temp);
		}
	}

	SetMediaMetaData(&message, medias);
	if (!SendRespondMediaMenuPullMessage(conn.sockfd, &message))
	{
		peer p;
		p.conn = conn;
		p.type = CLIENT_MAP;

		delete_peer(p);
		delete_shot(p);
	}

	ClearRespondMediaMenuPullMessage(&message);

	for (auto i = medias.begin(); i != medias.end(); ++i)
	{
		dzlog_info("send video: %s", (char*)i->VideoName);
		delete[] i->VideoName;
	}
}

void LoadBalanceServer::deal_respondpullotherloadbalance(const connection conn, RespondLoadBalancePullMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	if (message->length % 6 != 0)
	{
		dzlog_error("message->length mod 6 != 0");

		ClearRespondLoadBalancePullMessage(message);
		delete message;
	}

	int size = message->length / 6;
	for (int i = 0; i < size; ++i)
	{
		bool is_self = false;
		for (auto j = self_ip.begin(); j != self_ip.end(); ++j)
		{
			if (message->port[i] != _port) break;
			if (message->ip[i] == *j)
			{
				dzlog_info("remove self from loadbalance talbe");
				is_self = true;
				break;
			}
		}

		if (is_self) continue;

		ThreadpoolInstance::GetInstance()->schedule(
			std::bind(&LoadBalanceServer::send_first_keepalive, 
			this, message->ip[i], message->port[i]));
	}

	ClearRespondLoadBalancePullMessage(message);
	delete message;
}

void LoadBalanceServer::send_first_keepalive(uint32_t ip, uint16_t port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in	 addr;
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = ip;
	addr.sin_port		 = htons(port);

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		dzlog_error("connect failed, errno: %d", errno);
		return;
	}

	KeepAliveMessage message;
	InitKeepAliveMessage(&message);
	SetTransactionId(&message, rand());
	SetKeepAliveData(&message, _sid, load[load_index]);

	if (!SendKeepAliveMessage(sockfd, &message))
	{
		dzlog_error("send first keepalive to %d@%d:%d failed", sockfd, ip, port);
		return;
	}

	connection c;
	c.sockfd	= sockfd;
	c.ip		= ip;
	c.port		= port;

	peer p;
	p.type	= LOADBANLANCE_MAP;
	p.count = 0;
	p.conn	= c;

	dzlog_info("add a peer %d@%d:%d", sockfd, ip, port);

	{
		std::lock_guard<std::mutex> lk(shoting_lock);
		waiting_shoting[sockfd] = p;
	}
}

void LoadBalanceServer::deal_pullotherloadbalance(const connection conn, PullOtherLoadBalanceMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	if (server[LOADBANLANCE_MAP].count(conn.sockfd))
	{
		dzlog_error("pull loadbalance from %d@%s:%d neither client or resource", conn.sockfd, conn.ip.c_str(), conn.port);
		delete message;
		return;
	}

	std::vector<uint32_t> ips;
	std::vector<uint16_t> ports;

	{
		std::lock_guard<std::mutex> lk(server_lock[LOADBANLANCE_MAP]);

		for (auto i = server[LOADBANLANCE_MAP].begin(); i != server[LOADBANLANCE_MAP].end(); ++i)
		{
			connection conn = i->second.conn;

			dzlog_info("loadbalance %s:%d going to send", conn.ip.c_str(), conn.port);

			ips.push_back(inet_addr(conn.ip.c_str()));
			ports.push_back(conn.port);
		}

		dzlog_info("total send %ld loadbalance", ips.size());
	}

	RespondLoadBalancePullMessage _message;
	InitRespondLoadBalancePullMessage(&_message);
	SetOtherLoadBalanceData(&_message, ips, ports);
	SetTransactionId(&_message, rand());
	if (!SendRespondLoadBalancePullMessage(conn.sockfd, &_message))
	{
		peer p;
		p.conn = conn;
		p.type = -1;

		delete_peer(p);
		delete_shot(p);
	}

	ClearRespondLoadBalancePullMessage(&_message);

	delete message;
}

void LoadBalanceServer::deal_keepalive(const connection conn, KeepAliveMessage* message)
{
	if (message == nullptr)
	{
		dzlog_error("message == nullptr");
		return;
	}

	peer temp;
	switch (message->sid >> 24)
	{
	case 1: temp.type = RESOURCE_MAP;		break;
	case 2: temp.type = LOADBANLANCE_MAP;	break;
	case 3: temp.type = CLIENT_MAP;			break;
	default:
		dzlog_error("unknown server type");
		return;
	}

	auto now = std::chrono::steady_clock::now();

	temp.conn		= conn;
	temp.count		= message->count;
	temp.last_flesh = now;

	{
		std::lock_guard<std::mutex> lk(shoting_lock);

		if (waiting_shoting.count(conn.sockfd))
		{
			waiting_shoting[conn.sockfd].last_flesh = now;
			dzlog_info("flesh the connection %d@%s:%d", conn.sockfd, conn.ip.c_str(), conn.port);
		}
		else
		{
			std::lock_guard<std::mutex> lk2(server_lock[temp.type]);

			waiting_shoting[conn.sockfd]	= temp;
			server[temp.type][conn.sockfd]	= temp;

			dzlog_info("add the connection %d@%s:%d", conn.sockfd, conn.ip.c_str(), conn.port);

			switch (temp.type)
			{
			case CLIENT_MAP: 
				break;

			case LOADBANLANCE_MAP:
				// 不向在 keepalive 中得到的 loadbalance 拉取 
				ThreadpoolInstance::GetInstance()->schedule(
					std::bind(&LoadBalanceServer::push_new_loadbalance_to_all, 
					this, temp));

				break;

			case RESOURCE_MAP:
				ThreadpoolInstance::GetInstance()->schedule(
					std::bind(&LoadBalanceServer::pull_media_menu, 
					this, temp));

				break;

			default:
				dzlog_error("an impossible type: %d", temp.type);
				break;
			}

		}
	}

	delete message;
}

void LoadBalanceServer::pull_media_menu(const peer p)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(30));

	PullMediaMenuMessage message;
	InitPullMediaMenuMessage(&message);
	SetTransactionId(&message, rand());

	if (!SendPullMediaMenuMessage(p.conn.sockfd, &message))
	{
		delete_shot(p);
		delete_peer(p);
	}
}

void LoadBalanceServer::push_new_loadbalance_to_all(const peer p)
{
	std::vector<uint32_t> ip;
	std::vector<uint16_t> port;

	ip.push_back(inet_addr(p.conn.ip.c_str()));
	port.push_back(p.conn.port);

	PushLoadBalanceMessage message;
	InitPushLoadBalanceMessage(&message);
	SetOtherLoadBalanceData(&message, ip, port);
	SetTransactionId(&message, rand());

	{
		std::lock_guard<std::mutex> lk(server_lock[CLIENT_MAP]);

		for (auto i = server[CLIENT_MAP].begin(); i != server[CLIENT_MAP].end(); ++i)
		{
			if (!SendPushLoadBalanceMessage(i->first, &message))
			{
				delete_shot(i->second);
				delete_peer(i->second);
			}
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(30));

	{
		std::lock_guard<std::mutex> lk(server_lock[RESOURCE_MAP]);

		for (auto i = server[RESOURCE_MAP].begin(); i != server[RESOURCE_MAP].end(); ++i)
		{
			if (!SendPushLoadBalanceMessage(i->first, &message))
			{
				delete_shot(i->second);
				delete_peer(i->second);
			}
		}
	}

	ClearPushLoadBalanceMessage(&message);
}

LoadBalanceServer* const LoadBalanceServer::GetInstance()
{
	static LoadBalanceServer instance;
	return &instance;
}

LoadBalanceServer::LoadBalanceServer()
{
	status		= false;
	_port		= 0;
	load_index	= 0;

	load[0] = load[1] = 0;

	struct ifaddrs *ifaddr;
	int family;

	if (getifaddrs(&ifaddr) == -1) 
	{
		dzlog_error("getifaddrs failed");
		std::exit(0);
	}

	for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6) 
		{
			uint32_t self = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr;
			self_ip.push_back(self);
		} 
	}

	freeifaddrs(ifaddr);
}

LoadBalanceServer::~LoadBalanceServer()
{
	status = false;
	NetMessageListener::GetInstance()->stop();
	std::this_thread::sleep_for(std::chrono::seconds(5));
}

void LoadBalanceServer::listening(uint32_t sid, uint16_t port)
{
	status	= true;
	_port	= port;
	_sid	= sid;

	NetMessageListener::GetInstance()->SetCallback(&NetMessageListenerCallBack);
	NetMessageListener::GetInstance()->listening(port);

	ThreadpoolInstance::GetInstance()->schedule(std::bind(&LoadBalanceServer::shoting_dead, this));
}

void LoadBalanceServer::delete_peer(peer p)
{
	dzlog_info("type: %d - %d@%s:%d timeout", p.type, p.conn.sockfd, p.conn.ip.c_str(), p.conn.port);
	
	if (p.type == -1)
	{
		if (server[CLIENT_MAP].count(p.conn.sockfd))
		{
			std::lock_guard<std::mutex> lk2(server_lock[CLIENT_MAP]);
			server[CLIENT_MAP].erase(p.conn.sockfd);
			return;
		}

		if (server[RESOURCE_MAP].count(p.conn.sockfd))
		{
			std::lock_guard<std::mutex> lk2(server_lock[RESOURCE_MAP]);
			server[RESOURCE_MAP].erase(p.conn.sockfd);
			return;
		}
	}

	if (!server[p.type].count(p.conn.sockfd)) return;

	std::lock_guard<std::mutex> lk2(server_lock[p.type]);
	server[p.type].erase(p.conn.sockfd);
}

void LoadBalanceServer::delete_shot(peer p)
{
	dzlog_info("type: %d - %d@%s:%d err", p.type, p.conn.sockfd, p.conn.ip.c_str(), p.conn.port);

	if (!waiting_shoting.count(p.conn.sockfd)) return;

	std::lock_guard<std::mutex> lk(shoting_lock);
	waiting_shoting.erase(p.conn.sockfd);
}

void LoadBalanceServer::shoting_dead()
{
	while (status)
	{
		KeepAliveMessage keepalive;
		SetKeepAliveData(&keepalive, _sid, load[!load_index]);
		SetTransactionId(&keepalive, rand());

		load[load_index] = 0;
		load_index = !load_index;

		dzlog_info("start shoting dead servers");
		auto now = std::chrono::steady_clock::now();

		{
			std::lock_guard<std::mutex> lk(shoting_lock);
			for (auto i = waiting_shoting.begin(); i != waiting_shoting.end(); ++i)
			{
				// 这里不进行 socket 的 close
				peer temp = i->second;
				connection conn = temp.conn;

				if (now - temp.last_flesh > std::chrono::seconds(120))
				{
					delete_peer(temp);
					waiting_shoting.erase(i);
				}
				else if (!SendKeepAliveMessage(conn.sockfd, &keepalive))
				{
					delete_peer(temp);
					waiting_shoting.erase(i);
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
}

void LoadBalanceServer::shoting_dead_media_chain()
{
	while (status)
	{
		auto now = std::chrono::steady_clock::now();

		{
			std::lock_guard<std::mutex> lk(media_chain_lock);

			for (auto i = media_chain_map.begin(); i != media_chain_map.end(); ++i)
			{
				media_chain temp = i->second;

				if (temp.is_stream) continue;

				if (now - temp.last_flesh > std::chrono::seconds(10))
				{
					dzlog_error("media chain %s:%d -> %s:%d timeout, ssrc: %d", temp.client.ip.c_str(), temp.client.port, temp.resource.ip.c_str(), temp.resource.port, temp.ssrc);

					media_chain_map.erase(i);	
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(8));
	}
}

void LoadBalanceServer::stop()
{
	NetMessageListener::GetInstance()->stop();
	status = 0;
}

void LoadBalanceServer::inc_load()
{
	++load[load_index];
}
