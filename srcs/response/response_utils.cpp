#include "HTTPResponse.hpp"
#include "Colors.hpp"

std::string int_to_str(int value)
{
    std::ostringstream oss;
    oss << value;
    return (oss.str());
}

std::string get_date(void)
{
    std::time_t now = std::time(0);
    char buf[30];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", localtime(&now));
    return (std::string(buf));
}

bool is_body_allowed(int code)
{
	if (code == 304 || code == 204 || (code < 200 && code > 99 ))
		return (false);
	return (true);
}

bool is_error_code(int code)
{
	if (code > 399 && code < 600)
		return (true);
	return (false);
}

bool is_redirection_code(int code)
{
	if (code > 299 && code < 400)
		return (true);
	return (false);
}

bool is_success_code(int code)
{
	if (code > 199 && code < 300)
		return (true);
	return (false);
}

void print_response(std::vector<char> response)
{
	std::vector<char>::iterator it = response.begin();
	std::vector<char>::iterator ite = response.end();
	if (ite - it > 400)
		ite = it + 400;
	for (; it != ite; it++)
		std::cout << MAGENTA << *it ;
	std::cout << RESET << std::endl;
}


std::string body_type(const std::string &body_path)
{
	size_t pos = body_path.rfind('.');
	if (pos == std::string::npos || pos == 0 || pos == body_path.length() - 1)
		return ("text/plain; charset=UTF-8");

	std::string extension = body_path.substr(pos + 1);

	std::string extensions[] =
	{
		"html", "htm",
		"css",
		"js",
		"json",
		"jpg", "jpeg", "png",
		"pdf"
	};

	std::string types[] =
	{
		"text/html; charset=UTF-8",
		"text/html; charset=UTF-8",
		"text/css; charset=UTF-8",
		"application/javascript",
		"application/json ; charset=UTF-8",
		"image/jpeg",
		"image/jpeg",
		"image/png",
		"application/pdf"
	};

	size_t count = sizeof(extensions) / sizeof(extensions[0]);

	for (size_t i = 0; i < count; i++)
	{
		if (extension == extensions[i])
			return types[i];
	}

	return "text/plain; charset=UTF-8";
}
