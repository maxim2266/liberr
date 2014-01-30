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
