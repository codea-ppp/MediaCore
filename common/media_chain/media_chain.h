#ifndef MEDIA_CHAIN_H_
#define MEDIA_CHAIN_H_

#include "peer.h"
#include <chrono>
#include <stdint.h>

class media_chain
{
public:
	media_chain(const uint32_t ssrc, std::shared_ptr<peer> client, std::shared_ptr<peer> resource, const std::string video);

	void fresh();
	bool is_expires();

	void set_server_send_port(const uint32_t send_port);
	void set_client_recv_port(const uint32_t recv_port);

	const uint32_t get_server_send_port();
	const uint32_t get_client_recv_port();
	const uint32_t get_ssrc();

	const std::shared_ptr<peer> get_resource();
	const std::shared_ptr<peer> get_client();

private:
	std::chrono::time_point<std::chrono::steady_clock> _last_time_refresh;

	const std::string video_2_play;

	uint32_t _server_send_port;
	uint32_t _client_recv_port;

	const uint32_t _ssrc;

	std::shared_ptr<peer> _client;
	std::shared_ptr<peer> _resource;
};

#endif 
