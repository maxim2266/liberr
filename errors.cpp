/*
Copyright (c) 2014, Maxim Konakov
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
error_type::error_type(const std::string& msg) 
: msg_(msg)
{}

error_type::error_type(const text_arg_type msg)
: msg_(msg.str, msg.size)
{}

error_type::error_type(const text_arg_type prefix, const text_arg_type sep, const text_arg_type suffix)
{
	const size_t n = prefix.size + sep.size + suffix.size;
	
	if(n > 0)
	{
		msg_.resize(n);
		mempcpy(mempcpy(mempcpy((void*)&msg_[0], prefix.str, prefix.size), sep.str, sep.size), suffix.str, suffix.size);
	}
}

error_type::~error_type() throw()
{}

const char* error_type::what() const throw()
{
	return msg_.c_str();
}

void error_type::reraise_impl(const text_arg_type prefix, const text_arg_type suffix)
{
	if(suffix.size == 0)
		error_type::raise(prefix);
	else
		throw error_type(prefix, CONST_STRING_ARG("\n\t"), suffix);
}

void error_type::raise(const text_arg_type msg)
{
	throw error_type(msg);
}

void error_type::raise_fmt(const char* fmt, ...)
{
	char buff[MSG_BUFF_SIZE];
	va_list args;
	
	va_start(args, fmt);
	error_type::raise(text_arg_type(buff, format_message(buff, fmt, args)));
}

void error_type::raise_fmt_errno(int err, const char* fmt, ...)
{
	char buff[MSG_BUFF_SIZE];
	va_list args;
	
	va_start(args, fmt);
	
	const text_arg_type prefix(buff, format_message(buff, fmt, args));
	
	if(err == 0)
		error_type::raise(prefix);
	else
		throw error_type(prefix, CONST_STRING_ARG(": "), errno_text(err));
}

void error_type::reraise(const std::exception& e, const text_arg_type prefix)
{
	reraise_impl(prefix, e.what());
}

void error_type::reraise_fmt(const std::exception& e, const char* fmt, ...)
{
	char buff[MSG_BUFF_SIZE];
	va_list args;
	
	va_start(args, fmt);	
	reraise_impl(text_arg_type(buff, format_message(buff, fmt, args)), e.what());
}

