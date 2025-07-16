#include "Methods.hpp"

static std::string ext_trim(const std::string& s)
{
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}


std::string extension(const std::string &type)
{
	std::string extension;

	std::string cleaned = ext_trim(type);

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
		"application/pdf",
		"text/plain"
	};

	std::string extensions[] =
		{
			"html", "htm",
			"css",
			"js",
			"json",
			"jpg", "jpeg", "png",
			"pdf",
			"txt"
		};

	size_t count = sizeof(extensions) / sizeof(extensions[0]);

	for (size_t i = 0; i < count; i++)
	{
		if (cleaned == types[i])
			return ("." + extensions[i]);
	}

	return ".bin"; // Content-Type: application/octet-stream
}

static std::string create_name(const std::string &dir_path, const HTTPRequest &request, const multipart &mult)
{
	DIR *dir = opendir(dir_path.c_str());
	if (!dir)
		throw(HTTPException(("could not open directory: " + dir_path + strerror(errno)).c_str(), 500));

	std::string base_name = "file";
	std::string ext = extension(request.headerFields.at("content-type"));

	if (!(mult.content_type.empty() && mult.content_disposition.empty()))
	{
		ext = extension(mult.content_type);

		if (mult.content_disposition.find("filename") != mult.content_disposition.end())
		{
			std::string old_name = mult.content_disposition.at("filename");
			if (!old_name.empty())
			{
				size_t pos = old_name.rfind('.');
				if (pos == std::string::npos)
					ext = extension(mult.content_type);
				else
				{
					ext = valid_extension(old_name);
					base_name = old_name.substr(0, pos);
				}
			}
		}
	}

	std::string new_name = base_name + ext;
	std::cout << YELLOW << "base_name : " << base_name << ext << std::endl;
	std::cout << YELLOW << "create name file name: " << dir_path << " " << new_name << std::endl;

	std::vector<std::string> existing_files;
	struct dirent *input;

	while ((input = readdir(dir)))
	{
		std::string name = input->d_name;
		if (name == "." || name == "..")
			continue;
		existing_files.push_back(name);
	}

	closedir(dir);

	if (std::find(existing_files.begin(), existing_files.end(), new_name) == existing_files.end())
		return (dir_path + new_name);

	int count = 1;
	while (1)
	{
		std::stringstream ss;
		ss << base_name << "_" << count << ext;
		if (std::find(existing_files.begin(), existing_files.end(), ss.str()) == existing_files.end())
			return (dir_path + ss.str());
		count++;
	}
}

HTTPResponse post_file(const std::string &full_path, const ConfigFile &config, const HTTPRequest &request, const std::pair<Session *, bool> &session_info)
{
	struct stat st;

	std::string file_name = full_path;
	if (stat(file_name.c_str(), &st) == 0)
	{
		try
		{
			multipart mult_tmp;
			std::string dir_path = valid_uri(full_path);
			file_name = create_name(dir_path, request, mult_tmp);
		}
		catch (const HTTPException &e)
		{
			std::cerr << e.what() << '\n';
			return (http_response(e.get_code(), full_path, config, request, session_info));
		}
	}

	std::ofstream ofile(file_name.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!ofile.is_open())
		return (http_response(500, full_path, config, request, session_info));

	std::vector<char>::const_iterator it;
	std::vector<char>::const_iterator ite = request.body.end();
	for (it = request.body.begin(); it != ite; it++)
		ofile << *it;

	if (ofile.fail())
		return (http_response(500, full_path, config, request, session_info));

	ofile.close();

	return (http_response(201, full_path, config, request, session_info));
}

HTTPResponse post_file_in_dir(std::string &file_name, const std::string &full_path, const ConfigFile &config, const HTTPRequest &request, const multipart &mult, const std::pair<Session *, bool> &session_info)
{
	if (file_name.empty())
	{
		try
		{
			file_name = create_name(full_path, request, mult);
		}
		catch (const HTTPException &e)
		{
			return (http_response(e.get_code(), full_path, config, request, session_info));
		}
	}
	else
	{
		std::cout << full_path << std::endl;
		tryDeleteProfilePicture(full_path, file_name);
		file_name = full_path + file_name + extension(mult.content_type);
	}

	std::ofstream ofile(file_name.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	if (!ofile.is_open())
		return (http_response(500, full_path, config, request, session_info));

	std::vector<char> body;
	if (!(mult.content_type.empty() && mult.content_disposition.empty()))
	{
		if (!mult.body.empty())
			body = mult.body;
	}
	else
		body = request.body;

	std::vector<char>::const_iterator it;
	std::vector<char>::const_iterator ite = body.end();
	for (it = body.begin(); it != ite; it++)
		ofile << *it;

	if (ofile.fail())
		return (http_response(500, full_path, config, request, session_info));

	ofile.close();

	return (http_response(303, UPLOADS, config, request, session_info));
}

HTTPResponse post_multipart(std::string &full_path, const ConfigFile &config, const HTTPRequest &request, const std::string &boundary, const std::pair<Session *, bool> &session_info)
{
	std::vector<multipart> content;
	try
	{
		content = parse_multipart_body(request, boundary);
	}
	catch (const HTTPException &e)
	{
		return (http_response(e.get_code(), full_path, config, request, session_info));
	}
	catch (const std::exception &e)
	{
		std::cout << RED << e.what() << RESET << std::endl;
		return (http_response(500, full_path, config, request, session_info));
	}

	std::vector<multipart>::iterator it;
	std::vector<multipart>::iterator ite = content.end();
	std::string file_name = "";

	for (it = content.begin(); it != ite; it++)
	{
		if (it->content_disposition.count("filename") == 0)
		{
			if (it->content_disposition.count("name") && it->content_disposition.at("name") == "savename")
				file_name = std::string(it->body.begin(), it->body.end());
			continue;
		}

		if (it->content_disposition.find("name") == it->content_disposition.end())
			return (http_response(404, full_path, config, request, session_info));

		if (it->content_disposition.find("filename") == it->content_disposition.end() && it->body.empty())
			return (http_response(422, full_path, config, request, session_info));

		if (!is_valid_filename(it->content_disposition.at("filename")))
			return (http_response(400, full_path, config, request, session_info));

		if ((it->content_disposition.find("filename") == it->content_disposition.end()) || (it->content_disposition.at("filename").empty() && !it->body.empty()))
		{
			return (post_file_in_dir(file_name, full_path, config, request, *it, session_info));
		}

		std::string path_to_upload = full_path;

		if (full_path[full_path.length() - 1] != '/')
			path_to_upload += "/";

		path_to_upload += it->content_disposition.at("filename");

		struct stat st;
		if (stat(path_to_upload.c_str(), &st) == 0)
		{
			return (post_file_in_dir(file_name, full_path, config, request, *it, session_info));
		}
		else
		{
			return (post_file_in_dir(file_name, full_path, config, request, *it, session_info));
		}
	}
	return (http_response(500, full_path, config, request, session_info));
}

HTTPResponse logout_request(const ConfigFile &config, const std::pair<Session *, bool> &session_info, const HTTPRequest &request)
{
	Session *session = session_info.first;

	session->logout();

	return (http_response(302, LOGOUT, config, request, session_info));

}

HTTPResponse post_request(const ConfigFile &config, const HTTPRequest &request, EMETHODS method, const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + trim_location_root(request.target, config);;

	std::cout << BRIGHT_YELLOW << "POST: full_path: " << full_path << RESET << std::endl;

	struct stat st;

	if (stat(full_path.c_str(), &st) != 0)
		return (http_response(404, full_path, config, request, session_info));

	std::string uri_loc;
	uri_loc = find_location_prefixe(request.target, config);

	if (!is_method_allowed(method, uri_loc, config))
		return (http_response(405, full_path, config, request, session_info));

	if (request.target == "/logout")
		return (logout_request(config, session_info, request));

	if (!is_upload_enabled(uri_loc, config))
		return (http_response(403, full_path, config, request, session_info));

	if (get_content_header_code(request))
		return (http_response(get_content_header_code(request), full_path, config, request, session_info));

	if (request.exceedMaxBodySize)
		return (http_response(413, full_path, config, request, session_info));

	if (S_ISREG(st.st_mode))
	{
		std::string uri;
		uri = valid_uri(full_path);

		if (access(uri.c_str(), W_OK | X_OK))
			return (http_response(403, full_path, config, request, session_info));

		return (post_file(full_path, config, request, session_info));
	}
	else if (S_ISDIR(st.st_mode))
	{
		if (full_path[full_path.length() - 1] != '/')
			return (http_response(307, full_path, config, request, session_info));

		if (access(full_path.c_str(), W_OK) != 0)
			return (http_response(403, full_path, config, request, session_info));

		std::string content_type = request.headerFields.at("content-type");

		size_t pos_multipart = content_type.find("multipart/form-data");
		if (pos_multipart != std::string::npos)
		{
			size_t pos_boundary = content_type.find("boundary=");
			if (pos_boundary == std::string::npos)
				return http_response(400, full_path, config, request, session_info);

			std::string boundary = get_boundary(request);
			if (boundary.empty())
				return http_response(400, full_path, config, request, session_info);

			return (post_multipart(full_path, config, request, boundary, session_info));
		}
		multipart mult_tmp;
		std::string empty_str = "";
		return (post_file_in_dir(empty_str, full_path, config, request, mult_tmp, session_info));
	}
	return (http_response(500, full_path, config, request, session_info));
}
