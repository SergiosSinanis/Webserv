#ifndef CGI_DATA_HPP
#define CGI_DATA_HPP

#include <vector>
#include <string>
#include "ConfigFile.hpp"
#include "Session.hpp"

typedef struct s_CGI_Data
{
	int _fd[2];
	int _status;
	std::vector<char> _buffer;
	pid_t _pid;
	s_CGI_Data() : _status(-1), _pid(-1)
	{
		_fd[0] = -1;
		_fd[1] = -1;
	}

} CGI_Data ;

std::vector<char>	parse_cgi_data(std::vector<char> _buffer);
void				checkCGIExec(HTTPRequest const &_request, ConfigFile const &_config, std::string fullScriptPath);
std::string			getInterpreter(HTTPRequest const &_request, ConfigFile const &_config, std::string fullScriptPath);
void				runCGI(HTTPRequest const &_request, ConfigFile const &_config, CGI_Data &_cgi, std::pair <Session *, bool> _sessionInfos);
bool				checkCGIExitStatus(pid_t _pid);

#endif
