/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec.c

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
#include <stdio.h>
#include <string.h>

#include "d_string.h"
#include "libMagnum.h"


#ifdef TEST
#include "CuTest.h"

void Test_magnum_spec_interpolation(CuTest* tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// No Interpolation
	// Mustache-free templates should render as-is.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Hello from {Mustache}!\n");
	magnum_populate_from_string(source, "{}", out, NULL);
	CuAssertStrEquals(tc, "Hello from {Mustache}!\n", out->str);

	// Basic Interpolation
	// Unadorned tags should interpolate content into the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Hello, {{subject}}!\n");
	magnum_populate_from_string(source, "{\"subject\":\"world\"}", out, NULL);
	CuAssertStrEquals(tc, "Hello, world!\n", out->str);

	// HTML Escaping
	// Basic interpolation should be HTML escaped.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "These characters should be HTML escaped: {{forbidden}}\n");
	magnum_populate_from_string(source, "{\"forbidden\":\"& \\\" < >\"}", out, NULL);
	CuAssertStrEquals(tc, "These characters should be HTML escaped: &amp; &quot; &lt; &gt;\n", out->str);

	// Triple Mustache
	// Triple mustaches should interpolate without HTML escaping.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "These characters should not be HTML escaped: {{{forbidden}}}\n");
	magnum_populate_from_string(source, "{\"forbidden\":\"& \\\" < >\"}", out, NULL);
	CuAssertStrEquals(tc, "These characters should not be HTML escaped: & \" < >\n", out->str);

	// Ampersand
	// Ampersand should interpolate without HTML escaping.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "These characters should not be HTML escaped: {{&forbidden}}\n");
	magnum_populate_from_string(source, "{\"forbidden\":\"& \\\" < >\"}", out, NULL);
	CuAssertStrEquals(tc, "These characters should not be HTML escaped: & \" < >\n", out->str);

	// Basic Integer Interpolation
	// Integers should interpolate seamlessly.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{mph}} miles an hour!\"");
	magnum_populate_from_string(source, "{\"mph\":85}", out, NULL);
	CuAssertStrEquals(tc, "\"85 miles an hour!\"", out->str);

	// Triple Mustache Integer Interpolation
	// Integers should interpolate seamlessly.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{{mph}}} miles an hour!\"");
	magnum_populate_from_string(source, "{\"mph\":85}", out, NULL);
	CuAssertStrEquals(tc, "\"85 miles an hour!\"", out->str);

	// Ampersand Integer Interpolation
	// Integers should interpolate seamlessly.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{&mph}} miles an hour!\"");
	magnum_populate_from_string(source, "{\"mph\":85}", out, NULL);
	CuAssertStrEquals(tc, "\"85 miles an hour!\"", out->str);

	// Basic Decimal Interpolation
	// Decimals should interpolate seamlessly with proper significance.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{power}} jiggawatts!\"");
	magnum_populate_from_string(source, "{\"power\":1.210000}", out, NULL);
	CuAssertStrEquals(tc, "\"1.21 jiggawatts!\"", out->str);

	// Triple Mustache Decimal Interpolation
	// Decimals should interpolate seamlessly with proper significance.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{{power}}} jiggawatts!\"");
	magnum_populate_from_string(source, "{\"power\":1.210000}", out, NULL);
	CuAssertStrEquals(tc, "\"1.21 jiggawatts!\"", out->str);

	// Ampersand Decimal Interpolation
	// Decimals should interpolate seamlessly with proper significance.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{&power}} jiggawatts!\"");
	magnum_populate_from_string(source, "{\"power\":1.210000}", out, NULL);
	CuAssertStrEquals(tc, "\"1.21 jiggawatts!\"", out->str);

	// Basic Context Miss Interpolation
	// Failed context lookups should default to empty strings.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "I ({{cannot}}) be seen!");
	magnum_populate_from_string(source, "{}", out, NULL);
	CuAssertStrEquals(tc, "I () be seen!", out->str);

	// Triple Mustache Context Miss Interpolation
	// Failed context lookups should default to empty strings.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "I ({{{cannot}}}) be seen!");
	magnum_populate_from_string(source, "{}", out, NULL);
	CuAssertStrEquals(tc, "I () be seen!", out->str);

	// Ampersand Context Miss Interpolation
	// Failed context lookups should default to empty strings.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "I ({{&cannot}}) be seen!");
	magnum_populate_from_string(source, "{}", out, NULL);
	CuAssertStrEquals(tc, "I () be seen!", out->str);

	// Dotted Names - Basic Interpolation
	// Dotted names should be considered a form of shorthand for sections.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{person.name}}\" == \"{{#person}}{{name}}{{/person}}\"");
	magnum_populate_from_string(source, "{\"person\":{\"name\":\"Joe\"}}", out, NULL);
	CuAssertStrEquals(tc, "\"Joe\" == \"Joe\"", out->str);

	// Dotted Names - Triple Mustache Interpolation
	// Dotted names should be considered a form of shorthand for sections.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{{person.name}}}\" == \"{{#person}}{{{name}}}{{/person}}\"");
	magnum_populate_from_string(source, "{\"person\":{\"name\":\"Joe\"}}", out, NULL);
	CuAssertStrEquals(tc, "\"Joe\" == \"Joe\"", out->str);

	// Dotted Names - Ampersand Interpolation
	// Dotted names should be considered a form of shorthand for sections.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{&person.name}}\" == \"{{#person}}{{&name}}{{/person}}\"");
	magnum_populate_from_string(source, "{\"person\":{\"name\":\"Joe\"}}", out, NULL);
	CuAssertStrEquals(tc, "\"Joe\" == \"Joe\"", out->str);

	// Dotted Names - Arbitrary Depth
	// Dotted names should be functional to any level of nesting.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{a.b.c.d.e.name}}\" == \"Phil\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"name\":\"Phil\"}}}}}}", out, NULL);
	CuAssertStrEquals(tc, "\"Phil\" == \"Phil\"", out->str);

	// Dotted Names - Broken Chains
	// Any falsey value prior to the last part of the name should yield ''.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{a.b.c}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{}}", out, NULL);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Dotted Names - Broken Chain Resolution
	// Each part of a dotted name should resolve only against its parent.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{a.b.c.name}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{}},\"c\":{\"name\":\"Jim\"}}", out, NULL);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Dotted Names - Initial Resolution
	// The first part of a dotted name should resolve as any other name.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#a}}{{b.c.d.e.name}}{{/a}}\" == \"Phil\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"name\":\"Phil\"}}}}},\"b\":{\"c\":{\"d\":{\"e\":{\"name\":\"Wrong\"}}}}}", out, NULL);
	CuAssertStrEquals(tc, "\"Phil\" == \"Phil\"", out->str);

	// Interpolation - Surrounding Whitespace
	// Interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{string}} |");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "| --- |", out->str);

	// Triple Mustache - Surrounding Whitespace
	// Interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{{string}}} |");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "| --- |", out->str);

	// Ampersand - Surrounding Whitespace
	// Interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{&string}} |");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "| --- |", out->str);

	// Interpolation - Standalone
	// Standalone interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{string}}\n");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "  ---\n", out->str);

	// Triple Mustache - Standalone
	// Standalone interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{{string}}}\n");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "  ---\n", out->str);

	// Ampersand - Standalone
	// Standalone interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{&string}}\n");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "  ---\n", out->str);

	// Interpolation With Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{ string }}|");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "|---|", out->str);

	// Triple Mustache With Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{{ string }}}|");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "|---|", out->str);

	// Ampersand With Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{& string }}|");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, NULL);
	CuAssertStrEquals(tc, "|---|", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

