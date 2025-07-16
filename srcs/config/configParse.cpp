#include "ConfigFile.hpp"

void parse_config(const std::string &path, std::vector<ConfigFile> &configs) 
{
    
	std::ifstream ifConf(path.c_str());
    if (!ifConf)
        throw(configParseException("Error: could not open file " + path));

    std::vector<std::vector<std::string> > tokens;
	tokens = config_to_token(ifConf);

    std::vector<std::vector<std::string> >::iterator it = tokens.begin();
    std::vector<std::vector<std::string> >::iterator ite = tokens.end();

    while (it != ite)
	{
        if (!it->empty() && it->front() == "server")
		{
			ConfigFile config;
            parse_server(config, it, ite);
			configs.push_back(config);

		}
        else
            it++;
    }
}

int parse_config_file(char *av, std::vector<ConfigFile> &configs)
{
	std::string path = av;

	try 
	{
		parse_config(path, configs);
	}
	catch (configParseException &e)
	{
		std::cerr << e.what() << std::endl;
        return (0);
    }
	return (1);
}
