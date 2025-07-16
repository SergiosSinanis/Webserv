#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include "ConfigFile.hpp"

# define CHUNK_SIZE 50000

typedef struct s_FDInfo
{
	int max_fd;

	fd_set afds;
	fd_set rfds;
	fd_set wfds;

	s_FDInfo() : max_fd(0)
	{
		FD_ZERO(&afds);
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
	}
}	FDInfo;

class Server;

class ServerManager
{
private:
	std::vector<Server> _servers;
	FDInfo _fdInfo;

	bool	setFDInfo(void);
public:
	ServerManager();
	ServerManager(const ServerManager& other);
	ServerManager& operator=(const ServerManager& other);
	~ServerManager();

	ServerManager(const std::vector<ConfigFile>& configs);
	void runServers();
};

extern bool quit;
void safe_quit(int sig);

#endif
