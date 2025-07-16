#include "Methods.hpp"

HTTPResponse get_request_file(const ConfigFile &config, const HTTPRequest &request,const std::pair<Session *, bool> &session_info)
{
	std::string path = getRootByLocation(request.target, config) + trim_location_root(request.target, config);

	if (access(path.c_str(), R_OK) == -1)
		return (http_response(403, path, config, request, session_info));
	return (http_response(200, path, config, request, session_info));
}

HTTPResponse get_request_dir(const ConfigFile &config, const HTTPRequest &request,const std::pair<Session *, bool> &session_info)
{
	std::string path = config.root + request.target;

	std::vector<location>::const_iterator it;
	std::vector<location>::const_iterator ite = config.locations.end();

	std::string uri = find_location_prefixe(request.target, config);
	if (uri.length() > 1)
		uri = uri.substr(0, uri.length() - 1);

	for (it = config.locations.begin(); it != ite; it++)
	{
		if (uri == it->locationDir)
		{


			path = getRootByLocation(request.target, config) + trim_location_root(request.target, config);

			std::string index_path = find_index(it->index, path);
			if (!index_path.empty())
				return (http_response(200, index_path, config, request, session_info));

			std::string global_index_path = find_index(config.index, path);

			if (!global_index_path.empty())
				return (http_response(200, global_index_path, config, request, session_info));

			if (!it->autoindex.empty() && it->autoindex == "on")
				return (http_response(200, AUTO_INDEX, config, request, session_info));

			return (http_response(404, path, config, request, session_info));
		}
	}
	return (http_response(403, path, config, request, session_info));
}

HTTPResponse get_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + trim_location_root(request.target, config);

	struct stat st;

	if (stat(full_path.c_str(), &st) != 0)
		return (http_response(404, full_path, config, request, session_info));

	std::string uri_loc;
	if (S_ISREG(st.st_mode))
	{
		uri_loc = find_location_prefixe(request.target, config);
		if (!is_method_allowed(method, uri_loc, config))
			return (http_response(405, full_path, config, request, session_info));
		return (get_request_file(config, request, session_info));
	}
	else if (S_ISDIR(st.st_mode))
	{
		if (full_path[full_path.length() - 1] != '/')
			return (http_response(301, full_path, config, request, session_info));

		uri_loc = find_location_prefixe(request.target, config);

		if (!is_method_allowed(method, uri_loc, config))
			return (http_response(405, full_path, config, request, session_info));

		return (get_request_dir(config, request, session_info));
	}
	return (http_response(500, full_path, config, request, session_info));
}
