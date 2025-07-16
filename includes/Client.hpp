#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <vector>
#include <ctime>
#include <utility>

#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include "ConfigFile.hpp"
#include "Server.hpp"
#include "CGI_Data.hpp"

typedef enum ESTATE
{
	READING_FROM_SERVER,
	WRITING_TO_SERVER,
	PROCESSING_CGI
} STATE;

class Client
{

	private:

	int _fd;
	HTTPResponse _response;
	HTTPRequest _request;
	STATE		_state;
	CGI_Data	_cgi;
	clock_t		_start_time;
	std::pair <Session *, bool> _sessionInfos;
	
	bool session_timeout_check(SessionManager &sessions);
	bool is_return_response(const ConfigFile &config);
	std::pair<Session *, bool> process_session_id(HTTPRequest &request, SessionManager &sessions);


	public:
	Client();
	Client(int _fd);
	Client(const Client &other);
	Client &operator=(const Client &other);
	~Client();

	HTTPRequest		&getRequest(void);
	int				getFd(void);
	CGI_Data		getCGI(void);
	STATE			getState(void);
	void			setSession(Session* s);
	Session*		getSession() const;

	bool			readData();
	bool			writeData(ConfigFile const &_config, FDInfo &_info, SessionManager &sessions);
	// bool		writeData(ConfigFile const &_config, FDInfo &_info);
	bool			sendDataToCGI(FDInfo &_info);
	bool			recvDataFromCGI(ConfigFile const &_config, FDInfo &_info);
	void			checkTimeout(ConfigFile const &_config);
	HTTPResponse	create_response(const ConfigFile &config, SessionManager &sessions);
};

#endif
