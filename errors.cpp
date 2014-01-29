#include "errors.h"

#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>

#define MSG_BUFF_SIZE 2048

// text argument class
text_arg_type::text_arg_type(const char* s)
: str(s ? s : "")
, size(s ? strlen(s) : 0)
{}

text_arg_type::text_arg_type(const std::string& s)
: str(s.c_str())
, size(s.size())
{}

text_arg_type::text_arg_type(const char* s, size_t n)
: str(s)
, size(n)
{}

// utility functions
static
const char* errno_text(int err)
{
	static __thread char buff[300];
	
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
	// XSI-compliant version of strerror_r()
	if(strerror_r(err, buff, sizeof(buff)) != 0)
		snprintf(buff, sizeof(buff), "Unknown error code %d", err);
	
	return buff;
#else
	// GNU-specific version of strerror_r()
	return strerror_r(err, buff, sizeof(buff));	
#endif
}

static
size_t format_message(char* buff, const char* fmt, va_list args)
{
	const int n = vsnprintf(buff, MSG_BUFF_SIZE, fmt, args);
	
	va_end(args);
	
	if(n >= 0)
		return (size_t)std::min(n, MSG_BUFF_SIZE - 1);
	
	// invalid format
	static const char msg[] = "[Invalid error message format]";
	
	memcpy(buff, msg, sizeof(msg));	// including '\0'
	return sizeof(msg) - 1;
}

// exception type
yv_exception_type::yv_exception_type(const std::string& msg) 
: msg_(msg)
{}

yv_exception_type::yv_exception_type(const text_arg_type msg)
: msg_(msg.str, msg.size)
{}

yv_exception_type::yv_exception_type(const text_arg_type prefix, const text_arg_type sep, const text_arg_type suffix)
{
	const size_t n = prefix.size + sep.size + suffix.size;
	
	if(n > 0)
	{
		msg_.resize(n);
		mempcpy(mempcpy(mempcpy((void*)&msg_[0], prefix.str, prefix.size), sep.str, sep.size), suffix.str, suffix.size);
	}
}

yv_exception_type::~yv_exception_type() throw()
{}

const char* yv_exception_type::what() const throw()
{
	return msg_.c_str();
}

void yv_exception_type::reraise_impl(const text_arg_type prefix, const text_arg_type suffix)
{
	if(suffix.size == 0)
		yv_exception_type::raise(prefix);
	else
		throw yv_exception_type(prefix, CONST_STRING_ARG("\n\t"), suffix);
}

void yv_exception_type::raise(const text_arg_type msg)
{
	throw yv_exception_type(msg);
}

void yv_exception_type::raise_fmt(const char* fmt, ...)
{
	char buff[MSG_BUFF_SIZE];
	va_list args;
	
	va_start(args, fmt);
	yv_exception_type::raise(text_arg_type(buff, format_message(buff, fmt, args)));
}

void yv_exception_type::raise_fmt_errno(int err, const char* fmt, ...)
{
	char buff[MSG_BUFF_SIZE];
	va_list args;
	
	va_start(args, fmt);
	
	const text_arg_type prefix(buff, format_message(buff, fmt, args));
	
	if(err == 0)
		yv_exception_type::raise(prefix);
	else
		throw yv_exception_type(prefix, CONST_STRING_ARG(": "), errno_text(err));
}

void yv_exception_type::reraise(const std::exception& e, const text_arg_type prefix)
{
	reraise_impl(prefix, e.what());
}

void yv_exception_type::reraise_fmt(const std::exception& e, const char* fmt, ...)
{
	char buff[MSG_BUFF_SIZE];
	va_list args;
	
	va_start(args, fmt);	
	reraise_impl(text_arg_type(buff, format_message(buff, fmt, args)), e.what());
}

