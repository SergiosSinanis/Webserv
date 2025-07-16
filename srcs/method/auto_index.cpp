#include "Methods.hpp"

std::vector<char> generate_auto_index(const std::string &path, const std::string &uri)
{
	std::vector<std::string> files;

	DIR *dir = opendir(path.c_str());
	if (!dir)
	{
		if (!dir)
		{
			switch(errno)
			{
				case (EACCES):
					throw (HTTPException("Permission denied", 403));
				case (ENOENT):
					throw HTTPException("Directory not found", 404);
				default:
					throw HTTPException(strerror(errno), 500);
			}
		}
	}

	struct dirent *input;

	while ((input = readdir(dir)))
	{
		std::string file_name;
		file_name = input->d_name;
		if (file_name == "." || file_name == "..")
			continue;
		files.push_back(file_name);
	}
	closedir(dir);

	std::sort(files.begin(), files.end());

	std::ostringstream html;

	html << "<!DOCTYPE html>" << std::endl;
	html << "<html><head><title>Index of " << uri << "</title></head>";
	html << "<body><h1>Index of " << uri << "</h1><hr><ul>";

	if (uri != "/")
		html << "<li><a href=\"../\">..</a></li>";

	std::vector<std::string>::iterator it;
	std::vector<std::string>::iterator ite = files.end();

	for (it = files.begin(); it != ite; it++)
	{
		html << "<li><a href =\"" << *it << "\">" << *it << "</a></li>\n";
	}

	html << "</ul><hr></body></html>";

	std::vector<char> autoindex_body;
	for (size_t i = 0; i < html.str().length(); i++)
	{
		autoindex_body.push_back(html.str()[i]);
	}
	return (autoindex_body);
}
