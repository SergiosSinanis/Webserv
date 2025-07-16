#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP

#include <string>

class HTTPException : public std::exception
{
    private:
	std::string m_message;
    int m_code;

	public:
	HTTPException(const char * _msg, int _code) : m_message(_msg), m_code(_code) {}
	virtual ~HTTPException() throw() {}
	virtual const char * what() const throw() {return (m_message.c_str());};
    virtual int get_code() const {return (m_code);}
};

#endif
