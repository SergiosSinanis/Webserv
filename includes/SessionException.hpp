#ifndef SESSION_EXCEPTION_H
#define SESSION_EXCEPTION_H

#include <stdexcept>
#include <string>


class SessionException: public std::exception
{
	public:
		SessionException();
		SessionException(std::string error);
		virtual ~SessionException() throw();
		const char *what() const throw();
	private:
		std::string _error;
};
#endif 