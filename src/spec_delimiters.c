/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec_delimiters.c

	Set Delimiter tags are used to change the tag delimiters for all content
following the tag in the current compilation unit.

The tag's content MUST be any two non-whitespace sequences (separated by
whitespace) EXCEPT an equals sign ('=') followed by the current closing
delimiter.

Set Delimiter tags SHOULD be treated as standalone when appropriate.



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

void Test_magnum_spec_delimiters(CuTest * tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Determine current directory
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));

	// Shift to ../test/partials
	strcat(cwd, "/../test/partials");

	// Pair Behavior
	// The equals sign (used on both sides) should permit delimiter changes.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{=<% %>=}}(<%text%>)");
	magnum_populate_from_string(source, "{\"text\":\"Hey!\"}", out, cwd);
	CuAssertStrEquals(tc, "(Hey!)", out->str);

	// Special Characters
	// Characters with special meaning regexen should be valid delimiters.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "({{=[ ]=}}[text])");
	magnum_populate_from_string(source, "{\"text\":\"It worked!\"}", out, cwd);
	CuAssertStrEquals(tc, "(It worked!)", out->str);

	// Sections
	// Delimiters set outside sections should persist.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "[\n{{#section}}\n  {{data}}\n  |data|\n{{/section}}\n\n{{= | | =}}\n|#section|\n  {{data}}\n  |data|\n|/section|\n]\n");
	magnum_populate_from_string(source, "{\"section\":true,\"data\":\"I got interpolated.\"}", out, cwd);
	CuAssertStrEquals(tc, "[\n  I got interpolated.\n  |data|\n\n  {{data}}\n  I got interpolated.\n]\n", out->str);

	// Inverted Sections
	// Delimiters set outside inverted sections should persist.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "[\n{{^section}}\n  {{data}}\n  |data|\n{{/section}}\n\n{{= | | =}}\n|^section|\n  {{data}}\n  |data|\n|/section|\n]\n");
	magnum_populate_from_string(source, "{\"section\":false,\"data\":\"I got interpolated.\"}", out, cwd);
	CuAssertStrEquals(tc, "[\n  I got interpolated.\n  |data|\n\n  {{data}}\n  I got interpolated.\n]\n", out->str);

	// Partial Inheritence
	// Delimiters set in a parent template should not affect a partial.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "[ {{>include1}} ]\n{{= | | =}}\n[ |>include1| ]\n");
	magnum_populate_from_string(source, "{\"value\":\"yes\"}", out, cwd);
	CuAssertStrEquals(tc, "[ .yes. ]\n[ .yes. ]\n", out->str);

	// Post-Partial Behavior
	// Delimiters set in a partial should not affect the parent template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "[ {{>include2}} ]\n[ .{{value}}.  .|value|. ]\n");
	magnum_populate_from_string(source, "{\"value\":\"yes\"}", out, cwd);
	CuAssertStrEquals(tc, "[ .yes.  .yes. ]\n[ .yes.  .|value|. ]\n", out->str);

	// Surrounding Whitespace
	// Surrounding whitespace should be left untouched.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{=@ @=}} |");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "|  |", out->str);

	// Outlying Whitespace (Inline)
	// Whitespace should be left untouched.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " | {{=@ @=}}\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, " | \n", out->str);

	// Standalone Tag
	// Standalone lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Begin.\n{{=@ @=}}\nEnd.\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "Begin.\nEnd.\n", out->str);

	// Indented Standalone Tag
	// Indented standalone lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Begin.\n  {{=@ @=}}\nEnd.\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "Begin.\nEnd.\n", out->str);

	// Standalone Line Endings
	// "\r\n" should be considered a newline for standalone tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|\r\n{{= @ @ =}}\r\n|");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "|\r\n|", out->str);

	// Standalone Without Previous Line
	// Standalone tags should not require a newline to precede them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{=@ @=}}\n=");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "=", out->str);

	// Standalone Without Newline
	// Standalone tags should not require a newline to follow them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "=\n  {{=@ @=}}");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "=\n", out->str);

	// Pair with Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{= @   @ =}}|");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "||", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

