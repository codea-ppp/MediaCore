#ifndef LOAD_BALANCE_SERVER_H_
#define LOAD_BALANCE_SERVER_H_

#include <map>
#include <list>
#include <mutex>
#include <vector>
#include <stdint.h>
#include "MessageFormat.h"
#include "ThreadpoolInstance.h"
#include "NetMessageListener.h"

#define CLIENT_MAP			0
#define LOADBANLANCE_MAP	1
#define RESOURCE_MAP		2

using namespace MediaCoreMessageFormat;
void NetMessageListenerCallBack(const connection& conn, void* message);

class LoadBalanceServer
{
public:
	static LoadBalanceServer* const GetInstance();

	void listening(uint32_t sid, uint16_t port);
	void stop();
	void GetCallBack(const connection conn, void* message);

private:
	void deal_keepalive(const connection, KeepAliveMessage*);
	void deal_pullotherloadbalance(const connection, PullOtherLoadBalanceMessage*);
	void deal_respondpullotherloadbalance(const connection, RespondLoadBalancePullMessage*);
	void deal_pullmediamenu(const connection, PullMediaMenuMessage*);
	void deal_respondpullmediamenu(const connection, RespondMediaMenuPullMessage*);
	void deal_pushmediamenu(const connection, PushMediaMenuMessage*);
	void deal_clientpullstream(const connection, ClientPullMediaStreamMessage*);
	void deal_resourcerespondpullstream(const connection, ResourceServerRespondMediaPullMessage*);
	void deal_stopstream(const connection, StopStreamMessage* message);
	void deal_resourcereport(const connection, ResourceServerReportMessage* message);

private:
	void push_new_loadbalance_to_all(const peer);
	void pull_media_menu(const peer);
	void send_first_keepalive(uint32_t ip, uint16_t port);
	void send_mediamenu(const connection);
	void send_report(const connection, uint32_t tid, int err);
	void send_loadbalancerespondpullstream(const connection, uint32_t tid, uint32_t ssrc, uint16_t send_port);
	void send_stopstream(const connection, uint32_t tid, uint32_t ssrc);

private:
	void delete_peer(peer p);	// 这个删 server map
	void delete_shot(peer p);	// 这个删 shoting map
	void inc_load();

	void shoting_dead(); // 如果调用 delete_shot 就会死锁
	void shoting_dead_media_chain();

private:
	LoadBalanceServer();
	~LoadBalanceServer();
	LoadBalanceServer(const LoadBalanceServer&)		= delete;
	LoadBalanceServer(const LoadBalanceServer&&)	= delete;
	const LoadBalanceServer& operator=(const LoadBalanceServer&)	= delete;
	const LoadBalanceServer& operator=(const LoadBalanceServer&&)	= delete;

private:	
	// int for sockfd
	std::map<std::string, std::map<int, connection>> video_name_map;
	std::mutex video_names_lock;

	// tid for key
	std::map<int, media_chain> media_chain_map;
	std::mutex media_chain_lock;

	// sockfd for key
	std::map<int, peer> waiting_shoting;
	std::mutex shoting_lock;

	// sockfd for key
	std::map<int, peer> server[3];
	std::mutex server_lock[3];

	bool		status;
	uint32_t	_sid;
	uint16_t	_port;

	std::vector<uint32_t> self_ip;

	int load[2];
	int load_index;
};

#endif 
