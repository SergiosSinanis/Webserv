#ifndef INVALID_HEADER_EXCEPTION_HPP
#define INVALID_HEADER_EXCEPTION_HPP

#include <stdexcept>
#include <string>

class InvalidHeaderException : public std::runtime_error
{
	public:
		explicit InvalidHeaderException(const std::string& message);
};

#endif 