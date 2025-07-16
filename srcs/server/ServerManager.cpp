#include <algorithm>
#include "ServerManager.hpp"
#include "Client.hpp"
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/select.h>

bool quit = false;

bool ServerManager::setFDInfo(void)
{
	_fdInfo.rfds = _fdInfo.wfds = _fdInfo.afds;

	int max_fd = 0;
	for (std::vector<Server>::iterator servIt = _servers.begin(); servIt != _servers.end(); ++servIt)
	{
		max_fd = std::max(max_fd, servIt->getSocket());
		std::map<int, Client>& clients = servIt->getClients();
		for (std::map<int, Client>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			int fd = clientIt->second.getFd();
			if (fd >= 0)
				max_fd = std::max(max_fd, fd);

			int cgi_in = clientIt->second.getCGI()._fd[0];
			int cgi_out = clientIt->second.getCGI()._fd[1];
			if (cgi_in >= 0)
				max_fd = std::max(max_fd, cgi_in);
			if (cgi_out >= 0)
				max_fd = std::max(max_fd, cgi_out);
		}
	}
	++max_fd;
	_fdInfo.max_fd = max_fd;

	int sel = select(max_fd, &_fdInfo.rfds, &_fdInfo.wfds, NULL, NULL);
	if (sel < 0)
	{
		if (!quit)
			std::cout << RED << "select error: " << strerror(errno) << RESET << std::endl;
		return (false);
	}
	return (true);
}

ServerManager::ServerManager()
{}

ServerManager::ServerManager(const ServerManager& other)
	: _servers(other._servers)
{}

ServerManager& ServerManager::operator=(const ServerManager& other)
{
	if (this != &other)
	{
		_servers = other._servers;
	}
	return (*this);
}

ServerManager::~ServerManager()
{
	std::cout << CYAN << "Max FD : " << _fdInfo.max_fd << RESET << std::endl;

	for (size_t i = 0; i < _servers.size(); ++i)
		_servers[i].closeSocket();

	for (int i = 0; i <= _fdInfo.max_fd; ++i)
	{
		if (FD_ISSET(i, &_fdInfo.afds))
		{
			FD_CLR(i, &_fdInfo.afds);
			close(i);
		}
	}
}

void safe_quit(int sig)
{
	(void) sig;
	quit = true;
}

ServerManager::ServerManager(const std::vector<ConfigFile>& configs)
{
	for (std::vector<ConfigFile>::const_iterator it = configs.begin(); it != configs.end(); ++it)
	{
		_servers.push_back(Server(*it));
	}
}

void ServerManager::runServers(void)
{
	signal(SIGINT, safe_quit);
	signal(SIGTERM, safe_quit);
	signal(SIGPIPE, SIG_IGN);

	int i = 0;
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end();)
	{
		std::cout << GREEN
			<< "=== Server [" << i << "] on port " << it->getConfig().port
			<< " : ===" << RESET << std::endl;

		if (!it->setupSocket(_fdInfo))
			it = _servers.erase(it);
		else
			++it;
		i++;
	}

	while (!quit)
	{
		if (!setFDInfo())
			continue ;
		for (size_t i = 0; i < _servers.size(); ++i)
		{
			_servers[i].run(_fdInfo);
		}
	}
}
