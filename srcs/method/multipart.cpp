#include "Methods.hpp"

std::string get_boundary(const HTTPRequest &request)
{
	std::string boundary;

	std::istringstream content_type(request.headerFields.at("content-type"));

	getline(content_type, boundary, '=');
	getline(content_type, boundary);

	trim_unquote(boundary);

	return (boundary);
}
std::vector<std::vector<char> > group_by_line(const std::vector<char> &body)
{
    std::vector<std::vector<char> > res;
    std::vector<char> tmp;

    for (size_t i = 0; i < body.size(); ++i) {
        if (i + 1 < body.size() && body[i] == '\r' && body[i + 1] == '\n')
		{
            res.push_back(tmp);
            tmp.clear();
            i++;
        }
		else
            tmp.push_back(body[i]);
    }
    if (!tmp.empty())
        res.push_back(tmp);
    return res;
}

void trim_unquote(std::string &str)
{
	int start = 0;
	size_t pos = str.find('\"');
	if (pos != std::string::npos)
		start = pos + 1;
	while (str[start] == ' ')
		start++;

	int end = str.length() - 1;
	pos = str.rfind('\"');
	if (pos != std::string::npos)
		end = pos - 1;
	while (str[end] == ' ')
		end--;
	str = str.substr(start, end - start + 1);
}
void extract_key_val(multipart &file, const std::string &data)
{
	std::string key;
	std::string val;
	if (!data.empty())
	{
		std::istringstream field(data);
		getline(field, key, '=');
		getline(field, val);

		trim_unquote(key);
		trim_unquote(val);

		if (key == "filename" && !is_valid_filename(val))
			throw(HTTPException("multipart: invalid file name", 400));

		file.content_disposition.insert(std::make_pair(key, val));
	}
}

multipart parse_part(std::vector<std::vector<char> >::iterator &it, const std::string &start_boundary,
					 const std::string &end_boundary)
{
	it++;
	multipart file;

	while (!it->empty())
	{
		std::string line_str(it->begin(), it->end());
		std::string header;
		std::istringstream line(line_str);
		getline(line, header, ':');
		if (header == "Content-Disposition")
		{
			std::string line_str(it->begin(), it->end());
			if (line_str == start_boundary || line_str == end_boundary)
				break;

			std::string mime;
			getline(line, mime, ';');
			trim_unquote(mime);

			if (mime != "form-data")
				throw(HTTPException("MIME type not accepted", 400));

			std::string data;
			while (getline(line, data, ';'))
				extract_key_val(file, data);

			getline(line, data);
			extract_key_val(file, data);

			if (file.content_disposition.empty())
				throw(HTTPException("Missing data", 400));
		}
		else if (header == "Content-Type")
		{
			std::string content;
			getline(line, content);
			trim_unquote(content);
			file.content_type = content;
		}
		else
			break;

		it++;
	}
	return (file);
}

void fill_body(multipart &file, const std::vector<std::vector<char> > &body, std::vector<std::vector<char> >::iterator &it,
               const std::string &start_boundary, const std::string &end_boundary)
{
    it++;
    while (it != body.end())
    {
        std::string line_str(it->begin(), it->end());
        if (line_str == end_boundary || line_str == start_boundary)
            break;

        std::vector<std::vector<char> >::const_iterator next_it = it;
        next_it++;

        file.body.insert(file.body.end(), it->begin(), it->end());

        if (next_it != body.end())
        {
            std::string next_line_str(next_it->begin(), next_it->end());
            if (next_line_str != end_boundary && next_line_str != start_boundary)
            {
                file.body.push_back('\r');
                file.body.push_back('\n');
            }
        }

        it++;
    }
}

std::vector<multipart> parse_multipart_body(const HTTPRequest &request, const std::string &boundary)
{
	std::vector<multipart> res;

	std::string start_boundary = "--" + boundary;
	std::string end_boundary = "--" + boundary + "--";

	std::vector<std::vector<char> > body = group_by_line(request.body);
	std::vector<std::vector<char> >::iterator it = body.begin();
	std::vector<std::vector<char> >::iterator ite = body.end();

	while (it != ite)
	{
		std::string line_str(it->begin(), it->end());

		if (line_str == end_boundary)
			break;

		if (line_str == start_boundary)
		{
			multipart file = parse_part(it, start_boundary, end_boundary);
			fill_body(file, body, it, start_boundary, end_boundary);
			res.push_back(file);
		}
		else
			++it;
	}
	std::string line_str(it->begin(), it->end());
	if (line_str != end_boundary)
		throw(HTTPException("multipart: missing end boundary", 400));
	return (res);
}

void print_multipart(const std::vector<multipart> &mult)
{
	std::vector<multipart>::const_iterator it;
	std::vector<multipart>::const_iterator ite = mult.end();
	for (it = mult.begin(); it != ite; it++)
	{
		if (!it->content_type.empty())
			std::cout << "content-type: " << it->content_type << std::endl;

		if (!it->content_disposition.empty())
		{
			std::cout << "content-disposition: ";
			for (std::map<std::string, std::string>::const_iterator itm = it->content_disposition.begin();
				 itm != it->content_disposition.end(); itm++)
				std::cout << itm->first << ": " << itm->second << "  ";
			std::cout << std::endl;
		}
		if (!it->body.empty())
		{
			std::cout << "body: ";
			print_vector<char>(it->body);
			std::cout << std::endl;
		}
	}
}
