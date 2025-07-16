#include "HTTPResponse.hpp"
#include "Methods.hpp"

std::string get_status_message(int code)
{
	switch (code)
	{
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 204:
		return "No Content";
	case 301:
		return "Moved Permanently";
	case 302:
		return "Found";
	case 303:
		return "See Other";
	case 307:
		return "Temporary Redirect";
	case 400:
		return "Bad Request";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 408:
		return "Request Timeout";
	case 413:
		return "Request Entity Too Large";
	case 414:
		return "URI Too Long";
	case 415:
		return "Unsupported Media Type";
	case 500:
		return "Internal Server Error";
	case 503:
		return "Service Unavailable";
	default:
		return "Unknown Status";
	}
}

std::vector<char> generate_error_page(int code)
{
	std::ostringstream html;

	html << "<!DOCTYPE html>" << std::endl;
	html << "<html><head><title>Error " << code << " " << get_status_message(code) << "</title></head>";
	html << "<body><h1> Error " << code << "</h1>";
	html << "<h2>" << get_status_message(code) << "</h2>";
	html << "<a href=\"../\">..</a></body></html>";

	std::vector<char> error_body;
	for (size_t i = 0; i < html.str().length(); i++)
	{
		error_body.push_back(html.str()[i]);
	}
	return (error_body);
}

void set_body(HTTPResponse_data &response, const ConfigFile &config)
{
	if (response.body_path == AUTO_INDEX)
	{
		response.body = generate_auto_index(response.root + response.uri, response.uri);
	}
	else if (is_error_code(response.statusCode))
	{
		if (config.errorPage.find(response.statusCode) != config.errorPage.end())
			response.body = load_body(getRootByLocation(response.uri, config) + "/" + config.errorPage.at(response.statusCode));
		else
			response.body = generate_error_page(response.statusCode);
	}
	else
	{
		if (response.method == PUT || response.method == POST || response.method == DELETE)
			std::vector<char>().swap(response.body);
		else
			response.body = load_body(response.body_path);
	}
}

void set_header(HTTPResponse_data &response)
{
	response.headerFields.insert(std::make_pair("Date", get_date()));

	if (response.body.empty())
	{
		if (response.statusCode != 304)
			response.headerFields.insert(std::make_pair("Content-length", "0"));
	}
	else if (!response.body.empty() && is_body_allowed(response.statusCode))
	{
		response.headerFields.insert(std::make_pair("Content-Length", int_to_str(response.body.size())));
		if (response.body_path == AUTO_INDEX || is_error_code(response.statusCode))
			response.headerFields.insert(std::make_pair("Content-Type", "text/html; charset=UTF-8"));
		else
			response.headerFields.insert(std::make_pair("Content-Type", body_type(response.body_path)));
	}

	if ((response.method == PUT || response.method == POST) && is_success_code(response.statusCode))
		response.headerFields.insert(std::make_pair("Location", response.body_path.substr(4)));

}

std::vector<char> load_body(const std::string &body_path)
{
	std::ifstream file(body_path.c_str(), std::ios::in | std::ios::binary);
	std::vector<char> body;

	if (!file.is_open())
	{
		switch (errno)
		{
			case ENOENT:
				throw(HTTPException(("File not found: " + body_path).c_str(), 404));
			case EACCES:
				throw HTTPException(("Permission denied: " + body_path).c_str(), 403);
			default:
				throw HTTPException((body_path + ": " + strerror(errno)).c_str(), 500);
		}
	}

	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (body_path.size() >= 4 && body_path.substr(body_path.length() - 4) == ".png" && size < 8)
		throw(HTTPException((body_path + " invalid image").c_str(), 415));

	char c;
	while (file.get(c))
		body.push_back(c);

	file.close();
	return (body);
}

std::vector<char> build_response(const HTTPResponse_data &response)
{

	std::string response_str;
	response_str += response.protocol + " " + int_to_str(response.statusCode) + " " + get_status_message(response.statusCode) + "\r\n";

	std::map<std::string, std::string>::const_iterator it;
	std::map<std::string, std::string>::const_iterator ite = response.headerFields.end();
	for (it = response.headerFields.begin(); it != ite; it++)
		response_str += it->first + ": " + it->second + "\r\n";

	response_str += "\r\n";

	std::vector<char> response_vect;
	for (size_t i = 0; i < response_str.length(); i++)
		response_vect.push_back(response_str[i]);

	if (!response.body.empty() && is_body_allowed(response.statusCode) && response.method != HEAD)
		response_vect.insert(response_vect.end(), response.body.begin(), response.body.end());

	return (response_vect);
}

void add_location_header(int status_code, const HTTPRequest &request, HTTPResponse_data &response)
{
	if (status_code == 301 || status_code == 307)
		response.headerFields.insert(std::make_pair("Location", request.target + "/"));
	else if (status_code == 302 && response.body_path == LOGOUT)
		response.headerFields.insert(std::make_pair("Location", "/"));
	else if (status_code == 303 && response.body_path == UPLOADS)
		response.headerFields.insert(std::make_pair("Location", "/docs/profile-preview.html"));

}

void add_cookie_header(HTTPResponse_data &response, Session* session)
{
	if (!session || session->getId().empty())
		return;

	std::string cookie = "session_id=" + session->getId();
	cookie += "; Path=/";
	cookie += "; HttpOnly";
	if (response.body_path == LOGOUT)
		cookie += "; Max-Age=0";
	else
	{
		cookie += "; Max-Age=";
		cookie += SESSION_TIMEOUT;
	}

	response.headerFields["Set-Cookie"] = cookie;
}

HTTPResponse http_response(int code, std::string body_path, const ConfigFile &config, const HTTPRequest &request,const std::pair<Session *, bool> &session)
{
	HTTPResponse_data response;

	response.protocol = "HTTP/1.1";
	response.statusCode = code;
	response.uri = request.target;
	response.root = getRootByLocation(request.target, config);
	response.body_path = body_path;
	response.method = request.method;

	try
	{
		set_body(response, config);
	}
	catch (HTTPException &e)
	{
		if (!is_error_code(response.statusCode) && !is_redirection_code(response.statusCode))
			response.statusCode = e.get_code();

		if (is_error_code(e.get_code()))
			response.body = generate_error_page(e.get_code());
	}

	set_header(response);

	if (is_redirection_code(response.statusCode))
		add_location_header(code, request, response);

	if ((session.first && !request.session_id.empty() && session.second) || body_path == LOGOUT)
		add_cookie_header(response, session.first);

	HTTPResponse response_final;
	response_final.full_response = build_response(response);

	return (response_final);
}

HTTPResponse http_return_response(const HTTPRequest &request, const ConfigFile &config)
{
	HTTPResponse_data response;
	location location = find_location(request.target, config);
	
	response.protocol = "HTTP/1.1";
	response.statusCode = location.returnRedirection.first;
	response.method = request.method;

	response.headerFields.insert(std::make_pair("Date", get_date()));
	response.headerFields.insert(std::make_pair("Location",  location.returnRedirection.second));
	response.headerFields.insert(std::make_pair("Content-Length", "0"));

	HTTPResponse response_final;
	response_final.full_response = build_response(response);

	return (response_final);
}
