/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec_comments.c

	Comment tags represent content that should never appear in the resulting
output.

The tag's content may contain any substring (including newlines) EXCEPT the
closing delimiter.

Comment tags SHOULD be treated as standalone when appropriate.



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

void Test_magnum_spec_comments(CuTest * tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Determine current directory
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));

	// Shift to ../test/partials
	strcat(cwd, "/../test/partials");

	// Inline
	// Comment blocks should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "12345{{! Comment Block! }}67890");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "1234567890", out->str);

	// Multiline
	// Multiline comments should be permitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "12345{{!\n  This is a\n  multi-line comment...\n}}67890\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "1234567890\n", out->str);

	// Standalone
	// All standalone comment lines should be removed.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Begin.\n{{! Comment Block! }}\nEnd.\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "Begin.\nEnd.\n", out->str);

	// Indented Standalone
	// All standalone comment lines should be removed.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Begin.\n  {{! Indented Comment Block! }}\nEnd.\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "Begin.\nEnd.\n", out->str);

	// Standalone Line Endings
	// "\r\n" should be considered a newline for standalone tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|\r\n{{! Standalone Comment }}\r\n|");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "|\r\n|", out->str);

	// Standalone Without Previous Line
	// Standalone tags should not require a newline to precede them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{! I'm Still Standalone }}\n!");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "!", out->str);

	// Standalone Without Newline
	// Standalone tags should not require a newline to follow them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "!\n  {{! I'm Still Standalone }}");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "!\n", out->str);

	// Multiline Standalone
	// All standalone comment lines should be removed.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Begin.\n{{!\nSomething's going on here...\n}}\nEnd.\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "Begin.\nEnd.\n", out->str);

	// Indented Multiline Standalone
	// All standalone comment lines should be removed.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Begin.\n  {{!\n    Something's going on here...\n  }}\nEnd.\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "Begin.\nEnd.\n", out->str);

	// Indented Inline
	// Inline comments should not strip whitespace
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  12 {{! 34 }}\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "  12 \n", out->str);

	// Surrounding Whitespace
	// Comment removal should preserve surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "12345 {{! Comment Block! }} 67890");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "12345  67890", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

