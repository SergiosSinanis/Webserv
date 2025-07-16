#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <map>
#include <ctime>
#include "HTTPRequest.hpp"
#include "SessionManager.hpp"
#include "SessionException.hpp"

class Session
{
	private:
		std::string _session_id;
		std::string _username;
		std::map<std::string, std::string> _data;
		time_t _last_access;
		bool _is_authenticated;

	public:
		Session();
		Session(const std::string &id);
		Session(const Session& other);
		Session& operator=(const Session& other);
		~Session();

		const std::string& getId() const;
		void setId(const std::string& newId);

		time_t getLastAccessed() const;
		void setLastAccessed(time_t t);

		void setData(const std::string& key, const std::string& value);
		std::string getData(const std::string& key) const;

		bool isAuthenticated(void) const;

		bool hasData(const std::string& key) const;
		void removeData(const std::string& key);
		const std::map<std::string, std::string>& getAllData() const;

		void logout(void);

		bool empty() const;
};

#endif