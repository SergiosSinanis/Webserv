#include "Server.hpp"
#include "HTTPException.hpp"
#include "HTTPResponse.hpp"
#include "Client.hpp"

Server::Server() : _config()
{}

Server::Server(const ConfigFile &config) : _config(config)
{}

Server::Server(const Server &other)
{
	*this = other;
}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		_config = other._config;
		_clients = other._clients;
	}
	return (*this);
}

Server::~Server()
{}

void Server::closeSocket()
{
	if (_socket_fd >= 0)
	{
		close(_socket_fd);
		_socket_fd = -1;
	}
}

bool Server::setupSocket(FDInfo &_info)
{
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(_config.port);

	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (_socket_fd < 0)
	{
		std::cout << RED << "socket error: " << strerror(errno) << RESET << std::endl;
		return (false);
	}

	std::cout << GREEN << "socket is " << _socket_fd << RESET << std::endl;
	// pour liberer le port rapidement
	int yes = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(_socket_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		std::cout << RED << "bind error: " << strerror(errno) << RESET << std::endl;
		close(_socket_fd);
		return (false);
	}

	if (listen(_socket_fd, 2) < 0)
	{
		std::cout << RED << "listen error: " << strerror(errno) << RESET << std::endl;
		close(_socket_fd);
		return (false);
	}

	FD_SET(_socket_fd, &_info.afds);
	std::cout << GREEN << "Server listening on localhost:" << _config.port << RESET << std::endl;

	return (true);
}

void Server::run(FDInfo &_info)
{
		if (FD_ISSET(_socket_fd, &_info.rfds))
			acceptNewConnection(_info);

		for (std::map<int, Client>::iterator it = _clients.begin(), next_it = it; it != _clients.end(); it = next_it)
		{
			++next_it;
			int fd = it->first;
			Client &client = it->second;

			client.checkTimeout(_config);
			// std::cout << (client.getState() == WRITING_TO_SERVER) << std::endl;
			if (FD_ISSET(fd, &_info.rfds) && client.getState() == WRITING_TO_SERVER)
			{
				if (!client.writeData(_config, _info, _sessions))
				{
					FD_CLR(client.getFd(), &_info.rfds);
					disconnectClient(client, _info);
				}
				// envoyer une erreur 400 si writedata = false
			}
			else if (client.getState() == PROCESSING_CGI)
			{
				if (client.getCGI()._fd[0] != -1 && FD_ISSET(client.getCGI()._fd[0], &_info.wfds))
				{
					if (!client.sendDataToCGI(_info))
					{
						disconnectClient(client, _info);
					}
				}
				else if (client.getCGI()._fd[1] != -1 && FD_ISSET(client.getCGI()._fd[1], &_info.rfds))
				{
					if (!client.recvDataFromCGI(_config, _info))
					{
						disconnectClient(client, _info);
					}
				}
			}
			else if (FD_ISSET(fd, &_info.wfds) && client.getState() == READING_FROM_SERVER)
			{
				if (client.readData())
				{
					FD_CLR(client.getFd(), &_info.wfds);
					disconnectClient(client,_info);
				}
			}
		}
}

std::map<int, Client>& Server::getClients(void)
{
	return (_clients);
}

const ConfigFile &Server::getConfig()
{
	return (_config);
}

int Server::getSocket()
{
	return (_socket_fd);
}

void Server::acceptNewConnection(FDInfo &_info)
{
	std::cout << GREEN << "New Connection : " << RESET;
	struct sockaddr_storage client_addr;
	socklen_t len = sizeof(client_addr);

	int client_fd = accept(_socket_fd, (struct sockaddr *)&client_addr, &len);

	if (client_fd >= FD_SETSIZE)
	{
		std::cout << RED << "Too many clients (fd exceeds FD_SETSIZE)" << RESET << std::endl;
		close(client_fd);
		return;
	}

	if (client_fd < 0)
	{
		std::cout << RED << "accept error: " << strerror(errno) << RESET << std::endl;
		return;
	}
	FD_SET(client_fd, &_info.afds);
	std::cout << BRIGHT_GREEN << client_fd << RESET << std::endl;

	_clients.insert(std::make_pair(client_fd, Client(client_fd)));
}

void Server::disconnectClient(Client &_client, FDInfo &_info)
{
	std::cout << RED << "Disconnect Client ["
		<< _client.getFd() << "]"
		<< RESET << std::endl;

	if (_client.getCGI()._fd[0] != -1)
	{
		FD_CLR(_client.getCGI()._fd[0], &_info.wfds);
		close(_client.getCGI()._fd[0]);
	}

	if (_client.getCGI()._fd[1] != -1)
	{
		FD_CLR(_client.getCGI()._fd[1], &_info.rfds);
		close(_client.getCGI()._fd[1]);
	}

	FD_CLR(_client.getFd(), &_info.rfds);
	FD_CLR(_client.getFd(), &_info.wfds);
	FD_CLR(_client.getFd(), &_info.afds);

	close(_client.getFd());
	_clients.erase(_client.getFd());
}
