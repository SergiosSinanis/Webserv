#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Methods.hpp"


HTTPResponse patch_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method,const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + request.target;

	std::string uri_loc = find_location_prefixe(request.target, config);

	if (!is_method_allowed(method, uri_loc, config))
		return http_response(405, full_path, config, request, session_info);

	struct stat st;
	if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
		return http_response(409, full_path, config, request, session_info);

	if (request.headerFields.find("content-length") == request.headerFields.end())
		return http_response(411, full_path, config, request, session_info);

	const std::string &cl = request.headerFields.at("content-length");

	for (size_t i = 0; i < cl.length(); ++i)
	{
		if (!isdigit(cl[i]))
			return http_response(400, full_path, config, request, session_info);
	}

	if (request.exceedMaxBodySize)
		return http_response(413, full_path, config, request, session_info);

	if (stat(full_path.c_str(), &st) != 0)
		return http_response(404, full_path, config, request, session_info);

	if (access(full_path.c_str(), W_OK) != 0)
		return http_response(403, full_path, config, request, session_info);

	// append to patch
	std::ofstream outfile(full_path.c_str(), std::ios::out | std::ios::app | std::ios::binary);
	if (!outfile.is_open())
		return http_response(500, full_path, config, request, session_info);

	outfile.write(request.body.data(), request.body.size());
	if (outfile.fail())
		return http_response(500, full_path, config, request, session_info);
	outfile.close();

	return http_response(200, full_path, config, request, session_info);
}
