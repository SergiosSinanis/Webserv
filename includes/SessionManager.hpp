#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <string>
#include <map>
#include <ctime>
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
#include <algorithm>

# include "HTTPRequest.hpp"
# include "Session.hpp"
# include "HTTPException.hpp"

# define SESSION_TIMEOUT "1800"

class Session;

class SessionManager
{
	private:
		std::map<std::string, Session> _sessions;

		std::string extractSessionId(HTTPRequest& request);
		std::string generateSessionId();

	public:
		SessionManager();
		SessionManager(const SessionManager& other);
		SessionManager& operator=(const SessionManager& other);
		~SessionManager();

		Session* getSession(const std::string &id);
		Session* createSession(const HTTPRequest& req);
		void removeSession(const std::string &id);

		void print_session_id();
};


#endif
