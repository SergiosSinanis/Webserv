#include "Colors.hpp"
#include "ServerManager.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	std::vector<ConfigFile> configs;

	if (!parse_config_file(argv[1], configs))
	{
		std::cerr << "Config file error" << std::endl;
		return 1;
	}

	ServerManager manager(configs);
	manager.runServers();

	return 0;
}
