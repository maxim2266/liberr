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

#pragma once
#pragma GCC diagnostic error "-Wformat"

#include <exception>
#include <string>

// text argument type
struct text_arg_type
{
	const char* str;
	size_t size;
	
	text_arg_type(const char* s);
	text_arg_type(const std::string& s);
	text_arg_type(const char* s, size_t n) __attribute__((nonnull(2)));	// nonnull(2), because the first attribute is 'this', see http://gcc.gnu.org/ml/gcc/2006-04/msg00549.html
};

#define CONST_STRING_ARG(s)	text_arg_type((s), sizeof(s) - 1)

// exception type
struct error_type : std::exception
{
	// std::exception required interface
	explicit error_type(const std::string& msg);
    virtual ~error_type() throw();

    virtual const char* what() const throw();
	
    // exception throwing functions
	static void raise(const text_arg_type msg) __attribute__((noinline, noreturn));
	static void raise_fmt(const char* fmt, ...) __attribute__((noinline, noreturn, format(printf, 1, 2)));
	static void raise_fmt_errno(int err, const char* fmt, ...) __attribute__((noinline, noreturn, format(printf, 2, 3)));
	static void reraise(const std::exception& e, const text_arg_type prefix) __attribute__((noinline, noreturn));
	static void reraise_fmt(const std::exception& e, const char* fmt, ...) __attribute__((noinline, noreturn, format(printf, 2, 3)));

private:
	std::string msg_;
	
	error_type(const text_arg_type prefix, const text_arg_type sep, const text_arg_type suffix);
	error_type(const text_arg_type msg);
	
	static void reraise_impl(const text_arg_type prefix, const text_arg_type suffix) __attribute__((noreturn));
};
	


