#include "CGI_Data.hpp"
#include "Methods.hpp"
#include "HTTPResponse.hpp"
#include <sys/wait.h>

static std::map<std::string, std::string> parse_cgi_headers(std::vector<char> _buffer, size_t _pos)
{
	std::map<std::string, std::string> headerFields;

	std::string string_buffer(_buffer.begin(), _buffer.begin() + _pos);
	std::istringstream header_stream(string_buffer);
	std::string line;

	while (std::getline(header_stream, line))
	{
		if (line.empty() || line == "\r")
			continue;

		std::string::size_type colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);
			if (!value.empty() && value[value.size() - 1] == '\r')
				value.erase(value.size() - 1);

			std::transform(key.begin(), key.end(), key.begin(), ::tolower);

			headerFields[key] = value;
		}
		else
			throw (HTTPException("Invalid HeaderField", 500));
	}

	return (headerFields);
}

std::vector<char> parse_cgi_data(std::vector<char> _buffer)
{
	if (_buffer.size() == 0)
		throw (HTTPException("Empty Response", 500));

	std::string str_buffer(_buffer.begin(), _buffer.end());
	size_t pos = str_buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		throw (HTTPException("Malformed Response", 500));

	//PARSE HEADERS
	int status_code;
	std::map <std::string, std::string> cgiHeaders = parse_cgi_headers(_buffer, pos);
	if (cgiHeaders.size() == 0)
		throw (HTTPException("No HeaderFields", 500));

	if (cgiHeaders.count("status"))
		status_code = std::atoi(cgiHeaders["status"].c_str());
	else
		status_code = 200;

	//PARSE BODY
	std::vector<char> cgiBody;
	if (pos + 4 < _buffer.size())
	{
		cgiBody.insert(cgiBody.end(), _buffer.begin() + pos + 4 , _buffer.end());
	}
	if (cgiBody.size() > 0)
		cgiHeaders["Content-Length"] = int_to_str(cgiBody.size());

	//STATUS LINE
	std::string response_str;
	response_str += "HTTP/1.1 " + int_to_str(status_code) + " " + get_status_message(status_code) + "\r\n";

	//HEADERS
	std::map<std::string, std::string>::const_iterator it;
	std::map<std::string, std::string>::const_iterator ite = cgiHeaders.end();
	for (it = cgiHeaders.begin(); it != ite; it++)
		response_str += it->first + ": " + it->second + "\r\n";

	response_str += "\r\n";

	std::vector<char> response_vect;
	for (size_t i = 0; i < response_str.length(); i++)
		response_vect.push_back(response_str[i]);

	if (cgiBody.size() > 0)
		response_vect.insert(response_vect.end(), cgiBody.begin(), cgiBody.end());

	return (response_vect);
}

bool checkCGIExitStatus(pid_t _pid)
{
	int status = 0;
	pid_t ret = waitpid(_pid, &status, WNOHANG);

	if (ret == 0)
	{
		std::cout << "CGI child still running after EOF on stdout pipe." << std::endl;
	}
	else if (ret == _pid)
	{
		if (WIFEXITED(status))
		{
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0)
			{
				std::cout << RED << "CGI exited with non-zero status: " << exit_code << RESET << std::endl;
				throw HTTPException("CGI Execution Failed", 502);
			}
			return (true);
		}
		else if (WIFSIGNALED(status))
		{
			std::cout << RED << "CGI terminated by signal: " << WTERMSIG(status) << RESET << std::endl;
			throw HTTPException("CGI Crashed", 502);
		}
	}
	else if (ret == -1)
	{
		std::cerr << RED << "waitpid() failed: " << strerror(errno) << RESET << std::endl;
		throw HTTPException("CGI waitpid() error", 500);
	}
	return (false);
}

void checkCGIExec(HTTPRequest const &_request, ConfigFile const &_config, std::string fullScriptPath)
{
	//CHECK METHOD
	std::string uri_loc;
	uri_loc = find_location_prefixe(_request.target, _config);

	if (!is_method_allowed(_request.method, uri_loc, _config))
		throw HTTPException("Method Not Allowed", 405);

	//CHECK FILE EXIST
	struct stat buffer;

	if (stat(fullScriptPath.c_str(), &buffer) != 0)
		throw HTTPException("Not Found", 404);
	if (!S_ISREG(buffer.st_mode))
		throw HTTPException("Not A File", 404);
	if (access(fullScriptPath.c_str(), X_OK) != 0)
		throw HTTPException("Execution Not Allowed", 403);
}

std::string getInterpreter(HTTPRequest const &_request, ConfigFile const &_config, std::string fullScriptPath)
{
	std::string extension;
	size_t dotPos = fullScriptPath.rfind('.');

	if (dotPos != std::string::npos)
	{
		extension = fullScriptPath.substr(dotPos);
		std::string uri_loc = find_location_prefixe(_request.target, _config);
		if (uri_loc.length() > 1 && uri_loc[uri_loc.length() - 1] == '/')
			uri_loc = uri_loc.substr(0, uri_loc.length() - 1);

		std::vector<location>::const_iterator it;
		std::vector<location>::const_iterator ite = _config.locations.end();
		for (it = _config.locations.begin(); it != ite; it++)
		{
			if (uri_loc == it->locationDir)
			{
				if((*it).cgiExtensions.count(extension.c_str()) != 0)
					return ((*it).cgiExtensions.at(extension));
				else
				{
					std::string error = "Unsupported script extension: " + extension;
					throw HTTPException(error.c_str(), 500);
				}
			}
		}
	}
	return ("");
}

void runCGI(HTTPRequest const &_request, ConfigFile const &_config, CGI_Data &_cgi, std::pair <Session *, bool> _sessionInfos)
{
	std::string fullScriptPath = _config.root + _request.target; // e.g. "./cgi-bin/script.php"

	checkCGIExec(_request, _config, fullScriptPath);
	std::string interpreter = getInterpreter(_request, _config, fullScriptPath);

	int in_pipe[2] = {-1, -1};
	int out_pipe[2];

	if (pipe(out_pipe) == -1)
		throw HTTPException("pipe() failed (stdout)", 500);

	bool hasBody = !_request.body.empty();
	if (hasBody && pipe(in_pipe) == -1)
		throw HTTPException("pipe() failed (stdin)", 500);

	_cgi._pid = fork();
	if (_cgi._pid < 0)
	{
		std::string error = "fork() failed: " + std::string(strerror(errno));
		throw HTTPException(error.c_str(), 500);
	}

	if (_cgi._pid == 0)
	{
		// stdin (if we have a body)
		if (hasBody)
		{
			dup2(in_pipe[0], STDIN_FILENO);
			close(in_pipe[1]); // close write end in child
		}
		else
		{
			// if no body, optionally connect to /dev/null
			int null_fd = open("/dev/null", O_RDONLY);
			dup2(null_fd, STDIN_FILENO);
			close(null_fd);
		}

		// stdout
		dup2(out_pipe[1], STDOUT_FILENO);
		close(out_pipe[0]);

		// Set CGI env
		setenv("REQUEST_METHOD", method_to_str(_request.method).c_str(), 1);
		setenv("SCRIPT_FILENAME", fullScriptPath.c_str(), 1);
		setenv("REDIRECT_STATUS", "200", 1);
		setenv("SERVER_NAME", _config.host.c_str(), 1);
		setenv("SESSION_ID", _sessionInfos.first->getId().c_str(), 1);
		std::ostringstream oss;
		oss << _config.port;
		setenv("SERVER_PORT", oss.str().c_str(), 1);

		if (_request.body.size() > 0)
		{
			std::ostringstream oss;
			oss << _request.body.size();
			setenv("CONTENT_LENGTH", oss.str().c_str(), 1);
		}
		if (_request.query_str.size())
			setenv("QUERY_STRING", _request.query_str.c_str(), 1);
		// Add more like QUERY_STRING, CONTENT_LENGTH, etc. if needed

		// Exec
		std::vector<char*> args;
		args.push_back(const_cast<char*>(interpreter.c_str()));
		args.push_back(const_cast<char*>(fullScriptPath.c_str()));
		args.push_back(NULL);

		execv(interpreter.c_str(), &args[0]);
		exit(1);
	}
	else
	{
		close(out_pipe[1]); // close write end of stdout
		_cgi._fd[1] = out_pipe[0];
		if (hasBody)
		{
			close(in_pipe[0]); // close read end
			_cgi._fd[0] = in_pipe[1]; // we write to CGI here
		}
		else
		{
			_cgi._fd[0] = -1; // no need to write
		}
	}
}
