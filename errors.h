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
struct yv_exception_type : std::exception
{
	// std::exception required interface
	explicit yv_exception_type(const std::string& msg);
    virtual ~yv_exception_type() throw();

    virtual const char* what() const throw();
	
    // exception throwing functions
	static void raise(const text_arg_type msg) __attribute__((noinline, noreturn));
	static void raise_fmt(const char* fmt, ...) __attribute__((noinline, noreturn, format(printf, 1, 2)));
	static void raise_fmt_errno(int err, const char* fmt, ...) __attribute__((noinline, noreturn, format(printf, 2, 3)));
	static void reraise(const std::exception& e, const text_arg_type prefix) __attribute__((noinline, noreturn));
	static void reraise_fmt(const std::exception& e, const char* fmt, ...) __attribute__((noinline, noreturn, format(printf, 2, 3)));

private:
	std::string msg_;
	
	yv_exception_type(const text_arg_type prefix, const text_arg_type sep, const text_arg_type suffix);
	yv_exception_type(const text_arg_type msg);
	
	static void reraise_impl(const text_arg_type prefix, const text_arg_type suffix) __attribute__((noreturn));
};
	


