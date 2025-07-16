#include "Methods.hpp"

HTTPResponse put_file(const std::string &full_path, const ConfigFile &config, const HTTPRequest &request,const std::pair<Session *, bool> &session_info)
{
	struct stat st;
	int code;

	if (stat(full_path.c_str(), &st) == 0)
		code = 200;
	else
		code = 201;

	std::ofstream ofile(full_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!ofile.is_open())
		return (http_response(500, full_path, config, request, session_info));

	std::vector<char>::const_iterator it;
	std::vector<char>::const_iterator ite = request.body.end();
	for (it = request.body.begin(); it != ite; it++)
		ofile << *it;

	if (ofile.fail())
		return (http_response(500, full_path, config, request, session_info));

	ofile.close();

	return (http_response(code, full_path, config, request, session_info));
}

HTTPResponse put_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + trim_location_root(request.target, config);;

	std::string uri;
	uri = valid_uri(full_path);

	struct stat st;
	if (stat((uri).c_str(), &st) != 0)
		return (http_response(404, full_path, config, request, session_info));

	std::string uri_loc;
	uri_loc = find_location_prefixe(request.target, config);

	if (!is_method_allowed(method, uri_loc, config))
		return (http_response(405, full_path, config, request, session_info));

	if (!is_upload_enabled(uri_loc, config))
		return (http_response(403, full_path, config, request, session_info));

	if (uri.empty())
		return (http_response(409, full_path, config, request, session_info));

	if (get_content_header_code(request))
		return (http_response(get_content_header_code(request), full_path, config, request, session_info));

	if (request.exceedMaxBodySize)
		return (http_response(413, full_path, config, request, session_info));

	if (access(uri.c_str(), W_OK) != 0)
	{
		if (errno == ENOENT)
			return (http_response(404, full_path, config, request, session_info));
		return (http_response(403, full_path, config, request, session_info));
	}

	return (put_file(full_path, config, request, session_info));
}
