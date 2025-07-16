#include "Session.hpp"
#include <cstdlib>   // rand
#include <sstream>
#include <ctime>

Session::Session() : _last_access(time(NULL)), _is_authenticated(false) {}

Session::Session(const std::string &id) : _session_id(id), _last_access(time(NULL)), _is_authenticated(false) {}

Session::Session(const Session& other): _session_id(other._session_id), _username(other._username), _data(other._data), _last_access(other._last_access), _is_authenticated(other._is_authenticated) {}

Session& Session::operator=(const Session& other)
{
    if (this != &other)
    {
        _session_id = other._session_id;
		_username = other._username;
        _data = other._data;
        _last_access = other._last_access;
		_is_authenticated = other._is_authenticated;
    }
    return (*this);
}

Session::~Session() {}

const std::string& Session::getId() const
{
    return (_session_id);
}

void Session::setId(const std::string& id)
{
    _session_id = id;
}

time_t Session::getLastAccessed() const
{
    return (_last_access);
}

void Session::setLastAccessed(time_t t)
{
    _last_access = t;
}

void Session::setData(const std::string& key, const std::string& value)
{
    _data[key] = value;
}

std::string Session::getData(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = _data.find(key);
    if (it != _data.end())
        return (it->second);
    return ("");
}

bool Session::hasData(const std::string& key) const
{
    return (_data.find(key) != _data.end());
}

void Session::removeData(const std::string& key)
{
    _data.erase(key);
}

const std::map<std::string, std::string>& Session::getAllData() const
{
    return (_data);
}

bool Session::empty(void) const
{
	return (_session_id.empty());
}

void Session::logout(void)
{
	_is_authenticated = false;
	_username.clear();
	_data.clear();
	_last_access = time(NULL);
	
}

bool Session::isAuthenticated(void) const
{
	return (_is_authenticated);
}
