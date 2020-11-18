#ifndef CLIENT_ABILITY_H_
#define CLIENT_ABILITY_H_

#include "peer.h"
#include "server.h"

class client_ability : virtual public server
{
public:
	int fresh_or_insert_client(const connection conn, const uint32_t sid, const uint16_t listening_port, const uint32_t load);
	int fresh_client(int sock);
	void shoting_client();

	void set_client_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect);
	void set_new_client_map(uint32_t sid, uint32_t ip, uint16_t port, bool is_connect);
	void erase_client_map(uint32_t ip, uint16_t port);
	void rolling_client_map();

	int client_sockfd_2_sid(int);
	int insert_client_sock_sid(int sock, uint32_t sid);
	int remove_client_sock_sid(int sock);

protected:
	// sid for key
	std::map<uint32_t, std::shared_ptr<peer>> _client_map;
	std::mutex _client_map_lock;

private:
	// map<pair<ip, port>, pair<is_connect, sid>>
	std::map<std::pair<uint32_t, uint16_t>, std::pair<bool, uint32_t>> _client_2_is_connect_map;
	std::vector<uint32_t> _client_sids;
	std::mutex _client_2_is_connect_map_lock;

	std::map<int, uint32_t> _sock_2_sid;
	std::mutex _sock_2_sid_lock;
	
};

#endif 
