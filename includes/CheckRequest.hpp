#ifndef CHECK_REQUEST_HPP
#define CHECK_REQUEST_HPP

#include "HTTPRequest.hpp"
#include "Colors.hpp"
#include "InvalidHeaderException.hpp"

void checkRequest(HTTPRequest& request);

// 5 methods
void checkGetHeaders(HTTPRequest& request);
void checkPostHeaders(HTTPRequest& request);
void checkPutHeaders(HTTPRequest& request);
void checkDeleteHeaders(HTTPRequest& request);
void checkPatchHeaders(HTTPRequest& request);

bool isNumber(const std::string& str);
bool isValidMimeType(const std::string& mime);
bool isValidConnection(const std::string& value);
bool isValidAcceptEncoding(const std::string& value);
bool isValidAcceptLanguage(const std::string& value);
bool isValidURL(const std::string& value);
bool isValidUpgradeValue(const std::string& value);
bool isValidTransferEncoding(const std::string& value);
bool isValidUserAgent(const std::string& value);
bool isValidAuthorization(const std::string& value);
bool isValidCookieHeader(const std::string& value);
bool isValidMediaTypeEntry(const std::string& entry);
bool isValidQValue(const std::string& params);

float ft_atof(char const *str);

#endif
