/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec_partials.c

	Partial tags are used to expand an external template into the current
template.

The tag's content MUST be a non-whitespace character sequence NOT containing
the current closing delimiter.

This tag's content names the partial to inject.  Set Delimiter tags MUST NOT
affect the parsing of a partial.  The partial MUST be rendered against the
context stack local to the tag.  If the named partial cannot be found, the
empty string SHOULD be used instead, as in interpolations.

Partial tags SHOULD be treated as standalone when appropriate.  If this tag
is used standalone, any whitespace preceding the tag should treated as
indentation, and prepended to each line of the partial before rendering.



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

void Test_magnum_spec_partials(CuTest * tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Determine current directory
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));

	// Shift to ../test/partials
	strcat(cwd, "/../test/partials");

	// Basic Behavior
	// The greater-than operator should expand to the named partial.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{>text1}}\"");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "\"from partial\"", out->str);

	// Failed Lookup
	// The empty string should be used when the named partial is not found.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{>text2}}\"");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// Context
	// The greater-than operator should operate within the current context.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{>partial1}}\"");
	magnum_populate_from_string(source, "{\"text\":\"content\"}", out, cwd);
	CuAssertStrEquals(tc, "\"*content*\"", out->str);

	// Recursion
	// The greater-than operator should properly recurse.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{>node1}}");
	magnum_populate_from_string(source, "{\"content\":\"X\",\"nodes\":[{\"content\":\"Y\",\"nodes\":[]}]}", out, cwd);
	CuAssertStrEquals(tc, "X<Y<>>", out->str);

	// Surrounding Whitespace
	// The greater-than operator should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{>partial2}} |");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "| \t|\t |", out->str);

	// Inline Indentation
	// Whitespace should be left untouched.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{data}}  {{> partial3}}\n");
	magnum_populate_from_string(source, "{\"data\":\"|\"}", out, cwd);
	CuAssertStrEquals(tc, "  |  >\n>\n", out->str);

	// Standalone Line Endings
	// "\r\n" should be considered a newline for standalone tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|\r\n{{>partial4}}\r\n|");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "|\r\n>|", out->str);

	// Standalone Without Previous Line
	// Standalone tags should not require a newline to precede them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{>partial5}}\n>");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "  >\n  >>", out->str);

	// Standalone Without Newline
	// Standalone tags should not require a newline to follow them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, ">\n  {{>partial6}}");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, ">\n  >\n  >", out->str);

	// Standalone Indentation
	// Each line of the partial should be indented before rendering.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\\\n {{>partial7}}\n/\n");
	magnum_populate_from_string(source, "{\"content\":\"<\\n->\"}", out, cwd);
	CuAssertStrEquals(tc, "\\\n |\n <\n->\n |\n/\n", out->str);

	// Padding Whitespace
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{> partial8 }}|");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "|[]|", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

