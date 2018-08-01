/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec_sections.c

	Section tags and End Section tags are used in combination to wrap a section
of the template for iteration

These tags' content MUST be a non-whitespace character sequence NOT
containing the current closing delimiter; each Section tag MUST be followed
by an End Section tag with the same content within the same section.

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

For each element in the data list, the element MUST be pushed onto the
context stack, the section MUST be rendered, and the element MUST be popped
off the context stack.

Section and End Section tags SHOULD be treated as standalone when
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

void Test_magnum_spec_sections(CuTest* tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Determine current directory
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));

	// Shift to ../test/partials
	strcat(cwd, "/../test/partials");

	// Truthy
	// Truthy sections should have their contents rendered.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#boolean}}This should be rendered.{{/boolean}}\"");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "\"This should be rendered.\"", out->str);

	// Falsey
	// Falsey sections should have their contents omitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#boolean}}This should not be rendered.{{/boolean}}\"");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, cwd);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// Context
	// Objects and hashes should be pushed onto the context stack.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#context}}Hi {{name}}.{{/context}}\"");
	magnum_populate_from_string(source, "{\"context\":{\"name\":\"Joe\"}}", out, cwd);
	CuAssertStrEquals(tc, "\"Hi Joe.\"", out->str);

	// Deeply Nested Contexts
	// All elements on the context stack should be accessible.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{#a}}\n{{one}}\n{{#b}}\n{{one}}{{two}}{{one}}\n{{#c}}\n{{one}}{{two}}{{three}}{{two}}{{one}}\n{{#d}}\n{{one}}{{two}}{{three}}{{four}}{{three}}{{two}}{{one}}\n{{#e}}\n{{one}}{{two}}{{three}}{{four}}{{five}}{{four}}{{three}}{{two}}{{one}}\n{{/e}}\n{{one}}{{two}}{{three}}{{four}}{{three}}{{two}}{{one}}\n{{/d}}\n{{one}}{{two}}{{three}}{{two}}{{one}}\n{{/c}}\n{{one}}{{two}}{{one}}\n{{/b}}\n{{one}}\n{{/a}}\n");
	magnum_populate_from_string(source, "{\"a\":{\"one\":1},\"b\":{\"two\":2},\"c\":{\"three\":3},\"d\":{\"four\":4},\"e\":{\"five\":5}}", out, cwd);
	CuAssertStrEquals(tc, "1\n121\n12321\n1234321\n123454321\n1234321\n12321\n121\n1\n", out->str);

	// List
	// Lists should be iterated; list items should visit the context stack.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#list}}{{item}}{{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[{\"item\":1},{\"item\":2},{\"item\":3}]}", out, cwd);
	CuAssertStrEquals(tc, "\"123\"", out->str);

	// Empty List
	// Empty lists should behave like falsey values.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#list}}Yay lists!{{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[]}", out, cwd);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// Doubled
	// Multiple sections per template should be permitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{#bool}}\n* first\n{{/bool}}\n* {{two}}\n{{#bool}}\n* third\n{{/bool}}\n");
	magnum_populate_from_string(source, "{\"two\":\"second\",\"bool\":true}", out, cwd);
	CuAssertStrEquals(tc, "* first\n* second\n* third\n", out->str);

	// Nested (Truthy)
	// Nested truthy sections should have their contents rendered.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| A {{#bool}}B {{#bool}}C{{/bool}} D{{/bool}} E |");
	magnum_populate_from_string(source, "{\"bool\":true}", out, cwd);
	CuAssertStrEquals(tc, "| A B C D E |", out->str);

	// Nested (Falsey)
	// Nested falsey sections should be omitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| A {{#bool}}B {{#bool}}C{{/bool}} D{{/bool}} E |");
	magnum_populate_from_string(source, "{\"bool\":false}", out, cwd);
	CuAssertStrEquals(tc, "| A  E |", out->str);

	// Context Misses
	// Failed context lookups should be considered falsey.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "[{{#missing}}Found key 'missing'!{{/missing}}]");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "[]", out->str);

	// Implicit Iterator - String
	// Implicit iterators should directly interpolate strings.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#list}}({{.}}){{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[\"a\",\"b\",\"c\",\"d\",\"e\"]}", out, cwd);
	CuAssertStrEquals(tc, "\"(a)(b)(c)(d)(e)\"", out->str);

	// Implicit Iterator - Integer
	// Implicit iterators should cast integers to strings and interpolate.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#list}}({{.}}){{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[1,2,3,4,5]}", out, cwd);
	CuAssertStrEquals(tc, "\"(1)(2)(3)(4)(5)\"", out->str);

	// Implicit Iterator - Decimal
	// Implicit iterators should cast decimals to strings and interpolate.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#list}}({{.}}){{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[1.100000,2.200000,3.300000,4.400000,5.500000]}", out, cwd);
	CuAssertStrEquals(tc, "\"(1.1)(2.2)(3.3)(4.4)(5.5)\"", out->str);

	// Implicit Iterator - Array
	// Implicit iterators should allow iterating over nested arrays.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#list}}({{#.}}{{.}}{{/.}}){{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[[1,2,3],[\"a\",\"b\",\"c\"]]}", out, cwd);
	CuAssertStrEquals(tc, "\"(123)(abc)\"", out->str);

	// Dotted Names - Truthy
	// Dotted names should be valid for Section tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"Here\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":true}}}", out, cwd);
	CuAssertStrEquals(tc, "\"Here\" == \"Here\"", out->str);

	// Dotted Names - Falsey
	// Dotted names should be valid for Section tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":false}}}", out, cwd);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Dotted Names - Broken Chains
	// Dotted names that cannot be resolved should be considered falsey.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{}}", out, cwd);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Surrounding Whitespace
	// Sections should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " | {{#boolean}}\t|\t{{/boolean}} | \n");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, " | \t|\t | \n", out->str);

	// Internal Whitespace
	// Sections should not alter internal whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " | {{#boolean}} {{! Important Whitespace }}\n {{/boolean}} | \n");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, " |  \n  | \n", out->str);

	// Indented Inline Sections
	// Single-line sections should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " {{#boolean}}YES{{/boolean}}\n {{#boolean}}GOOD{{/boolean}}\n");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, " YES\n GOOD\n", out->str);

	// Standalone Lines
	// Standalone lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| This Is\n{{#boolean}}\n|\n{{/boolean}}\n| A Line\n");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "| This Is\n|\n| A Line\n", out->str);

	// Indented Standalone Lines
	// Indented standalone lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| This Is\n  {{#boolean}}\n|\n  {{/boolean}}\n| A Line\n");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "| This Is\n|\n| A Line\n", out->str);

	// Standalone Line Endings
	// "\r\n" should be considered a newline for standalone tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|\r\n{{#boolean}}\r\n{{/boolean}}\r\n|");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "|\r\n|", out->str);

	// Standalone Without Previous Line
	// Standalone tags should not require a newline to precede them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{#boolean}}\n#{{/boolean}}\n/");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "#\n/", out->str);

	// Standalone Without Newline
	// Standalone tags should not require a newline to follow them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "#{{#boolean}}\n/\n  {{/boolean}}");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "#\n/\n", out->str);

	// Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{# boolean }}={{/ boolean }}|");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, cwd);
	CuAssertStrEquals(tc, "|=|", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

