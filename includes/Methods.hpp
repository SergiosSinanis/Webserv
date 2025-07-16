#ifndef METHODS_HPP
# define METHODS_HPP

# include "HTTPRequest.hpp"
# include "HTTPResponse.hpp"
# include "ConfigFile.hpp"
# include "Server.hpp"
# include "Session.hpp"
# include "SessionManager.hpp"

# include <sys/stat.h>
# include <unistd.h>
# include <utility>
# include <dirent.h>
# include <dirent.h>
# include <string>
# include <sstream>
# include <vector>
# include <algorithm>
# include <sys/stat.h>
# include <iomanip>
# include <ctime>
# include <map>

# define AUTO_INDEX "__AUTO_INDEX__"
# define LOGOUT "__LOGOUT__"
# define UPLOADS "__UPLOADS__"
# define REDIRECT "__UPLOADS__"


// METHODS

HTTPResponse get_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info);
HTTPResponse put_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info);
HTTPResponse delete_request(const ConfigFile& config, const HTTPRequest& request, EMETHODS method,const std::pair<Session *, bool> &session_info);
HTTPResponse patch_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info);
HTTPResponse post_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info);

// RESPONSE UTILS

void add_cookie_header(HTTPResponse_data &response, Session* session);
int get_content_header_code(const HTTPRequest &request);
std::vector<char> generate_auto_index(const std::string &path, const std::string &uri);

// MULTIPART BODY

typedef struct s_multipart
{
	std::map<std::string, std::string> content_disposition;
	std::string content_type;
	std::vector<char> body;

	s_multipart() {}

} multipart;

std::vector<multipart> parse_multipart_body(const HTTPRequest &request,const std::string &boundary);
void print_multipart(const std::vector<multipart> &mult);
std::string get_boundary(const HTTPRequest &request);
bool is_valid_filename(const std::string &filename);

// UTILS
bool is_upload_enabled(std::string uri_loc, const ConfigFile &config);
bool is_method_allowed(METHODS method, std::string uri_loc, const ConfigFile &config);
std::string method_to_str(METHODS _method);
METHODS str_to_method(const char * method_str);
void trim_unquote(std::string &str);
std::string valid_extension(const std::string &file);
std::string find_location_prefixe(std::string path, const ConfigFile &config);
location find_location(std::string path, const ConfigFile &config);
std::string trim_location_root(std::string path, const ConfigFile &config);
std::string find_index(std::vector<std::string> index, std::string path);
std::string valid_uri(const std::string &full_path);
void tryDeleteProfilePicture(const std::string& path, const std::string& fileName);

unsigned int getMaxURIByLocation(std::string _location, const ConfigFile &_config);
unsigned long getTimeoutByLocation(std::string _location, const ConfigFile &_config);
std::string getRootByLocation(std::string _location, const ConfigFile &_config);
size_t getMaxBodySizeByLocation(std::string _location, const ConfigFile &_config);

#endif
