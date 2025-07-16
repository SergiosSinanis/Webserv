#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <string>
# include <map>
# include <utility>
# include <iostream>
# include <fstream>
#include <vector>

#include <sstream>
#include <string>

#include <ctime>
#include <cstring>
#include <unistd.h>

#include "HTTPRequest.hpp"
#include "ConfigFile.hpp"
#include "Colors.hpp"
# include "HTTPException.hpp"
# include "Session.hpp"   

typedef struct s_HTTPResponse_data
{
	//START-LINE
	METHODS method;
	std::string protocol;
	int statusCode;
	std::string uri;
	std::string root;

	//HEADER FIELDS
	std::map<std::string, std::string> headerFields;

	//BODY
	bool has_body;
	std::string body_path;
	std::vector<char> body;

}			HTTPResponse_data;


typedef struct s_HTTPResponse
{
	std::vector<char> full_response;
	size_t total_sent;
	s_HTTPResponse() : total_sent(0) {}
}			HTTPResponse;

std::string int_to_str(int value);
std::string get_date(void);
bool is_body_allowed(int code);
bool is_error_code(int code);
bool is_redirection_code(int code);
bool is_success_code(int code);
std::string get_status_message(int code);

HTTPResponse http_response(int code, std::string body_path, const ConfigFile &config, const HTTPRequest &request, const std::pair<Session *, bool> &session);
HTTPResponse http_return_response(const HTTPRequest &request, const ConfigFile &config);

void print_response(std::vector<char> response);

std::vector<char> load_body(const std::string &body_path);
std::string body_type(const std::string &body_path);

#endif
