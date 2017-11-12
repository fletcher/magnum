/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec.c

	@brief Bootstrap test suite from https://github.com/mustache/spec


	@author	Fletcher T. Penney
	@bug


**/

/*

	Original Code Copyright © 2017 Fletcher T. Penney.

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
#include <stdio.h>
#include <string.h>

#include "d_string.h"
#include "libMagnum.h"


#ifdef TEST
#include "CuTest.h"

void Test_magnum_spec_inverted(CuTest* tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Falsey
	// Falsey sections should have their contents rendered.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^boolean}}This should be rendered.{{/boolean}}\"");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, "\"This should be rendered.\"", out->str);

	// Truthy
	// Truthy sections should have their contents omitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^boolean}}This should not be rendered.{{/boolean}}\"");
	magnum_populate_from_string(source, "{\"boolean\":true}", out, NULL);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// Context
	// Objects and hashes should behave like truthy values.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^context}}Hi {{name}}.{{/context}}\"");
	magnum_populate_from_string(source, "{\"context\":{\"name\":\"Joe\"}}", out, NULL);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// List
	// Lists should behave like truthy values.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^list}}{{n}}{{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[{\"n\":1},{\"n\":2},{\"n\":3}]}", out, NULL);
	CuAssertStrEquals(tc, "\"\"", out->str);

	// Empty List
	// Empty lists should behave like falsey values.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^list}}Yay lists!{{/list}}\"");
	magnum_populate_from_string(source, "{\"list\":[]}", out, NULL);
	CuAssertStrEquals(tc, "\"Yay lists!\"", out->str);

	// Doubled
	// Multiple inverted sections per template should be permitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{^bool}}\n* first\n{{/bool}}\n* {{two}}\n{{^bool}}\n* third\n{{/bool}}\n");
	magnum_populate_from_string(source, "{\"two\":\"second\",\"bool\":false}", out, NULL);
	CuAssertStrEquals(tc, "* first\n* second\n* third\n", out->str);

	// Nested (Falsey)
	// Nested falsey sections should have their contents rendered.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |");
	magnum_populate_from_string(source, "{\"bool\":false}", out, NULL);
	CuAssertStrEquals(tc, "| A B C D E |", out->str);

	// Nested (Truthy)
	// Nested truthy sections should be omitted.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| A {{^bool}}B {{^bool}}C{{/bool}} D{{/bool}} E |");
	magnum_populate_from_string(source, "{\"bool\":true}", out, NULL);
	CuAssertStrEquals(tc, "| A  E |", out->str);

	// Context Misses
	// Failed context lookups should be considered falsey.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "[{{^missing}}Cannot find key 'missing'!{{/missing}}]");
	magnum_populate_from_string(source, "{}", out, NULL);
	CuAssertStrEquals(tc, "[Cannot find key 'missing'!]", out->str);

	// Dotted Names - Truthy
	// Dotted names should be valid for Inverted Section tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":true}}}", out, NULL);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Dotted Names - Falsey
	// Dotted names should be valid for Inverted Section tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":false}}}", out, NULL);
	CuAssertStrEquals(tc, "\"Not Here\" == \"Not Here\"", out->str);

	// Dotted Names - Broken Chains
	// Dotted names that cannot be resolved should be considered falsey.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{^a.b.c}}Not Here{{/a.b.c}}\" == \"Not Here\"");
	magnum_populate_from_string(source, "{\"a\":{}}", out, NULL);
	CuAssertStrEquals(tc, "\"Not Here\" == \"Not Here\"", out->str);

	// Surrounding Whitespace
	// Inverted sections should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " | {{^boolean}}\t|\t{{/boolean}} | \n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, " | \t|\t | \n", out->str);

	// Internal Whitespace
	// Inverted should not alter internal whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " | {{^boolean}} {{! Important Whitespace }}\n {{/boolean}} | \n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, " |  \n  | \n", out->str);

	// Indented Inline Sections
	// Single-line sections should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, " {{^boolean}}NO{{/boolean}}\n {{^boolean}}WAY{{/boolean}}\n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, " NO\n WAY\n", out->str);

	// Standalone Lines
	// Standalone lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| This Is\n{{^boolean}}\n|\n{{/boolean}}\n| A Line\n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, "| This Is\n|\n| A Line\n", out->str);

	// Standalone Indented Lines
	// Standalone indented lines should be removed from the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| This Is\n  {{^boolean}}\n|\n  {{/boolean}}\n| A Line\n");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, "| This Is\n|\n| A Line\n", out->str);

	// Standalone Line Endings
	// "\r\n" should be considered a newline for standalone tags.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|\r\n{{^boolean}}\r\n{{/boolean}}\r\n|");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, "|\r\n|", out->str);

	// Standalone Without Previous Line
	// Standalone tags should not require a newline to precede them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{^boolean}}\n^{{/boolean}}\n/");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, "^\n/", out->str);

	// Standalone Without Newline
	// Standalone tags should not require a newline to follow them.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "^{{^boolean}}\n/\n  {{/boolean}}");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, "^\n/\n", out->str);

	// Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{^ boolean }}={{/ boolean }}|");
	magnum_populate_from_string(source, "{\"boolean\":false}", out, NULL);
	CuAssertStrEquals(tc, "|=|", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

