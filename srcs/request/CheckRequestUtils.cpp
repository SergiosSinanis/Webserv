#include <cfloat>

#include "CheckRequest.hpp"

bool isNumber(const std::string& str)
{
	for (size_t i = 0; i < str.length(); i++)
		if (!std::isdigit(str[i]))
			return false;
	return !str.empty();
}

bool isValidMimeType(const std::string& mime)
{
	return mime.find('/') != std::string::npos;
}

bool isValidConnection(const std::string& value)
{
	return value == "keep-alive" || value == "close";
}

bool isValidAcceptEncoding(const std::string& value)
{
	return value.find("gzip") != std::string::npos ||
		   value.find("deflate") != std::string::npos ||
		   value.find("br") != std::string::npos ||
		   value.find("zstd") != std::string::npos;
}

bool isValidAcceptLanguage(const std::string& value)
{
	return !value.empty();
}

bool isValidURL(const std::string& value)
{
	return !value.empty();
}

bool isValidUpgradeValue(const std::string& value)
{
	return value == "1";
}

bool isValidTransferEncoding(const std::string& value)
{
	return value == "chunked";
}

bool isValidUserAgent(const std::string& value)
{
	return !value.empty();
}

bool isValidAuthorization(const std::string& value)
{
	return !value.empty();
}

bool isValidCookieHeader(const std::string& value)
{
	return !value.empty() && value.find('=') != std::string::npos;
}

float ft_atof(char const *str)
{
	float result = 0;
	float decimal = 0;
	float weight = 1;
	bool negate;

	if (str == NULL || *str == '\0')
		throw std::invalid_argument("null or empty string argument");
	while (std::isspace(*str))
		++str;
	negate = (*str == '-');
	if (*str == '+' || *str == '-')
		++str;
	if (*str == '\0')
		throw std::invalid_argument("sign character only.");
	while (*str)
	{
		if (*str < '0' || *str > '9')
		{
			if (*str == '.')
			{
				++str;
				break;
			}
			while (std::isspace(*str))
				++str;
			if (*str)
				throw std::invalid_argument("invalid input string");
			else
				break;
		}
		result = result * 10 + (*str - '0');
		++str;
	}

	while (*str)
	{
		if (*str < '0' || *str > '9')
		{
			if (weight == 0)
				throw std::invalid_argument("invalid input string");

			while (std::isspace(*str))
				++str;

			if (*str)
				throw std::invalid_argument("invalid input string");
			else
				break;
		}
		decimal = decimal * 10 + (*str - '0');
		weight *= 10;
		++str;
	}
	decimal /= weight;
	return negate ? -(result + decimal) : (result + decimal);
}
