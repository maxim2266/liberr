#include "errors.h"

#include <errno.h>
#include <error.h>
#include <stdio.h>

// test framework :)
static
bool run_test(void (*f)())
try
{
	f();
	return true;
}
catch(const std::exception& e)
{
	printf("Exception: %s\n", e.what());
	return false;
}

#define ASSERT_PASSED(test)	if(!run_test(test)) error(1, 0, "### Test failed: " #test)
#define ASSERT_FAILED(test)	if(run_test(test)) error(1, 0, "### Test failed: " #test)

// tests
static
void test_string()
{
	error_type::raise("simple string");
}

static
void test_format()
{
	error_type::raise_fmt("Application name is \"%s\"", program_invocation_name);
}

static
void test_errno()
{
	const int err = ENOSPC;
	
	error_type::raise_fmt_errno(err, "errno (%d)", err);
}

static
void test_reraise()
{
	try
	{
		test_errno();
	}
	catch(const std::exception& e)
	{
		error_type::reraise(e, "Rethrown, original exception:");
	}
}

static
void test_reraise_fmt()
{
	try
	{
		test_reraise();
	}
	catch(const std::exception& e)
	{
		error_type::reraise_fmt(e, "%s: Rethrown again, original exception:", program_invocation_name);
	}
}

// entry point
int main()
{
	ASSERT_FAILED(test_string);
	ASSERT_FAILED(test_format);
	ASSERT_FAILED(test_errno);
	ASSERT_FAILED(test_reraise_fmt);
	
	puts("All passed.");
	return 0;
}
