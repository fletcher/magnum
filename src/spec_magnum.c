/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec_magnum.c

	Tests specific to magnum, as opposed to the general Mustache spec.


	@brief Bootstrap test suite from https://github.com/mustache/spec


	@author	Fletcher T. Penney
	@bug


**/

/*

	Original Code Copyright © 2017-2018 Fletcher T. Penney.

	## The MIT License ##

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

*/


#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "d_string.h"
#include "libMagnum.h"


#ifdef TEST
#include "CuTest.h"

void Test_magnum_spec_magnum(CuTest * tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Determine current directory
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));

	// Shift to ../test/partials
	strcat(cwd, "/../test/partials");

	// Literal JSON
	// '$' indicates that the template should should be replaced with the raw JSON.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{$person}}");
	magnum_populate_from_string(source, "{\"person\":{\"name\":\"John Doe\",\"age\":35,\"title\":\"King of the World\"}}", out, cwd);
	CuAssertStrEquals(tc, "{\\\"name\\\":\\\"John Doe\\\",\\\"age\\\":35,\\\"title\\\":\\\"King of the World\\\"}", out->str);

	// Escaped tag names
	// ':' indicates that the rest of the tag name should be used literally.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{:>text}}");
	magnum_populate_from_string(source, "{\">text\":\"templated text\"}", out, cwd);
	CuAssertStrEquals(tc, "templated text", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

