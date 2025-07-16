#include "SessionManager.hpp"
#include <sstream>
#include <ctime>

SessionManager::SessionManager() {}

SessionManager::SessionManager(const SessionManager& other) : _sessions(other._sessions) {}

SessionManager& SessionManager::operator=(const SessionManager& other)
{
    if (this != &other)
        _sessions = other._sessions;
    return (*this);
}

SessionManager::~SessionManager() {}

std::string SessionManager::extractSessionId(HTTPRequest& request)
{
	if (!request.session_id.empty())
		return (request.session_id);
	return ("");
}

std::string SessionManager::generateSessionId()
{
	std::string new_id = "";

	std::ifstream random("/dev/urandom");
	if (!random)
		throw(HTTPException("session initialisation failed", 500));

	for (int i = 0; i < 10; i++)
	{
		char c = (random.get() % 26) + 97;
		new_id += c;
	}

	if (_sessions.find(new_id) == _sessions.end())
		return (new_id);
	else
		return (generateSessionId());
}

Session* SessionManager::getSession(const std::string &id)
{
	std::map<std::string, Session>::iterator it = _sessions.find(id);
	if (it != _sessions.end())
		return (&it->second);
	return (NULL);
}

Session* SessionManager::createSession(const HTTPRequest& req)
{
	(void)req;
    std::string id = generateSessionId();
    Session new_session(id);

    _sessions[id] = new_session;
    return (&_sessions[id]);
}

void SessionManager::removeSession(const std::string &id)
{
	std::map<std::string, Session>::iterator it = _sessions.find(id);
	if (it != _sessions.end())
	{
		if (it->second.isAuthenticated())
			it->second.logout();
		_sessions.erase(it);
	}
}

void SessionManager::print_session_id(void)
{
	std::cout << "SESSIONS IDS" << std::endl;
	std::map<std::string, Session>::iterator it;
	std::map<std::string, Session>::iterator ite = _sessions.end();
	for (it = _sessions.begin(); it != ite; it++)
		std::cout << it->second.getId() << std::endl;
	std::cout << std::endl;
}