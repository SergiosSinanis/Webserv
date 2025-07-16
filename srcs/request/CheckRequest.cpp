#include "CheckRequest.hpp"

bool isValidAcceptHeader(const std::string& value) //split the accept media types exemple text/html, application/ etc, split by ','
{
    if (value.empty())
        return false;

    std::string remaining = value;
    size_t pos = 0;

    while (pos != std::string::npos)
	{
        size_t comma_pos = remaining.find(',', pos); //split by ',' eg "text/html" and then pass it to validmediatypeentry
        std::string media_type;

        if (comma_pos != std::string::npos)
		{
            media_type = remaining.substr(pos, comma_pos - pos);
            pos = comma_pos + 1;
        }
		else
		{
            media_type = remaining.substr(pos);
            pos = std::string::npos;
        }

        // whitespace cleaning
        size_t start = media_type.find_first_not_of(" \t");
        size_t end = media_type.find_last_not_of(" \t");
        if (start == std::string::npos)
            continue; // skip empty things

        media_type = media_type.substr(start, end - start + 1);

        if (!isValidMediaTypeEntry(media_type)) //check for 1 media type eg ;q=0.8
            return false;
    }

    return true; //all are valid
}

bool isValidMediaTypeEntry(const std::string& entry) //find ; seperator
{
    size_t semicolon_pos = entry.find(';');
    std::string media_type = (semicolon_pos != std::string::npos) ? entry.substr(0, semicolon_pos) : entry;

    // Trim media type
    size_t start = media_type.find_first_not_of(" \t");
    size_t end = media_type.find_last_not_of(" \t");
    if (start == std::string::npos)
        return false;

    media_type = media_type.substr(start, end - start + 1);

    if (media_type == "*/*")
        return true;

    size_t slash_pos = media_type.find('/');
    if (slash_pos == std::string::npos || slash_pos == 0 || slash_pos == media_type.length() - 1)
        return false;

    std::string type = media_type.substr(0, slash_pos);
    std::string subtype = media_type.substr(slash_pos + 1);

    if (type.find(' ') != std::string::npos || subtype.find(' ') != std::string::npos)
        return false;

    if (!(type == "text" || type == "application" || type == "image" ||
          type == "audio" || type == "video" || type == "multipart" ||
          type == "message" || type == "*"))
        return false;

    if (semicolon_pos != std::string::npos) //param is q value (optional)
	{
        std::string params = entry.substr(semicolon_pos + 1);
        return isValidQValue(params);
    }

    return true;
}

bool isValidQValue(const std::string& params)
{
    size_t q_pos = params.find("q=");
    if (q_pos == std::string::npos)
        return true; // q= param not required always

    size_t value_start = q_pos + 2;
    size_t value_end = params.find_first_of(" ;,", value_start);
    if (value_end == std::string::npos)
        value_end = params.length();

    std::string q_value = params.substr(value_start, value_end - value_start);

    try
	{
        float q = ft_atof(q_value.c_str());
        return q >= 0.0f && q <= 1.0f;
    }
	catch (const std::invalid_argument&) //standard
	{
        return false;
    }
    catch (const std::out_of_range&) //standard
	{
        return false;
	}
}

void checkCommonHeaders(HTTPRequest& request)
{
	if (request.headerFields.find("host") == request.headerFields.end())
		throw InvalidHeaderException("Missing 'Host' header");
	if (request.headerFields["host"].empty())
		throw InvalidHeaderException("'Host' header is empty");

	if (request.headerFields.count("connection") &&
		!isValidConnection(request.headerFields["connection"])) //check if "onnection" key exists in my map, 1 if exists
		throw InvalidHeaderException("'Connection' header has invalid value");

	if (request.headerFields.count("accept") &&
		!isValidAcceptHeader(request.headerFields["accept"]))
		throw InvalidHeaderException("'Accept' header has invalid format");

	if (request.headerFields.count("accept-encoding") &&
		!isValidAcceptEncoding(request.headerFields["accept-encoding"]))
		throw InvalidHeaderException("'Accept-Encoding' has unsupported value");

	if (request.headerFields.count("accept-language") &&
		!isValidAcceptLanguage(request.headerFields["accept-language"]))
		throw InvalidHeaderException("'Accept-Language' is invalid");

	if (request.headerFields.count("referer") &&
		!isValidURL(request.headerFields["referer"]))
		throw InvalidHeaderException("'Referer' URL is invalid");

	if (request.headerFields.count("origin") &&
		!isValidURL(request.headerFields["origin"]))
		throw InvalidHeaderException("'Origin' URL is invalid");

	if (request.headerFields.count("upgrade-insecure-requests") &&
		!isValidUpgradeValue(request.headerFields["upgrade-insecure-requests"]))
		throw InvalidHeaderException("'Upgrade-Insecure-Requests' is invalid");

	if (request.headerFields.count("transfer-encoding") &&
		!isValidTransferEncoding(request.headerFields["transfer-encoding"]))
		throw InvalidHeaderException("'Transfer-Encoding' is not supported");

	if (request.headerFields.count("user-agent") &&
		!isValidUserAgent(request.headerFields["user-agent"]))
		throw InvalidHeaderException("'User-Agent' is invalid");

	if (request.headerFields.count("authorization") &&
		!isValidAuthorization(request.headerFields["authorization"]))
		throw InvalidHeaderException("'Authorization' header is invalid");

	if (request.headerFields.count("cookie") &&
		!isValidCookieHeader(request.headerFields["cookie"]))
		throw InvalidHeaderException("'Cookie' header is invalid");
}

void checkGetHeaders(HTTPRequest& request)
{
	checkCommonHeaders(request);
}

void checkPostHeaders(HTTPRequest& request)
{
	checkCommonHeaders(request);

	if (request.headerFields.count("content-length") != 0)
	{
		if (!isNumber(request.headerFields["content-length"]))
			throw InvalidHeaderException("'Content-Length' is not a valid number");
	}

	if (request.headerFields.count("transfer-encoding") != 0)
	{
		if (request.headerFields["transfer-encoding"] != "chunked")
			throw InvalidHeaderException("'Transfer-encoding' is not a valid argument");
	}

	if (request.headerFields.count("content-length") == 0)
	{
		if (request.headerFields.count("transfer-encoding") == 0)
			throw InvalidHeaderException("Missing 'Content-Length | Transfer-Encoding' header");
	}


	if (request.headerFields.count("content-type") == 0)
		throw InvalidHeaderException("Missing 'Content-Type' header");

	if (!isValidMimeType(request.headerFields["content-type"]))
		throw InvalidHeaderException("'Content-Type' is not a valid MIME type");
}

void checkPutHeaders(HTTPRequest& request)
{
	checkPostHeaders(request);
}

void checkPatchHeaders(HTTPRequest& request)
{
	checkPostHeaders(request);
}

void checkDeleteHeaders(HTTPRequest& request)
{
	checkCommonHeaders(request);
}

void checkRequest(HTTPRequest& request)
{
	switch (request.method)
	{
		case GET:
			checkGetHeaders(request);
			break;
		case HEAD:
			checkGetHeaders(request);
			break;
		case POST:
			checkPostHeaders(request);
			break;
		case PUT:
			checkPutHeaders(request);
			break;
		case PATCH:
			checkPatchHeaders(request);
			break;
		case DELETE:
			checkDeleteHeaders(request);
			break;
		default:
			throw InvalidHeaderException("Unknown HTTP method");
	}
}
