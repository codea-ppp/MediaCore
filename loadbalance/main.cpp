#include <thread>
#include <chrono>
#include <stdio.h>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <json/json.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "connection.h"
#include "loadbalance.h"

void print_help()
{
	printf("flags:\n\tlog config file path: --log-config\n\tserver config file path: --server-config\n");
}

bool analysis_args(int argc, char* argv[], std::string& log_config_path, std::string& server_config_path)
{
	for (int i = 1; i < argc; i += 2)
	{
		if (0 == strcmp(argv[i], "--log-config"))
			log_config_path = argv[i + 1];
		else if (0 == strcmp(argv[i], "--server-config"))	
			server_config_path = argv[i + 1];
		else
		{
			print_help();
			return false;
		}
	}

	return true;
}

bool analysis_json(const std::string& config_path, std::map<uint32_t, std::pair<uint32_t, uint16_t>>& lb, uint32_t& sid, uint16_t& port)
{
	std::ifstream config_file_stream(config_path, std::ifstream::binary);
	std::string errors;

	Json::Value json_root;
	Json::CharReaderBuilder file_reader;

	if (!Json::parseFromStream(file_reader, config_file_stream, &json_root, &errors))
	{
		dzlog_error("failed to analysis %s: %s", config_path.c_str(), errors.c_str());
		return false;
	}

	if (!json_root.isMember("sid") || !json_root["sid"].isInt())
		return false;
	if (!json_root.isMember("port") || !json_root["port"].isInt())
		return false;
	if (!json_root.isMember("other_loadbalance") || !json_root["other_loadbalance"].isArray())
		return false;
	if (!json_root["other_loadbalance"].isValidIndex(0))
		return false;
	if (!json_root["other_loadbalance"][0].isMember("sid") || !json_root["other_loadbalance"][0]["sid"].isInt())
		return false;
	if (!json_root["other_loadbalance"][0].isMember("ip") || !json_root["other_loadbalance"][0]["ip"].isString())
		return false;
	if (!json_root["other_loadbalance"][0].isMember("port") || !json_root["other_loadbalance"][0]["port"].isInt())
		return false;

	dzlog_info("json config file rule correct");

	sid	 = json_root["sid"].asInt();
	port = json_root["port"].asInt();

	uint32_t	other_lb_sid;
	std::string other_lb_ip;
	uint16_t	other_lb_port;

	struct sockaddr_in addr;

	for (unsigned int i = 0; i < json_root["other_loadbalance"].size(); ++i)
	{
		other_lb_sid	= json_root["other_loadbalance"][i]["sid"].asInt();
		other_lb_ip		= json_root["other_loadbalance"][i]["ip"].asString();
		other_lb_port	= htons(json_root["other_loadbalance"][i]["port"].asInt());

		dzlog_info("other loadbalance %s:%d", other_lb_ip.c_str(), other_lb_port);

		if (inet_pton(AF_INET, other_lb_ip.c_str(), &addr.sin_addr) <= 0)
		{
			dzlog_error("loadbalance %s cannot analysis", other_lb_ip.c_str());
			return false;
		}
	
		std::pair<uint32_t, uint16_t> v = std::make_pair(addr.sin_addr.s_addr, other_lb_port);
		lb[other_lb_sid] = v;
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		print_help();
		return 0;
	}

	std::string log_config_path;
	std::string server_config_path;
	if (!analysis_args(argc, argv, log_config_path, server_config_path))
		return 0;

	printf("log config file path:\t%s\nserver config path:\t%s\n\n", log_config_path.c_str(), server_config_path.c_str());

	if (dzlog_init(log_config_path.c_str(), "loadbalance"))
		return 0;

	uint32_t sid;
	uint16_t port;	// 这个 port 不要 hton
	std::map<uint32_t, std::pair<uint32_t, uint16_t>> lb; // 这里的 port 是要 hton 的

	if (!analysis_json(server_config_path, lb, sid, port))
	{
		dzlog_error("server config file analysis failed");
		return 0;
	}

	for (auto i = lb.begin(); i != lb.end(); ++i)
	{
		connection fake_conn(-1, i->second.first, i->second.second);
		loadbalance::get_instance()->set_loadbalance_map(i->first, fake_conn, false);
	}

	loadbalance::get_instance()->listening(port, sid);

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	return 0;
}
