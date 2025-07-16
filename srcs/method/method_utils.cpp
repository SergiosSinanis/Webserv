#include "Methods.hpp"

bool file_exists(const std::string &path)
{
	struct stat st;

	if (stat(path.c_str(), &st) != 0)
		return (false);
	if (!S_ISREG(st.st_mode))
		return false;
	if (access(path.c_str(), R_OK) == -1)
		return (false);
	return (true);
}

bool is_method_allowed(METHODS method, std::string uri_loc, const ConfigFile &config)
{
	if (uri_loc.length() > 1 && uri_loc[uri_loc.length() - 1] == '/')
		uri_loc = uri_loc.substr(0, uri_loc.length() - 1);

	std::vector<location>::const_iterator it;
	std::vector<location>::const_iterator ite = config.locations.end();
	for (it = config.locations.begin(); it != ite; it++)
	{
		if (uri_loc == it->locationDir)
		{
			if (std::find(it->allowedMethods.begin(), it->allowedMethods.end(), method) == it->allowedMethods.end())
				return (false);
		}
	}
	return (true);
}
bool is_upload_enabled(std::string uri_loc, const ConfigFile &config)
{
	if (uri_loc.length() > 1 && uri_loc[uri_loc.length() - 1] == '/')
		uri_loc = uri_loc.substr(0, uri_loc.length() - 1);
	std::vector<location>::const_iterator it;
	std::vector<location>::const_iterator ite = config.locations.end();
	for (it = config.locations.begin(); it != ite; it++)
	{
		if (uri_loc == it->locationDir)
		{
			if (it->upload.empty() || it->upload == "off")
				return (false);
		}
	}
	return (true);
}

std::string find_location_prefixe(std::string path, const ConfigFile &config)
{
	std::string prefixe;

	while (!path.empty())
	{
		struct stat st;

		if ((stat((config.root + path).c_str(), &st) == 0 && S_ISDIR(st.st_mode)) || path[path.length() - 1] == '/')
			prefixe = path;
		else
		{
			if (prefixe[prefixe.length() - 1] == '/')
				prefixe = prefixe.substr(0, prefixe.length() - 2);
			size_t pos = path.rfind('/');
			if (pos == std::string::npos)
				break;
			prefixe = path.substr(0, pos + 1);
		}

		std::vector<location>::const_iterator it;
		std::vector<location>::const_iterator ite = config.locations.end();
		for (it = config.locations.begin(); it != ite; it++)
		{
			if (prefixe == it->locationDir + "/")
				return (prefixe);
			if (prefixe == it->locationDir)
				return (prefixe + "/");
		}

		if (prefixe == "/")
			break;
		path = prefixe.substr(0, prefixe.length() - 1);
	}
	return ("/");
}

location find_location(std::string path, const ConfigFile &config)
{

	std::string uri_loc = find_location_prefixe(path, config);
	if (uri_loc.length() > 1 && uri_loc[uri_loc.length() - 1] == '/')
		uri_loc = uri_loc.substr(0, uri_loc.length() - 1);

	// std::cout << YELLOW << path << ": " << uri_loc << RESET << std::endl;

	std::vector<location>::const_iterator it;
	std::vector<location>::const_iterator ite = config.locations.end();
	for (it = config.locations.begin(); it != ite; it++)
	{
		// std::cout << YELLOW << uri_loc << ": " << it->locationDir << RESET << std::endl;

		if (uri_loc == it->locationDir)
			return (*it);
	}
	return (config.locations.front());
}

std::string trim_location_root(std::string path, const ConfigFile &config)
{
	location location = find_location(path, config);
	if (!location.root.empty())
	{
		size_t pos = path.find(location.locationDir);

		if (pos == 0)
			return (location.locationDir.length() == 1 ? path : path.substr(location.locationDir.length()));
		}
	return (path);
}

std::string find_index(std::vector<std::string> index, std::string path)
{

	if (!index.empty())
	{
		std::vector<std::string>::const_iterator it;
		std::vector<std::string>::const_iterator ite = index.end();
		for (it = index.begin(); it != ite; it++)
		{
			if (file_exists(path + *it))
				return (path + *it);
		}
	}
	return ("");
}

std::string valid_uri(const std::string &full_path)
{
	struct stat st;

	if (stat(full_path.c_str(), &st) == 0)
	{
		if (S_ISDIR(st.st_mode))
			return (full_path);
	}
	std::string dir_path;

	size_t pos = full_path.rfind('/');
	if (pos == std::string::npos)
		return ("");
	dir_path = full_path.substr(0, pos + 1);

	return (dir_path);
}

int get_content_header_code(const HTTPRequest &request)
{
	if (request.headerFields.find("content-length") == request.headerFields.end() && request.headerFields.find("transfer-encoding") == request.headerFields.end())
		return (411);
	if (request.headerFields.find("content-length") == request.headerFields.end() && request.headerFields.find("transfer-encoding") == request.headerFields.end())
		return (411);
	if (request.headerFields.at("content-length").empty() && request.headerFields.at("transfer-encoding").empty())
		return (411);
	if (!is_number(request.headerFields.at("content-length")))
		return (400);

	int length = std::atoi(request.headerFields.at("content-length").c_str());
	if (length < 0)
		return (400);
	std::map<std::string, std::string>::const_iterator it = request.headerFields.find("content-type");
	if (it == request.headerFields.end() || it->second.empty())
		return (415);
	return (0);
}

std::string method_to_str(METHODS _method)
{
	switch (_method)
	{
	case GET:
		return "GET";
	case POST:
		return "POST";
	case PUT:
		return "PUT";
	case DELETE:
		return "DELETE";
	case PATCH:
		return "PATCH";
	case HEAD:
		return "HEAD";
	default:
		return "UNKNOWN";
	}
}

METHODS str_to_method(const char *method_str)
{
	if (strcmp(method_str, "GET") == 0)
		return GET;
	if (strcmp(method_str, "POST") == 0)
		return POST;
	if (strcmp(method_str, "PUT") == 0)
		return PUT;
	if (strcmp(method_str, "DELETE") == 0)
		return DELETE;
	if (strcmp(method_str, "PATCH") == 0)
		return PATCH;
	if (strcmp(method_str, "HEAD") == 0)
		return HEAD;
	return OTHER;
}

bool is_valid_filename(const std::string &filename)
{
	if (filename.empty())
		return false;

	if (filename.find("..") != std::string::npos)
		return false;
	if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos)
		return false;

	for (size_t i = 0; i < filename.length(); ++i)
	{
		char c = filename[i];
		if (!(isalnum(c) || c == '.' || c == '-' || c == '_' || c == ' '))
			return false;
	}
	return true;
}

std::string valid_extension(const std::string &file)
{
	std::string arr[] = {
		"html", "htm",
		"css",
		"js",
		"json",
		"jpg", "jpeg", "png",
		"pdf",
		"bin",
		"txt"
	};
	std::vector<std::string> allowed_extension(arr, arr + sizeof(arr) / sizeof(arr[0]));

	size_t pos = file.rfind('.');
	if (pos == std::string::npos || pos == 0 || pos == file.length() - 1)
		return (".bin");
	std::string extension = file.substr(pos + 1);

	if (std::find(allowed_extension.begin(), allowed_extension.end(), extension) == allowed_extension.end())
		return (".bin");

	return ("." + extension);
}

unsigned long getTimeoutByLocation(std::string _location, const ConfigFile &_config)
{
	if (find_location(_location, _config).timeout)
		return (find_location(_location, _config).timeout);
	else
		return (_config.locations.front().timeout);
}

unsigned int getMaxURIByLocation(std::string _location, const ConfigFile &_config)
{
	if (find_location(_location, _config).maxUriSize)
		return (find_location(_location, _config).maxUriSize);
	else
		return (_config.locations.front().maxUriSize);
}

size_t getMaxBodySizeByLocation(std::string _location, const ConfigFile &_config)
{
	if (find_location(_location, _config).maxBodySize)
		return (find_location(_location, _config).maxBodySize);
	else
		return (_config.locations.front().maxBodySize);
}

std::string getRootByLocation(std::string _location, const ConfigFile &_config)
{
	std::string root = find_location(_location, _config).root;
	if (!root.empty())
	{
		if (root[root.length() - 1] != '/')
			return (root + "/");
		else
			return (root);

	}
	else
		return (_config.root);
}
