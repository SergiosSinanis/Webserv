#include "ConfigFile.hpp"

void listen_header(ConfigFile &config, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2 || !is_number(token_line[1]))
		throw(configParseException("Error: PORT is not a number"));
	int port = std::atoi((token_line)[1].c_str());
	if (port < 0 || port > 65535)
		throw(configParseException("Error: port " + token_line[1] + " is out of range"));
	config.port = port;
}

void server_name_header(ConfigFile &config, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing server name"));
	for (size_t i = 1; i < token_line.size() && token_line[i] != ";"; i++)
	{
		std::string name = (token_line)[i];
		if (is_DNS(name))
		{
			if (std::find(config.serverName.begin(), config.serverName.end(), name) == config.serverName.end())
				config.serverName.push_back(name);
		}
	}
}

void root_header(ConfigFile &config, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw configParseException("Error: missing root value");

	std::string root = token_line[1];
	is_directory(root, "");
	config.root = root;
}

void host_header(ConfigFile &config, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing host value"));

	std::string host = token_line[1];
	for (size_t i = 0; i < host.length(); i++)
	{
		if (!isdigit(host[i]) && host[i] != '.' && host[i] != '-')
		{
			if (is_DNS(host))
			{
				config.host = host;
				return;
			}
		}
	}
	if (is_IPV4(host))
		config.host = host;
	else
		throw(configParseException("Error: wrong host value"));
}

void index_header(ConfigFile &config, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 2)
		throw(configParseException("Error: missing index value"));

	for (size_t i = 1; i < token_line.size() && token_line[i] != ";"; i++)
	{
		std::string index = token_line[i];
		if (!is_valid_index_name(index))
			throw configParseException("Error: " + index + " invalid characters in index name");

		if (is_file(index, config.root))
		{
			if (std::find(config.index.begin(), config.index.end(), index) == config.index.end())
				config.index.push_back(index);
		}
	}
}

void error_page_header(ConfigFile &config, const std::vector<std::string> &token_line)
{
	if (token_line.size() < 3 || !is_number(token_line[1]))
		throw(configParseException("Error: missing or invalid error code"));

	size_t line_size = token_line.size() - 1;
	if (token_line[token_line.size() - 1] == ";")
		line_size--;

	std::vector<int> error_codes;
	size_t i;
	for (i = 1; i < line_size; i++)
	{
		if (!is_number(token_line[i]))
			throw(configParseException("Error: error code is not a number"));

		int err_code = std::atoi(token_line[i].c_str());
		if (err_code < 400 || err_code > 599)
			throw(configParseException("Error: error code is out of valid range (400-599)"));

		if (std::find(error_codes.begin(), error_codes.end(), err_code) == error_codes.end())
			error_codes.push_back(err_code);
	}

	std::string error = token_line[i];
	if (!is_http(error))
	{
		is_file(error, config.root);
		// has_extension(error);
	}

	std::vector<int>::iterator it;
	std::vector<int>::iterator ite = error_codes.end();

	for (it = error_codes.begin(); it != ite; it++)
		config.errorPage.insert(std::make_pair(*it, error));
}

void parse_server_header(const std::string &header, ConfigFile &config, const std::vector<std::string> &token_line)
{
	typedef void (*header_fct)(ConfigFile &, const std::vector<std::string> &);

	std::map<std::string, header_fct> allowed_header;
	allowed_header.insert(std::make_pair("listen", listen_header));
	allowed_header.insert(std::make_pair("server_name", server_name_header));
	allowed_header.insert(std::make_pair("root", root_header));
	allowed_header.insert(std::make_pair("host", host_header));
	allowed_header.insert(std::make_pair("index", index_header));
	allowed_header.insert(std::make_pair("error_page", error_page_header));

	std::map<std::string, header_fct>::iterator it;
	it = allowed_header.find(header);
	if (it != allowed_header.end())
		it->second(config, token_line);
	else
	{
		std::cerr << header << std::endl;
		throw(configParseException("Error: unknown header"));
	}
}

void parse_server(ConfigFile &config,
					   std::vector<std::vector<std::string> >::iterator &it,
					   std::vector<std::vector<std::string> >::iterator ite)
{
	if ((*it).size() < 2 || (*it)[1] != "{")
	{
		throw(configParseException("Error: missing { after server"));
	}

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
			if ((*it).front() == "location")
			{
				location loc;
				parse_location(loc, it, ite);
				config.locations.push_back(loc);
				continue;
			}
			else
			{
				if ((*it).back() != ";")
				{
					throw(configParseException("Error: missing ;"));
				}
				parse_server_header((*it).front(), config, *it);
			}
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
		throw(configParseException("Error: missing closing } for server block"));
}
