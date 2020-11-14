#include "server.h"

server::server()
{
	load[0] = load[1] = load_index = _sid = 0;
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

int server::deal_message(const connection, std::shared_ptr<loadbalance_respond_media_pull_message> mess)
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

int server::deal_message(const connection, std::shared_ptr<respond_media_pull_message> mess)
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
