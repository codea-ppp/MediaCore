#include "media_chain.h"

media_chain::media_chain(const uint32_t ssrc, std::shared_ptr<peer> client, std::shared_ptr<peer> resource, const std::string video)
	: video_2_play(video), _ssrc(ssrc)
{
	_last_time_refresh	= std::chrono::steady_clock::now();
	_server_send_port	= _client_recv_port = 0;

	_resource	= resource;
	_client		= client;
}

void media_chain::fresh()
{
	_last_time_refresh = std::chrono::steady_clock::now();
}

bool media_chain::is_expires()
{
	if (_server_send_port != 0)
		return false;

	return (std::chrono::steady_clock::now() - _last_time_refresh) > std::chrono::seconds(180);
}

void media_chain::set_server_send_port(const uint32_t send_port)
{
	_server_send_port = send_port;
}

void media_chain::set_client_recv_port(const uint32_t recv_port)
{
	_client_recv_port = recv_port;
}

const uint32_t media_chain::get_server_send_port()
{
	return _server_send_port;
}

const uint32_t media_chain::get_client_recv_port()
{
	return _client_recv_port;
}

const uint32_t media_chain::get_ssrc()
{
	return _ssrc;
}

const std::shared_ptr<peer> media_chain::get_resource()
{
	return _resource;
}

const std::shared_ptr<peer> media_chain::get_client()
{
	return _client;
}
