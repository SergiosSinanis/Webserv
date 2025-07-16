#include <sys/wait.h>

#include "Client.hpp"
#include "HTTPException.hpp"
#include "HTTPResponse.hpp"
#include "Server.hpp"
#include "Methods.hpp"

Client::Client()
{
	_fd = -1;
	_request.size = 0;
	_state = WRITING_TO_SERVER;
}

Client::Client(int fd)
{
	_fd = fd;
	_request.size = 0;
	_request.raw.clear();
	_state = WRITING_TO_SERVER;
}

Client::Client(const Client &_other)
{
	_request = _other._request;
	_response = _other._response;
	_fd = _other._fd;
	_cgi = _other._cgi;
	_state = _other._state;
}

Client &Client::operator=(const Client &_other)
{
	if (this != &_other)
	{
		_request = _other._request;
		_response = _other._response;
		_fd = _other._fd;
		_cgi = _other._cgi;
		_state = _other._state;
	}
	return (*this);
}

Client::~Client()
{
}

HTTPRequest &Client::getRequest(void)
{
	return (_request);
}

int Client::getFd(void)
{
	return (_fd);
}

CGI_Data Client::getCGI(void)
{
	return _cgi;
}

STATE Client::getState(void)
{
	return (_state);
}

void Client::setSession(Session *s)
{
	_sessionInfos.first = s;
}
Session *Client::getSession() const
{
	return _sessionInfos.first;
}

// ================================ SOCKET DATA =============================

bool Client::readData()
{
	std::vector<char> buf;
	std::vector<char>::iterator start = _response.full_response.begin() + _response.total_sent;
	size_t buf_size = _response.total_sent + CHUNK_SIZE > _response.full_response.size() ? _response.full_response.size() - _response.total_sent : CHUNK_SIZE;

	if (buf_size == 0)
	{
		std::cout << RED
				  << "No more Data to send to Client ["
				  << _fd << "] : size is " << _response.full_response.size()
				  << " but expected size is " << _response.total_sent
				  << RESET << std::endl;
		return (true);
	}

	buf.insert(buf.end(), start, start + buf_size);
	ssize_t sent = send(_fd, buf.data(), buf_size, 0);

	if (sent < 0)
	{
		std::cout << RED << "Error Sending Data to client [" << _fd << "]" << RESET << std::endl;
		return (true);
	}

	std::cout << MAGENTA << "Sent data (" << sent << ") on _fd " << _fd << " for a total of " << _response.total_sent + sent << RESET << std::endl;
	_response.total_sent += sent;

	if (_response.total_sent >= _response.full_response.size())
	{
		print_response(_response.full_response);
		return (true);
	}
	return (false);
}

bool Client::writeData(ConfigFile const &_config, FDInfo &_info, SessionManager &sessions)
{
	char buf[CHUNK_SIZE];
	ssize_t bytes = recv(_fd, buf, CHUNK_SIZE, 0);

	if (bytes < 0)
	{
		std::cout << RED << "Error Receiving Data from client [" << _fd << "]" << RESET << std::endl;
		return (false);
	}

	if (bytes == 0 && _request.size == 0)
	{
		std::cout << RED << "Client not sending any data [" << _fd << "]" << RESET << std::endl;
		return (false);
	}

	std::cout << BRIGHT_MAGENTA << "CHUNK_SIZE = " << CHUNK_SIZE << RESET << std::endl;
	std::cout << MAGENTA << "Received data (" << bytes << ") on _fd " << _fd << " for a total of " << _request.size + bytes << RESET << std::endl;
	std::vector<char> vec_buffer(buf, buf + bytes);
	_request.raw.insert(_request.raw.end(), vec_buffer.begin(), vec_buffer.end());
	if (_request.size == 0)
		_start_time = clock();
	_request.size += bytes;

	// WAIT FULL HEADER
	if (!isHeaderComplete(_request))
		return (true);
	try
	{
		parse_request(_request, _config);
	}
	catch (const HTTPException &e)
	{
		std::cout << RED
				  << "HTTP Request error : " << e.what()
				  << RESET << std::endl;

		if (e.get_code() != 0)
		{
			Session *null_sess = NULL;
			std::pair<Session *, bool> null_pair = std::make_pair(null_sess, false);
			_response = http_response(e.get_code(), "", _config, _request, null_pair); ///// to check
			_state = READING_FROM_SERVER;
			return (true);
		}
		return (false);
	}

	if (getRawBodySize(_request) > getMaxBodySizeByLocation(_request.target, _config))
	{
		_request.exceedMaxBodySize = true;
		print_request(_request);
		_state = READING_FROM_SERVER;
		_response = create_response(_config, sessions);
		return (true);
	}

	// WAIT FULL BODY
	if (!isBodyComplete(_request))
		return (true);
	if (_request.waitFullBody)
		parse_body(_request);

	checkCGILocation(_request, _config);
	print_request(_request);

	// SESSION MGT
	if (!_sessionInfos.first)
	{
		_sessionInfos = process_session_id(_request, sessions);
	}

	FD_CLR(_fd, &_info.rfds);

	if (_request.useCGI)
	{
		try
		{
			runCGI(_request, _config, _cgi, _sessionInfos);
		}
		catch (const HTTPException &e)
		{
			std::cerr << RED
					  << "CGI Error : " << e.what()
					  << RESET << std::endl;
			_state = READING_FROM_SERVER;
			_response = http_response(e.get_code(), "", _config, _request, _sessionInfos);
			return (true);
		}
		_cgi._buffer.clear();
		if (_cgi._fd[0] != -1)
			FD_SET(_cgi._fd[0], &_info.afds);
		if (_cgi._fd[1] != -1)
			FD_SET(_cgi._fd[1], &_info.afds);
		_state = PROCESSING_CGI;
	}
	else
	{
		_state = READING_FROM_SERVER;
		_response = create_response(_config, sessions);
	}
	return (true);
}

// ================================ RESPONSE & SESS MGT =============================


std::pair<Session *, bool> Client::process_session_id(HTTPRequest &request, SessionManager &sessions)
{
	Session *sess;

	sess = sessions.getSession(request.session_id);
	if (!sess || sess->empty())
	{
		sess = sessions.createSession(request);
		request.session_id = sess->getId();
		return (std::make_pair(sess, true));
	}
	return (std::make_pair(sess, false));
}

bool Client::session_timeout_check(SessionManager &sessions)
{
	if (_sessionInfos.first)
	{
		time_t max_duration = std::atoi(SESSION_TIMEOUT);
		if (_sessionInfos.first->isAuthenticated())
			max_duration *= 3;
		if (time(NULL) - _sessionInfos.first->getLastAccessed() > max_duration)
		{
			sessions.removeSession(_sessionInfos.first->getId());
			return (true);
		}
		else
		{
			_sessionInfos.first->setLastAccessed(time(NULL));
			return (false);
		}
	}
	return (false);
}

bool Client::is_return_response(const ConfigFile &config)
{
	location location = find_location(_request.target, config);
	if (!location.returnRedirection.second.empty())
		return (true);
	return (false);
}

HTTPResponse Client::create_response(const ConfigFile &config, SessionManager &sessions)
{
	HTTPResponse response;

	if (this->session_timeout_check(sessions))
		return (http_response(302, LOGOUT, config, _request, _sessionInfos));

	if (this->is_return_response(config))
		return (http_return_response(_request, config));

	switch (_request.method)
	{
		case GET:
			response = get_request(config, _request, GET, _sessionInfos);
			break;
		case HEAD:
			response = get_request(config, _request, HEAD, _sessionInfos);
			break;
		case PUT:
			response = put_request(config, _request, PUT, _sessionInfos);
			break;
		case DELETE:
			response = delete_request(config, _request, DELETE, _sessionInfos);
			break;
		case POST:
			response = post_request(config, _request, POST, _sessionInfos);
			break;
		case PATCH:
			response = patch_request(config, _request, PATCH, _sessionInfos);
			break;
		default:
			response = http_response(400, _request.target, config, _request, _sessionInfos);
	}

	return (response);
}

// ================================ CGI DATA =============================

bool Client::sendDataToCGI(FDInfo &_info)
{
	std::vector<char> to_send;

	size_t start_pos = _cgi._buffer.size();

	size_t buf_size = _cgi._buffer.size() + CHUNK_SIZE > _request.expectedBodySize ? _request.expectedBodySize - _cgi._buffer.size() : CHUNK_SIZE;

	if (buf_size == 0)
	{
		std::cout << RED
				  << "No more Data to send to CGI ["
				  << _cgi._pid << "] : size is " << _cgi._buffer.size()
				  << " but expected size is " << _request.expectedBodySize
				  << RESET << std::endl;
		return (false);
	}

	to_send.insert(to_send.end(), _request.body.begin() + start_pos, _request.body.begin() + start_pos + buf_size);
	ssize_t sent = write(_cgi._fd[0], to_send.data(), buf_size);
	if (sent < 0)
		return (false);

	std::cout << MAGENTA << "Sent data to CGI (" << sent << ") on _cgi._fd[0] " << _cgi._fd[0] << " for a total of " << _cgi._buffer.size() + sent << RESET << std::endl;

	_cgi._buffer.insert(_cgi._buffer.end(), _request.body.begin() + start_pos, _request.body.begin() + start_pos + sent);

	if (_cgi._buffer.size() >= _request.expectedBodySize)
	{
		close(_cgi._fd[0]);
		FD_CLR(_cgi._fd[0], &_info.afds);
		_cgi._fd[0] = -1;
		std::cout << GREEN
				  << "Done sending Data to CGI !"
				  << RESET << std::endl;
		_cgi._buffer.clear();
		return (true);
	}
	return (true);
}

bool Client::recvDataFromCGI(ConfigFile const &_config, FDInfo &_info)
{
	char buffer[CHUNK_SIZE];

	ssize_t received = read(_cgi._fd[1], buffer, CHUNK_SIZE);
	if (received < 0)
	{
		std::cout << RED << "CGI Read Error" << RESET << std::endl;
		return (false);
	}
	std::cout << MAGENTA << "Received data from CGI (" << received << ") on _cgi._fd[1] " << _cgi._fd[1] << " for a total of " << _cgi._buffer.size() + received << RESET << std::endl;
	if (received == 0)
	{
		bool cgiSucceed = false;
		try
		{
			cgiSucceed = checkCGIExitStatus(_cgi._pid); // All good or not done yet
		}
		catch (const HTTPException &e) // catch error code from CGI
		{
			close(_cgi._fd[1]);
			FD_CLR(_cgi._fd[1], &_info.afds);
			_cgi._fd[1] = -1;
			std::cout << RED
					  << "CGI Failed [" << e.get_code()
					  << "] : " << e.what() << RESET << std::endl;
			_response = http_response(e.get_code(), "", _config, _request, _sessionInfos);
			_state = READING_FROM_SERVER;
		}

		if (cgiSucceed) // All good
		{
			close(_cgi._fd[1]);
			FD_CLR(_cgi._fd[1], &_info.afds);
			_cgi._fd[1] = -1;

			std::vector<char> bufferPrint(_cgi._buffer);
			bufferPrint.push_back(0);

			std::cout << GREEN
					  << "Received all the Data from CGI ("
					  << _cgi._buffer.size() << ")!"
					  << RESET << std::endl;
			std::cout << CYAN
					  << "CGI Data : " << bufferPrint.data()
					  << RESET << std::endl;
			_state = READING_FROM_SERVER;
			try
			{
				_response.full_response = parse_cgi_data(_cgi._buffer);
			}
			catch (const HTTPException &e)
			{
				std::cout << RED
						  << "CGI Invalid Output [" << e.get_code()
						  << "] : " << e.what() << RESET << std::endl;
				_response = http_response(e.get_code(), "", _config, _request, _sessionInfos);
			}
			_state = READING_FROM_SERVER;
			_cgi._buffer.clear();
			_cgi._pid = -1;
		}
		return (true);
	}
	_cgi._buffer.insert(_cgi._buffer.end(), buffer, buffer + received);
	return (true);
}

// ================================ TIMEOUT =============================

void Client::checkTimeout(const ConfigFile &_config)
{
	if (_state == READING_FROM_SERVER || _request.size == 0)
		return;
	clock_t now = clock();
	unsigned long elapsed = (now - _start_time) * 1000 / CLOCKS_PER_SEC;
	unsigned long timeOut = getTimeoutByLocation(_request.target, _config);
	if (elapsed > timeOut)
	{
		std::cout << RED
				  << "Client [" << _fd << "] "
				  << "Timeout !"
				  << RESET << std::endl;
		if (_state == PROCESSING_CGI)
		{
			int status = 0;
			if (_cgi._pid != -1)
			{
				pid_t ret = waitpid(_cgi._pid, &status, WNOHANG);
				if (ret == 0)
				{
					kill(_cgi._pid, SIGKILL);
					waitpid(_cgi._pid, &status, 0);
				}
			}
		}
		_response = http_response(408, "", _config, _request, _sessionInfos);
		_state = READING_FROM_SERVER;
	}
}
