#ifndef LOADBALANCE_ABILITY_H_
#define LOADBALANCE_ABILITY_H_

#include "peer.h"
#include "server.h"

class loadbalance_ability : virtual public server 
{
public:
	int	fresh_or_insert_loadbalance(const connection conn, const uint32_t sid, const uint16_t listening_port, const uint32_t load);
	int	fresh_loadbalance(int sock);
	void shoting_loadbalance();

	void set_loadbalance_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect);
	void set_loadbalance_map(std::vector<uint32_t> sid, std::vector<uint32_t> ip, std::vector<uint16_t> port, bool is_connect);
	void set_new_loadbalance_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect);
	void set_new_loadbalance_map(std::vector<uint32_t> sid, std::vector<uint32_t> ip, std::vector<uint16_t> port, bool is_connect);
	void erase_loadbalance_map(uint32_t ip, uint16_t port);
	void rolling_loadbalance_map();

	int loadbalance_sockfd_2_sid(int);
	int loadbalance_sockfd_2_sid(std::vector<uint32_t>&, const std::vector<uint32_t>&);
	int insert_loadbalance_sock_sid(int sock, uint32_t sid);
	int remove_loadbalance_sock_sid(int sock);

protected:
	// sid for key
	std::map<uint32_t, std::shared_ptr<peer>> _loadbalance_map;
	std::mutex _loadbalance_map_lock;

private:
	// map<pair<ip, port>, pair<is_connect, sid>>
	std::map<std::pair<uint32_t, uint16_t>, std::pair<bool, uint32_t>> _loadbalance_2_is_connect_map;
	std::vector<uint32_t> _loadbalance_sids;
	std::mutex _loadbalance_2_is_connect_map_lock;

	std::map<int, uint32_t> _sock_2_sid;
	std::mutex _sock_2_sid_lock;
};

#endif 
