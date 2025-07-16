#include "ConfigFile.hpp"

configParseException::configParseException(): _error("Error"){};

configParseException::configParseException(std::string error): _error(error) {}

configParseException::~configParseException() throw() {}

const char *configParseException::what(void) const throw()
{
	return ((this->_error).c_str());
}