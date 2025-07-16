#include "ConfigFile.hpp"
#include "Colors.hpp"

void location_header(location &loc, const std::vector<std::string> &token_line)
{
	size_t pos = token_line[1].find("/");
	if (pos != std::string::npos && pos > 3)
		throw(configParseException("Error: missing directory"));
	if (token_line.size() < 3)
		throw(configParseException("Error: incomplete location declaration"));

	loc.locationDir = token_line[1];

	if (token_line.size() < 3 || token_line[2] != "{")
		throw(configParseException("Error: missing {"));
}

void auto_index_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2 || (token_line[1] != "on" && token_line[1] != "off"))
		throw(configParseException("Error: undefined autoindex"));
	loc.autoindex = token_line[1];
}

void allow_methods_header(location &loc, const std::vector<std::string> &token_line)
{
	std::string arr[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD"};
	std::vector<std::string> allowed_enum(arr, arr + sizeof(arr) / sizeof(arr[0]));

	std::string str_enum;
	for (size_t i = 1; i < token_line.size() && token_line[i] != ";"; i++)
	{
		std::vector<std::string>::iterator it;
		str_enum = token_line[i];
		if (std::find(allowed_enum.begin(), allowed_enum.end(), str_enum) == allowed_enum.end())
			throw(configParseException("Error: " + str_enum + " unknown method"));

		if (std::find(loc.allowedMethods.begin(), loc.allowedMethods.end(), str_to_enum(str_enum)) == loc.allowedMethods.end())
			loc.allowedMethods.push_back(str_to_enum(str_enum));
	}
}

void return_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2 || !is_number(token_line[1]))
		throw(configParseException("Error: return status is not a number"));

	int code = std::atoi(token_line[1].c_str());
	if (code < 100 || code > 599)
		throw(configParseException("Error: return status is out of valid HTTP range"));
	if (code >= 100 && code < 200)
		throw(configParseException("Error: return status " + token_line[1] + " is informational (1xx), not allowed"));

	loc.returnRedirection.first = code;

	if (token_line.size() == 2 || (token_line.size() == 3 && token_line[2] == ";"))
	{
		loc.returnRedirection.second = "";
		return;
	}

	if (token_line.size() >= 3)
	{
		std::string redirection = token_line[2];
		if (redirection.empty())
			throw(configParseException("Error: redirection path is empty"));

		if (!is_http(redirection) && redirection[0] != '/')
			throw(configParseException("Error: invalid return target — must be full URL or start with '/'"));

		loc.returnRedirection.second = redirection;

		if (token_line.size() > 4)
			throw(configParseException("Error: too many arguments in return directive"));
	}
}

void index_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing index value"));

	if (token_line.size() < 2)
		throw(configParseException("Error: missing index value"));

	std::string index;
	for (size_t i = 1; i < token_line.size() && token_line[i] != ";"; i++)
	{
		index = token_line[i];
		for (size_t j = 0; j < index.length(); j++)
		{
			if (!isalnum(index[j]) && index[j] != '-' && index[j] != '_' && index[j] != '.')
				throw(configParseException("Error: " + index + " wrong index format"));
		}

		if (std::find(loc.index.begin(), loc.index.end(), index) == loc.index.end())
			loc.index.push_back(index);
	}
}

void cgi_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing cgi extension"));

	std::string extension = token_line[1];
	// has_extension(extension);
	if (extension.empty() || extension[0] != '.')
		throw(configParseException("Error: cgi extension must start with '.'"));

	std::string interpreter;
	if (token_line.size() >= 3 && token_line[2] != ";")
		interpreter = token_line[2];
	else
		interpreter = "";

	loc.cgiExtensions.insert(std::pair<std::string, std::string>(extension, interpreter));
}

void upload_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2 || (token_line[1] != "on" && token_line[1] != "off"))
		throw(configParseException("Error: undefined upload enabler"));
	loc.upload = token_line[1];
}

void timeout_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing timeout value"));
	if (!is_number(token_line[1]))
		throw(configParseException("Error:timeout value is not a number"));
	unsigned long timeout = std::atol(token_line[1].c_str());
	loc.timeout = timeout;
}

void  max_uri_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing value for max uri"));
	if (!is_number(token_line[1]))
		throw(configParseException("Error:uri max size value is not a number"));
	unsigned int size = std::atoi(token_line[1].c_str());
	loc.maxUriSize = size;
}

void root_location_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw configParseException("Error: missing root value");

	std::string root = token_line[1];
	is_directory(root, "");
	loc.root = root;
}

void max_size_header(location &loc, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing max_body_size value"));
	std::string max_size = token_line[1];
	if (max_size.empty())
		throw(configParseException("Error: empty max_body_size value"));

	char suffix = max_size[max_size.length() - 1];
	std::string number = max_size.substr(0, max_size.size() - 1); //1
	size_t multiplier = 1;

	if (!isdigit(suffix))
	{
		switch (suffix)
		{
		case 'k':
		case 'K':
			multiplier = 1e3;
			break;
		case 'm':
		case 'M':
			multiplier = 1e6;
			break;
		case 'g':
		case 'G':
			multiplier = 1e9;
			break;
		default:
			throw(configParseException("Error: client_max_body_size suffix not supported"));
			break;
		}
	}
	else
		number = max_size;
	for (size_t i = 0; i < number.size(); i++)
	{
		if (!isdigit(number[i]) && number[i] != '.' && number[i] != '-')
			throw(configParseException("Error:  client_max_body_size is not a number"));
	}

	int size = std::atoi(number.c_str());
	if (size < 0)
		throw(configParseException("Error:  client_max_body_size value cannot be negative"));

	std::stringstream stream(number);
	size_t output;
	stream >> output;
	loc.maxBodySize = output * multiplier;
}

void parse_location_header(const std::string &header, location &loc, const std::vector<std::string> &token_line)
{
	typedef void (*header_fct)(location &,const std::vector<std::string> &);

	std::map<std::string, header_fct> allowed_header;

	allowed_header.insert(std::make_pair("location", location_header));
	allowed_header.insert(std::make_pair("autoindex", auto_index_header));
	allowed_header.insert(std::make_pair("allow_methods", allow_methods_header));
	allowed_header.insert(std::make_pair("return", return_header));
	allowed_header.insert(std::make_pair("index", index_header));
	allowed_header.insert(std::make_pair("cgi", cgi_header));
	allowed_header.insert(std::make_pair("upload_enabled", upload_header));
	allowed_header.insert(std::make_pair("timeout", timeout_header));
	allowed_header.insert(std::make_pair("max_uri_size", max_uri_header));
	allowed_header.insert(std::make_pair("max_body_size", max_size_header));
	allowed_header.insert(std::make_pair("root", root_location_header));

	std::map<std::string, header_fct>::iterator it;

	it = allowed_header.find(header);
	if (it != allowed_header.end())
		it->second(loc, token_line);
	else
		throw(configParseException("Error: unknown header"));
}

void parse_location(location &loc,
					std::vector<std::vector<std::string> >::iterator &it,
					std::vector<std::vector<std::string> >::iterator ite)
{
	if ((*it).size() < 3 || (*it)[2] != "{")
	{
		throw(configParseException("Error: missing { after location"));
	}

	parse_location_header((*it).front(), loc, *it);
	++it;

	int brace_level = 1;
	while (it != ite && brace_level > 0)
	{
		if ((*it).empty())
		{
			++it;
			continue;
		}

		if (brace_level > 0 && (*it).front() != "}")
		{
			if ((*it).back() != ";")
			{
				throw(configParseException("Error: missing ;"));
			}
			parse_location_header((*it).front(), loc, *it);
		}

		for (size_t i = 0; i < (*it).size(); ++i)
		{
			if ((*it)[i] == "{")
			++brace_level;
			else if ((*it)[i] == "}")
			--brace_level;
		}

		++it;
	}

	if (brace_level != 0)
		throw(configParseException("Error: missing closing } for location block"));
}

