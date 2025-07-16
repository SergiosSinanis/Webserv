#include "ConfigFile.hpp"


void print_location(const location &loc)
{
	std::cout << "location directory: " << loc.locationDir << std::endl;
	if (!loc.autoindex.empty())
		std::cout << "auto index: " << loc.autoindex << std::endl;
	if (!loc.index.empty())
	{
		std::cout << "index: ";
		print_vector<std::string>(loc.index);
	}
	if (!loc.allowedMethods.empty())
	{
		std::cout << "allowed methods: ";
		for (std::vector<METHODS>::const_iterator it = loc.allowedMethods.begin(); it != loc.allowedMethods.end(); it++)
			std::cout << *it << " ";
		std::cout << std::endl;
	}

	if (!loc.returnRedirection.second.empty())
		std::cout << "return redirection: " << loc.returnRedirection.first << ", " << loc.returnRedirection.second << std::endl;

	if (!loc.cgiExtensions.empty())
	{
		std::cout << "cgi extendion: ";
		for (std::map<std::string, std::string>::const_iterator it = loc.cgiExtensions.begin(); it != loc.cgiExtensions.end(); it++)
			std::cout << it->first << " " << it->second << " --- ";
		std::cout << std::endl;
	}
	if (!loc.upload.empty())
		std::cout << "upload: " << loc.upload << std::endl;

	if (loc.timeout)
		std::cout << "timeout: " << loc.timeout << std::endl;

	if (loc.maxBodySize)
		std::cout << "maxBodySize: " << loc.maxBodySize << std::endl;
}

void print_server_config(const ConfigFile &conf)
{
	std::cout << "server name: ";
	print_vector<std::string>(conf.serverName);
	std::cout << "port: " << conf.port << std::endl;
	std::cout << "host: " << conf.host << std::endl;

	std::cout << "error page: ";
	for (std::map<int, std::string>::const_iterator it = conf.errorPage.begin(); it != conf.errorPage.end(); it++)
		std::cout << it->first <<" , " << it->second << std::endl;
	std::cout << "root: " << conf.root << std::endl;
	std::cout << "index: ";
	print_vector<std::string>(conf.index);
	std::cout << std::endl;

	std::vector<location>::const_iterator it;
	std::vector<location>::const_iterator ite = conf.locations.end();
	for (it = conf.locations.begin(); it != ite; it++)
	{
		print_location(*it);
		std::cout << std::endl;
	}

}

void print_tokens(const std::vector<std::vector<std::string> > &containers)
{
	std::vector<std::vector<std::string> >::const_iterator it;
	std::vector<std::vector<std::string> >::const_iterator ite = containers.end();

	for (it = containers.begin(); it != ite; ++it)
	{
		std::cout << "-";

		std::vector<std::string>::const_iterator it_node;
		std::vector<std::string>::const_iterator ite_node = it->end();

		for (it_node = it->begin(); it_node != ite_node; ++it_node)
		{
			std::cout << *it_node << " ";
		}
		std::cout << "-" << std::endl;
	}
	std::cout << std::endl;
}

METHODS str_to_enum(const std::string &str)
{
	if (str == "GET")
		return GET;
	if (str == "HEAD")
		return HEAD;
	else if (str == "POST")
		return POST;
	else if (str == "PUT")
		return PUT;
	else if (str == "DELETE")
		return DELETE;
	else if (str == "PATCH")
		return PATCH;
	return OTHER;
}

bool is_number(const std::string &nb)
{
	size_t i = 0;
	if (nb[i] == '-' || nb[i] == '+')
		i++;
	for (; i < nb.length(); i++)
	{
		if (!isdigit(nb[i]))
			return false;
	}
	return (true);
}

bool is_DNS(const std::string &name)
{
	if (name.empty() || name.length() > 253)
		throw(configParseException("Error: " + name + " length invalid"));

	size_t pos = name.find('*');
	size_t start = 0;
	if (pos != std::string::npos && pos != 0)
		throw(configParseException("Error: " + name + " invalid wildcard should be at the beginning"));
	if (pos == 0)
	{
		start = 1;
		if (name.length() < 2 || name[1] != '.')
			throw(configParseException("Error: " + name + " invalid wildcard, must be followed by '.'"));
	}

	for (size_t i = start; i < name.length(); i++)
	{
		char c = name[i];
		if (!(isalnum(c) || c == '.' || c == '-'))
			throw(configParseException("Error: " + name + " wrong domain name character"));
	}

	if (name[0] == '.' || name[name.length() - 1] == '.' || name[0] == '-' || name[name.length() - 1] == '-')
		throw(configParseException("Error: " + name + " domain can't start or end with '.' / '-' "));

	size_t prev_pos = 0;
	while (1)
	{
		size_t pos_pt = name.find('.', prev_pos);
		if (pos_pt == std::string::npos)
		{
			if (prev_pos > name.length() - 2)
				throw(configParseException("Error: " + name + " extension should at least have 2 characters"));
			break;
		}
		if (pos_pt == prev_pos)
			throw(configParseException("Error: " + name + " consecutive dots are not allowed"));
		prev_pos = pos_pt + 1;
	}

	return (true);
}

bool is_IPV4(const std::string &ip)
{
	std::istringstream iip(ip);
	std::string node;

	for (int i = 0; i < 3; i++)
	{
		getline(iip, node, '.');
		if (node.length() < 1 || node.length() > 3)
			throw(configParseException("Error: " + ip + " wrong IPV4 format"));
		if (!is_number(node))
			throw(configParseException("Error: " + ip + " wrong IPV4 format"));
		int oct = std::atoi(node.c_str());
		if (oct < 0 || oct > 255)
			throw(configParseException("Error " + ip + " wrong IPV4 format " + node + " out of bound"));
		node.clear();
	}
	getline(iip, node);
	if (node.length() < 1 || node.length() > 3)
		throw(configParseException("Error: " + ip + " wrong IPV4 format"));

	if (!is_number(node))
		throw(configParseException("Error: " + ip + " wrong IPV4 format"));
	int oct = std::atoi(node.c_str());
	if (oct < 0 || oct > 255)
		throw(configParseException("Error " + ip + " wrong IPV4 format " + node + " out of bound"));
	return (true);
}

bool is_http(const std::string& url)
{
    if (url.compare(0, 7, "http://") == 0)
		return (true);
    if (url.compare(0, 8, "https://") == 0)
		return (true);
    return (false);
}

bool is_valid_index_name(const std::string& name)
{
	for (size_t i = 0; i < name.length(); ++i) {
		if (!isalnum(name[i]) && name[i] != '-' && name[i] != '_' && name[i] != '.')
			return (false);
	}
	return (true);
}

bool has_extension(const std::string &file)
{
	std::string arr[] = {"html", "php", "sh", "bin"};
	std::vector<std::string> allowed_extension(arr, arr + sizeof(arr) / sizeof(arr[0]));

    size_t pos = file.rfind('.');
    if (pos == std::string::npos || pos == 0 || pos == file.length() - 1)
		return (false);
    std::string extension = file.substr(pos + 1);

	if (std::find(allowed_extension.begin(), allowed_extension.end(), extension) == allowed_extension.end())
		throw(configParseException("Error: " + file + " unknown extension"));

    return (true);
}



bool is_file(const std::string &file, const std::string &root)
{
	std::string full_path;

	if (!root.empty() && root[root.length() - 1] != '/')
		full_path += root + '/';

	full_path += file;

	std::ifstream ipath(full_path.c_str());
	if (!ipath)
		throw(configParseException("Error: " + full_path + " " + strerror(errno)));

	return (true);
}


bool is_directory(const std::string &directory, const std::string &root)
{
    std::string full_path;

    if (root.empty())
        full_path = directory;
    else
    {
        full_path = root;
        if (full_path[full_path.length() - 1] != '/')
            full_path += '/';
        full_path += directory;
    }

    struct stat info;
    if (stat(full_path.c_str(), &info) != 0 || !S_ISDIR(info.st_mode))
        throw configParseException("Error: " + full_path + " " + strerror(errno));

    DIR *dir = opendir(full_path.c_str());
    if (!dir)
        throw configParseException("Error: " + full_path + " " + strerror(errno));
    closedir(dir);
    return (true);
}

