#include "Delete.hpp"
#include <unistd.h>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdlib>
#include "HTTPResponse.hpp"
#include "Methods.hpp"

void tryDeleteProfilePicture(const std::string& path, const std::string& fileName)
{
	std::vector<std::string> valid_extensions;
	valid_extensions.push_back("jpg");
	valid_extensions.push_back("jpeg");
	valid_extensions.push_back("png");
	valid_extensions.push_back("gif");
	valid_extensions.push_back("webp");

	DIR *dir = opendir(path.c_str());
	if (!dir)
	{
		std::cout << RED
			<< "Failed to open directory: " << path
			<< RESET << std::endl;
		return;
	}

	std::string prefix = fileName + ".";

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string filename = entry->d_name;

		if (filename.compare(0, prefix.size(), prefix) == 0)
		{
			std::string full_path = path + "/" + filename;
			if (std::remove(full_path.c_str()) == 0) {
				std::cout << GREEN
					<< "Removed: " << full_path
					<< RESET << std::endl;
			} else {
				std::cout << RED
					<< "Failed to remove: " << full_path
					<< RESET << std::endl;
			}
		}
	}

	closedir(dir);
}

bool deleteDirectory(const std::string& path) //delete directory recusrsive
{
		DIR* dir = opendir(path.c_str()); //ptr to directory stream et open it (opendir returns a DIR* and is used by readdir to read from)
		if (!dir) //if not exist
			return false;

		struct dirent* entry; //declare a ptr to a dirent struct representing an 1 entry at a time (a placeholder to iterate the direcotry 1 item at a time) inside a directory like a subdirectory, a file etc ==here we'll use the dir ptr
		while ((entry = readdir(dir))) //readdir reads 1 file/item at a time from the directory pointed by dir ptr we opended and save it by returning a dirent* structure that contains the name and type of this item we read, if null directory is fully read
		{
			std::string name = entry->d_name; //save the name of entry

			if (name == "." || name == "..") //exclude current and parent directory
				continue;

			std::string fullPath = path + "/" + name;	//build full path of entry

			struct stat st; //to save info about the file
			if (stat(fullPath.c_str(), &st) == 0)
			{
					if (S_ISDIR(st.st_mode))
						deleteDirectory(fullPath); //recursion if its a directory
					else
						remove(fullPath.c_str()); //delete directly if file
			}
		}
		closedir(dir);
		return rmdir(path.c_str()) == 0; //delete the now empty folder itself
}

std::string resolvePath(const std::string& uri)
{
	std::string raw = "./www" + uri;
	char real[PATH_MAX];
	char www_root[PATH_MAX];

	// resolve full path of target
	if (realpath(raw.c_str(), real) == NULL)
		return ""; // not found path

	// Resolve root path
	if (realpath("./www", www_root) == NULL)
		return "";

	std::string resolved(real);
	std::string root(www_root);

	if (resolved.find(root) != 0)
		return ""; // esc root

	return resolved;
}

HTTPResponse delete_request(const ConfigFile& config, const HTTPRequest& request, EMETHODS method,const std::pair<Session *, bool> &session_info)
{
	std::string full_path = getRootByLocation(request.target, config) + request.target;
	std::string uri_loc = find_location_prefixe(request.target, config); //find at which location the target is

	struct stat st;
	if (stat(full_path.c_str(), &st) != 0)
		return (http_response(404, full_path, config, request, session_info));

	if (!is_method_allowed(method, uri_loc, config)) //check if delete is allowed in config
		return http_response(405, full_path, config, request, session_info);

	// char resolved_path[PATH_MAX];
	// if (!realpath(full_path.c_str(), resolved_path))
	//     return http_response(404, full_path, config, request, session_info); // path doesn't exist

	// std::string uri(resolved_path);

	//or

	// std::string uri = resolvePath(request.target);
	// if (uri.empty())
   	//     return http_response(404, full_path, config, request, session_info);

	// security check
	// if (uri.find(config.root) != 0)
	//     return http_response(403, full_path, config, request, session_info); // esc root dir

	// if (uriNeedsRedirect(request.target, uri))
	//     return http_response(301, full_path, config, request, session_info);

	// if (access(uri.c_str(), W_OK) != 0)
	//     return http_response(403, full_path, config, request, session_info);

	// if (uriNeedsRedirect(uri, request.target))
	// 	return http_response(301, full_path, config, request, session_info);

	struct stat pathStat; //struct holds metadata about the file-path
	if (stat(full_path.c_str(), &pathStat) == 0) //retrieve data about path and writes it in pathstruct
	{
		if (S_ISREG(pathStat.st_mode))
		{
			if (access(full_path.c_str(), W_OK) != 0)
				return http_response(403, full_path, config, request, session_info);
		}
		else if (S_ISDIR(pathStat.st_mode))
		{
			return http_response(409, full_path, config, request, session_info);
		}
	}
	// else
	// 	return http_response(404, full_path, config, request, session_info);

	if (remove(full_path.c_str()) != 0)
		return http_response(500, full_path, config, request, session_info);

	return http_response(204, full_path, config, request, session_info);
}
