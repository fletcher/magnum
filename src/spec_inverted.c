/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec_inverted.c

	Inverted Section tags and End Section tags are used in combination to wrap a
section of the template.

These tags' content MUST be a non-whitespace character sequence NOT
containing the current closing delimiter; each Inverted Section tag MUST be
followed by an End Section tag with the same content within the same
section.

This tag's content names the data to replace the tag.  Name resolution is as
follows:
  1) Split the name on periods; the first part is the name to resolve, any
  remaining parts should be retained.
  2) Walk the context stack from top to bottom, finding the first context
  that is a) a hash containing the name as a key OR b) an object responding
  to a method with the given name.
  3) If the context is a hash, the data is the value associated with the
  name.
  4) If the context is an object and the method with the given name has an
  arity of 1, the method SHOULD be called with a String containing the
  unprocessed contents of the sections; the data is the value returned.
  5) Otherwise, the data is the value returned by calling the method with
  the given name.
  6) If any name parts were retained in step 1, each should be resolved
  against a context stack containing only the result from the former
  resolution.  If any part fails resolution, the result should be considered
  falsey, and should interpolate as the empty string.
If the data is not of a list type, it is coerced into a list as follows: if
the data is truthy (e.g. `!!data == true`), use a single-element list
containing the data, otherwise use an empty list.

This section MUST NOT be rendered unless the data list is empty.

Inverted Section and End Section tags SHOULD be treated as standalone when
appropriate.



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

void Test_magnum_spec_inverted(CuTest * tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Determine current directory
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));

	// Shift to ../test/partials
	strcat(cwd, "/../test/partials");

	// Falsey
	// Falsey sections should have their contents rendered.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^boolean}}This should be rendered.{{/boolean}}\"");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "\"This should be rendered.\"", out->str);

	// Truthy
	// Truthy sections should have their contents omitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^boolean}}This should not be rendered.{{/boolean}}\"");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// Context
	// Objects and hashes should behave like truthy values.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^context}}Hi {{name}}.{{/context}}\"");
	magnum_populate_from_string(source, "{\"context\":{\"name\":\"Joe\"}}", out, cwd);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// List
	// Lists should behave like truthy values.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^list}}{{n}}{{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[{\"n\":1},{\"n\":2},{\"n\":3}]}", out, cwd);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// Empty List
	// Empty lists should behave like falsey values.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^list}}Yay lists!{{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[]}", out, cwd);
	CuAssertStrEquals(tc, "\"Yay lists!\"", out->str);

	// Doubled
	// Multiple inverted sections per template should be permitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{^bool}}\n* first\n{{/bool}}\n* {{two}}\n{{^bool}}\n* third\n{{/bool}}\n");
	magnum_populate_from_string(source, "{\"two\":\"second\",\"bool\":false}", out, cwd);
	CuAssertStrEquals(tc, "* first\n* second\n* third\n", out->str);

	// Nested (Falsey)
	// Nested falsey sections should have their contents rendered.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |");
	magnum_populate_from_string(source, "{\"bool\":false}", out, cwd);
	CuAssertStrEquals(tc, "| A B C D E |", out->str);

	// Nested (Truthy)
	// Nested truthy sections should be omitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |");
	magnum_populate_from_string(source, "{\"bool\":true}", out, cwd);
	CuAssertStrEquals(tc, "| A  E |", out->str);

	// Context Misses
	// Failed context lookups should be considered falsey.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "[{{^missing}}Cannot find key 'missing'!{{/missing}}]");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "[Cannot find key 'missing'!]", out->str);

	// Dotted Names - Truthy
	// Dotted names should be valid for Inverted Section tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":true}}}", out, cwd);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Dotted Names - Falsey
	// Dotted names should be valid for Inverted Section tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":false}}}", out, cwd);
	CuAssertStrEquals(tc, "\"Not Here\" == \"Not Here\"", out->str);

	// Dotted Names - Broken Chains
	// Dotted names that cannot be resolved should be considered falsey.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"");
	magnum_populate_from_string(source, "{\"a\":{}}", out, cwd);
	CuAssertStrEquals(tc, "\"Not Here\" == \"Not Here\"", out->str);

	// Surrounding Whitespace
	// Inverted sections should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " | {{^boolean}}\t|\t{{/boolean}} | \n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, " | \t|\t | \n", out->str);

	// Internal Whitespace
	// Inverted should not alter internal whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " | {{^boolean}} {{! Important Whitespace }}\n {{/boolean}} | \n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, " |  \n  | \n", out->str);

	// Indented Inline Sections
	// Single-line sections should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " {{^boolean}}NO{{/boolean}}\n {{^boolean}}WAY{{/boolean}}\n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, " NO\n WAY\n", out->str);

	// Standalone Lines
	// Standalone lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| This Is\n{{^boolean}}\n|\n{{/boolean}}\n| A Line\n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "| This Is\n|\n| A Line\n", out->str);

	// Standalone Indented Lines
	// Standalone indented lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| This Is\n  {{^boolean}}\n|\n  {{/boolean}}\n| A Line\n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "| This Is\n|\n| A Line\n", out->str);

	// Standalone Line Endings
	// "\r\n" should be considered a newline for standalone tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|\r\n{{^boolean}}\r\n{{/boolean}}\r\n|");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "|\r\n|", out->str);

	// Standalone Without Previous Line
	// Standalone tags should not require a newline to precede them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{^boolean}}\n^{{/boolean}}\n/");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "^\n/", out->str);

	// Standalone Without Newline
	// Standalone tags should not require a newline to follow them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "^{{^boolean}}\n/\n  {{/boolean}}");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "^\n/\n", out->str);

	// Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{^ boolean }}={{/ boolean }}|");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "|=|", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

