#ifndef SERVER_HPP
# define SERVER_HPP

# include "Colors.hpp"
# include "ConfigFile.hpp"
# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"
# include "Methods.hpp"
# include "ServerManager.hpp"
# include "SessionManager.hpp"

# include <vector>
# include <csignal>
# include <cstring>
# include <cerrno>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <errno.h>
# include <netdb.h>
# include <unistd.h>
# include <iostream>
# include <fstream>
# include <sstream>
# include <cstdlib>
# include <string.h>

typedef struct s_FDInfo FDInfo;
class Client;

class Server
{
	public:
	Server();
	Server(const ConfigFile& config);
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();

	bool setupSocket(FDInfo &_info);
	void closeSocket();
	void run(FDInfo &_info);
	std::map<int, Client> &getClients(void);
	const ConfigFile &getConfig();
	int getSocket();

	private:
		int _socket_fd;
		ConfigFile _config;
		std::map<int, Client> _clients;
		void acceptNewConnection(FDInfo &_info);
		void disconnectClient(Client &client, FDInfo &_info);
		SessionManager _sessions;
};

#endif
