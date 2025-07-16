#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>
# include <vector>
#include <iomanip>

// # include "CheckRequest.hpp"

typedef enum EMETHODS
{
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	PATCH,
	OTHER
} METHODS;

typedef struct s_HTTPRequest
{
	//RAW DATA
	std::vector<char> raw;
	size_t size;
	size_t expectedBodySize;

	//FLAG
	bool headerParsed;
	bool waitFullBody;
	bool exceedMaxBodySize;
	bool useCGI;

	//START-LINE
	METHODS method;
	std::string target;
	std::string query_str;
	std::string protocol;

	//HEADER FIELDS
	std::map<std::string, std::string> headerFields;

	//BODY
	std::vector<char> body;

	//Bonus
	std::map<std::string, std::string> cookies;
	std::string session_id;

	s_HTTPRequest() : size(0), expectedBodySize(0), headerParsed(false),
		waitFullBody(false), exceedMaxBodySize(false), useCGI(false),
		method(OTHER) {}

} HTTPRequest;

std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
void parseCookies(HTTPRequest& request);

struct s_ConfigFile;
typedef struct s_ConfigFile ConfigFile;

void		parse_request(HTTPRequest &request, ConfigFile const &_config);
void		parse_body(HTTPRequest &_request);
void		fill_fields(HTTPRequest &_request);
void		print_request(const HTTPRequest &_request);
bool		isHeaderComplete(const HTTPRequest &_request);
bool		isBodyComplete(const HTTPRequest &_request);
size_t		getRawBodySize(const HTTPRequest &_request);


void		checkCGILocation(HTTPRequest &_request, const ConfigFile &_config);
void		print_cookie(const HTTPRequest &request);



#endif
