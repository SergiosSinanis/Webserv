#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <map>
# include <utility>
# include <fstream>
# include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <cctype>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <iterator>


# include <vector>
# include "HTTPRequest.hpp"

typedef struct s_location
{
	std::string locationDir;
	std::string autoindex;
	std::string root;
	std::vector<std::string> index;
	std::vector<METHODS> allowedMethods;
	std::pair<int, std::string> returnRedirection;
	std::map<std::string, std::string> cgiExtensions;
	std::string upload;
	unsigned long timeout;
	unsigned int maxUriSize;
	size_t maxBodySize;

	s_location() : timeout(0), maxUriSize(0), maxBodySize(0) {}

} 				location;

typedef struct s_ConfigFile
{
	int port;
	std::string root;
	std::vector<std::string> serverName;
	std::string host;
	std::map<int, std::string> errorPage;
	std::vector<location> locations;
	std::vector<std::string> index;

	s_ConfigFile() : port(0) {}
} 				ConfigFile;

class configParseException: public std::exception
{
	public:
		configParseException();
		configParseException(std::string error);
		virtual ~configParseException() throw();
		const char *what() const throw();
	private:
		std::string _error;
};

void print_vector(const std::vector<std::string> &v);
void print_location(const location &loc);
void print_tokens(const std::vector<std::vector<std::string> >& containers);
void print_server_config(const ConfigFile &conf);

bool is_number(const std::string &nb);

METHODS str_to_enum(const std::string &str);
bool is_DNS(const std::string &name);
bool is_IPV4(const std::string &ip);
bool is_http(const std::string& url);
bool is_valid_index_name(const std::string& name);
bool is_file(const std::string &file, const std::string &root);
bool is_directory(const std::string &directory, const std::string &root);
bool has_extension(const std::string &file);

std::vector<std::vector<std::string> > config_to_token(std::ifstream &ifConf);
void parse_server(ConfigFile &config, std::vector<std::vector<std::string> >::iterator &it, std::vector<std::vector<std::string> >::iterator ite);
void parse_location_header(const std::string &header, location &loc, const std::vector<std::string> &token_line);
void parse_server_header(const std::string &header, ConfigFile &config, const std::vector<std::string> &token_line);
void parse_location(location &loc,
std::vector<std::vector<std::string> >::iterator &it,
std::vector<std::vector<std::string> >::iterator ite);

int parse_config_file(char *av, std::vector<ConfigFile> &configs);

#include "../srcs/config/configFile.tpp"

#endif
