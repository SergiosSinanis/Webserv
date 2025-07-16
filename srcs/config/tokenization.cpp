#include "ConfigFile.hpp"

std::vector<std::vector<std::string> > group_token_by_line(std::vector<std::string> token)
{
	std::vector<std::string>::iterator it;
	std::vector<std::string>::iterator ite = token.end();

	std::vector<std::vector<std::string> > res;
	std::vector<std::string> tmp;
	for (it = token.begin(); it != ite; it++)
	{
		if (*it == "\n")
		{
			res.push_back(tmp);
			tmp.clear();
		}
		else
			tmp.push_back(*it);
	}
	return (res);
}

std::vector<std::string> tokenize_line(const std::string &line) 
{
    std::vector<std::string> tokens;
    std::string current;

    for (size_t i = 0; i < line.size(); ++i) 
	{
        char c = line[i];

        if (c == '{' || c == '}' || c == ';') 
		{
            if (!current.empty()) 
			{
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(std::string(1, c));
        }
        else if (isspace(c)) 
		{
            if (!current.empty()) 
			{
                tokens.push_back(current);
                current.clear();
            }
        }
        else 
            current += c;
    }

    if (!current.empty()) 
	{
        tokens.push_back(current);
    }
    return (tokens);
}

std::vector<std::vector<std::string> > config_to_token(std::ifstream &ifConf)
{
    std::vector<std::string> tokens;

    std::string line;
    while (std::getline(ifConf, line))
    {
        std::vector<std::string> lineTokens = tokenize_line(line);
        for (size_t i = 0; i < lineTokens.size(); ++i) {
            tokens.push_back(lineTokens[i]);
        }
        tokens.push_back("\n"); 
    }

    return (group_token_by_line(tokens));
}



