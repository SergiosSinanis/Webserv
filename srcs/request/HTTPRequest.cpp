#include <sstream>
#include <cstdlib>
#include <algorithm>

#include "HTTPRequest.hpp"
#include "Colors.hpp"
#include "HTTPException.hpp"
#include "CheckRequest.hpp"
#include "Methods.hpp"

// ================================ CGI =============================

void checkCGILocation(HTTPRequest &_request, const ConfigFile &_config)
{
	std::string uri_loc = find_location_prefixe(_request.target, _config);
	if (uri_loc.length() > 1 && uri_loc[uri_loc.length() - 1] == '/')
		uri_loc = uri_loc.substr(0, uri_loc.length() - 1);

	std::vector<location>::const_iterator it;
	std::vector<location>::const_iterator ite = _config.locations.end();
	for (it = _config.locations.begin(); it != ite; it++)
	{
		if (uri_loc == it->locationDir)
		{
			_request.useCGI = !(*it).cgiExtensions.empty();
			break ;
		}
	}
}

// ================================ HEADERS =============================

static void parse_request_line(const std::string &line, HTTPRequest &_request, ConfigFile const &_config)
{
	// split request line
	std::istringstream start_line(line);
	std::string method_str, target, protocol;
	if (!(start_line >> method_str >> target >> protocol))
	{
		std::string msg = "Invalid HTTP request format : " + line;
		throw(HTTPException(msg.c_str(), 0));
	}

	// check leftover
	std::string left;
	if (start_line >> left)
	{
		std::string msg = "Too many fields in HTTP request line : " + line;
		throw(HTTPException(msg.c_str(), 0));
	}

	if (method_str == "GET")
		_request.method = GET;
	else if (method_str == "POST")
		_request.method = POST;
	else if (method_str == "PUT")
		_request.method = PUT;
	else if (method_str == "DELETE")
		_request.method = DELETE;
	else if (method_str == "PATCH")
		_request.method = PATCH;
	else if (method_str == "HEAD")
		_request.method = HEAD;
	else
	{
		std::string msg = "Unknown Method : " + method_str;
		throw(HTTPException(msg.c_str(), 0));
	}

	size_t query_pos = target.rfind("?");
	if (query_pos != std::string::npos)
	{
		_request.query_str = target.substr(query_pos + 1, target.size());
		_request.target = target.substr(0, query_pos);
	}
	else
		_request.target = target;

	_request.protocol = protocol;

	if (_request.target.size() > getMaxURIByLocation(_request.target, _config))
		throw(HTTPException("URI Too Long", 414));
}

bool isHeaderComplete(const HTTPRequest &_request)
{
	std::string string_buffer(_request.raw.begin(), _request.raw.end());
	return (string_buffer.find("\r\n\r\n") != std::string::npos);
}

static void parse_headers(const std::string &_headers, HTTPRequest &_request, ConfigFile const &_config)
{
	std::istringstream header_stream(_headers);
	std::string line;

	if (!std::getline(header_stream, line) || line.empty())
	{
		throw(HTTPException("Empty Header", 0));
	}

	try
	{
		parse_request_line(line, _request, _config); //
	}
	catch (const HTTPException &e)
	{
		std::string msg = "Request Line : ";
		msg.append(e.what());
		throw(HTTPException(msg.c_str(), e.get_code()));
	}
	fill_fields(_request); // remplir struct HTTPRequest with header
}

void fill_fields(HTTPRequest &_request)
{
	std::string string_buffer(_request.raw.begin(), _request.raw.end());
	size_t pos = string_buffer.find("\r\n\r\n");
	std::string header_string = string_buffer.substr(0, pos);
	std::istringstream header_stream(header_string);
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

			_request.headerFields[key] = value;
		}
	}
}

// ================================ COOKIES =============================

std::string trim(const std::string &str)
{
	size_t left = str.find_first_not_of(" \t\r\n"); // slot to word start
	if (left == std::string::npos)					// verifier entier whitespaces
		return "";
	size_t right = str.find_last_not_of(" \t\r\n"); // slot to word end
	return str.substr(left, right - left + 1);
}

// ft_split
std::vector<std::string> split(const std::string &str, char delimiter)
{
	std::vector<std::string> result;
	std::istringstream stream(str);
	std::string token;

	while (std::getline(stream, token, delimiter))
	{
		result.push_back(token);
	}
	return result;
}

void parseCookies(HTTPRequest &request)
{
	std::map<std::string, std::string>::iterator it = request.headerFields.find("cookie");
	if (it == request.headerFields.end())
		return; // pas de cookies

	const std::string &cookieLine = it->second;

	std::vector<std::string> cookieParts = split(cookieLine, ';');

	for (size_t i = 0; i < cookieParts.size(); i++)
	{
		size_t equalPos = cookieParts[i].find('=');
		if (equalPos == std::string::npos)
			continue;

		std::string key = trim(cookieParts[i].substr(0, equalPos));
		std::string value = trim(cookieParts[i].substr(equalPos + 1));
		request.cookies[key] = value;
	}

	// session_id
	std::map<std::string, std::string>::iterator sessionIt = request.cookies.find("session_id");

	if (sessionIt != request.cookies.end())
	{
		request.session_id = sessionIt->second;
	}

}

// ================================ BODY =============================

size_t getRawBodySize(const HTTPRequest &_request)
{
	std::string string_buffer(_request.raw.begin(), _request.raw.end());
	size_t pos = string_buffer.find("\r\n\r\n");

	std::string::size_type body_start = pos + 4;
	return (body_start > string_buffer.size() ? 0 : string_buffer.size() - body_start);
}

bool isBodyComplete(const HTTPRequest &_request)
{
	if (_request.waitFullBody)
	{
		if (_request.headerFields.count("content-length"))
		{
			std::string string_buffer(_request.raw.begin(), _request.raw.end());
			size_t pos = string_buffer.find("\r\n\r\n");

			std::string::size_type body_start = pos + 4;
			std::string::size_type body_size = string_buffer.size() - body_start;

			return (body_size >= _request.expectedBodySize);
		}
		else if (_request.headerFields.count("transfer-encoding") && _request.headerFields.at("transfer-encoding") == "chunked")
		{
			std::string string_buffer(_request.raw.begin(), _request.raw.end());
			size_t pos = string_buffer.find("\r\n0\r\n\r\n");
			return (pos != std::string::npos);
		}
	}
	return (true);
}

void parse_body(HTTPRequest &_request)
{
	if (_request.headerFields.count("content-length"))
	{
		_request.body.insert(_request.body.end(),
							 _request.raw.end() - _request.expectedBodySize,
							 _request.raw.end());
	}
	else if (_request.headerFields.count("transfer-encoding") && _request.headerFields.at("transfer-encoding") == "chunked")
	{
		std::vector<char>::iterator it = _request.raw.begin();
		std::vector<char>::iterator end = _request.raw.end();

		// find the start of the body after headers
		std::string headers(it, end);
		size_t header_end = headers.find("\r\n\r\n");
		if (header_end == std::string::npos)
			return; // malformed request
		it += header_end + 4;

		while (it < end)
		{
			std::vector<char>::iterator line_end = std::search(it, end, const_cast<char *>("\r\n"), const_cast<char *>("\r\n") + 2);
			if (line_end == end)
				break;

			std::string size_str(it, line_end);
			std::istringstream size_stream(size_str);
			int chunk_size = 0;
			size_stream >> std::hex >> chunk_size;
			if (chunk_size == 0)
				break;

			it = line_end + 2;

			std::string debug_str(it, end);

			if (std::distance(it, end) < chunk_size)
				break;

			_request.body.insert(_request.body.end(), it, it + chunk_size);

			it += chunk_size;
			while (*it != '\r')
				it++;

			if (std::distance(it, end) < 2)
				break;
			while (std::distance(it, end) > 2 && *it == '\r' && *(it + 1) == '\n')
				it += 2;
		}
	}
	_request.body.push_back(0);
}

// ================================ REQUEST =============================

void parse_request(HTTPRequest &request, ConfigFile const &_config)
{
	std::string string_buffer(request.raw.begin(), request.raw.end());
	std::string headers;
	std::string::size_type pos = string_buffer.find("\r\n\r\n");
	headers = string_buffer.substr(0, pos);
	try
	{
		parse_headers(headers, request, _config);
	}
	catch (const HTTPException &e)
	{
		throw(HTTPException(e.what(), e.get_code()));
	}
	try
	{
		checkRequest(request);
	}
	catch(const InvalidHeaderException& e)
	{
		throw(HTTPException(e.what(), 400));
	}

	if (request.headerFields.count("content-length"))
	{
		request.expectedBodySize = std::atoi(request.headerFields["content-length"].c_str());
		request.waitFullBody = true;
		if(request.expectedBodySize > getMaxBodySizeByLocation(request.target, _config))
			throw(HTTPException("Content Too Large", 413));
	}
	else if (request.headerFields.count("transfer-encoding") && request.headerFields["transfer-encoding"] == "chunked")
		request.waitFullBody = true;

	if (request.headerFields.count("cookie"))
		parseCookies(request);
	request.headerParsed = true;
}

// ================================ DEBUG =============================

void print_request(const HTTPRequest &request)
{
	std::cout << CYAN << "=== HTTP REQUEST ===" << std::endl;

	// Method
	std::cout << "Method: ";
	switch (request.method)
	{
	case GET:
		std::cout << BRIGHT_CYAN << "GET";
		break;
	case POST:
		std::cout << BRIGHT_CYAN << "POST";
		break;
	case PUT:
		std::cout << BRIGHT_CYAN << "PUT";
		break;
	case DELETE:
		std::cout << BRIGHT_CYAN << "DELETE";
		break;
	case PATCH:
		std::cout << BRIGHT_CYAN << "PATCH";
		break;
	case HEAD:
		std::cout << BRIGHT_CYAN << "HEAD";
		break;
	case OTHER:
		std::cout << BRIGHT_CYAN << "OTHER";
		break;
	default:
		std::cout << BRIGHT_CYAN << "UNKNOWN";
		break;
	}
	std::cout << RESET << std::endl;

	// Target and query string
	std::cout << CYAN << "Target: " << BRIGHT_CYAN << request.target << RESET << std::endl;
	if (!request.query_str.empty())
		std::cout << CYAN << "Query string: " << BRIGHT_CYAN << request.query_str << RESET << std::endl;

	// Protocol
	std::cout << CYAN << "Protocol: " << BRIGHT_CYAN << request.protocol << RESET << std::endl;

	// Flags
	std::cout << CYAN << "--- Flags ---" << RESET << std::endl;
	std::cout << CYAN << "Waiting full body: " << (request.waitFullBody ? BRIGHT_CYAN "true" : BRIGHT_CYAN "false") << RESET << std::endl;
	std::cout << CYAN << "Exceeded max body size: " << (request.exceedMaxBodySize ? BRIGHT_CYAN "true" : BRIGHT_CYAN "false") << RESET << std::endl;

	// Size info
	std::cout << CYAN << "Size (raw data): " << BRIGHT_CYAN << request.size << RESET << std::endl;
	std::cout << CYAN << "Expected body size: " << BRIGHT_CYAN << request.expectedBodySize << RESET << std::endl;

	// Headers
	std::cout << CYAN << "--- Headers ---" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request.headerFields.begin(); it != request.headerFields.end(); ++it)
	{
		std::cout << BRIGHT_CYAN << it->first << ": " << it->second << RESET << std::endl;
	}

	// Cookies
	if (!request.cookies.empty())
	{
		std::cout << CYAN << "--- Cookies ---" << std::endl;
		for (std::map<std::string, std::string>::const_iterator it = request.cookies.begin(); it != request.cookies.end(); ++it)
		{
			std::cout << BRIGHT_CYAN << it->first << ": " << it->second << RESET << std::endl;
		}
	}

	// Session ID
	if (!request.session_id.empty())
		std::cout << CYAN << "Session ID: " << BRIGHT_CYAN << request.session_id << RESET << std::endl;

	// CGI usage
	std::cout << CYAN << "--- CGI ---" << std::endl;
	std::cout << BRIGHT_CYAN << (request.useCGI ? "true" : "false") << RESET << std::endl;

	// Body
	std::cout << CYAN << "--- Body (" << request.body.size() << " bytes) ---" << RESET << std::endl;
	if (!request.body.empty())
	{
		std::cout << BRIGHT_CYAN;
		for (std::vector<char>::const_iterator it = request.body.begin(); it != request.body.end(); it++)
		{
			std::cout << (*it);
		}
		std::cout << RESET << std::endl;
	}
	std::cout << CYAN << "=====================" << std::endl;
}


void print_cookie(const HTTPRequest &request)
{
	/////
	std::cout << BRIGHT_YELLOW;
	std::cout << "=====COOKIES=====" << std::endl;
	std::cout << "ID: " << request.session_id << std::endl;
	std::map<std::string, std::string>::const_iterator it;
	std::map<std::string, std::string>::const_iterator ite = request.cookies.end();
	for (it = request.cookies.begin(); it != ite; it++)
		std::cout << it->first << " " << it->second << std::endl;

	std::cout << RESET;
}
