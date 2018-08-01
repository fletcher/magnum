/**

	Magnum -- C implementation of Mustache logic-less templates

	@file spec_interpolation.c

	Interpolation tags are used to integrate dynamic content into the template.

The tag's content MUST be a non-whitespace character sequence NOT containing
the current closing delimiter.

This tag's content names the data to replace the tag.  A single period (`.`)
indicates that the item currently sitting atop the context stack should be
used; otherwise, name resolution is as follows:
  1) Split the name on periods; the first part is the name to resolve, any
  remaining parts should be retained.
  2) Walk the context stack from top to bottom, finding the first context
  that is a) a hash containing the name as a key OR b) an object responding
  to a method with the given name.
  3) If the context is a hash, the data is the value associated with the
  name.
  4) If the context is an object, the data is the value returned by the
  method with the given name.
  5) If any name parts were retained in step 1, each should be resolved
  against a context stack containing only the result from the former
  resolution.  If any part fails resolution, the result should be considered
  falsey, and should interpolate as the empty string.
Data should be coerced into a string (and escaped, if appropriate) before
interpolation.

The Interpolation tags MUST NOT be treated as standalone.



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

void Test_magnum_spec_interpolation(CuTest* tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Determine current directory
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));

	// Shift to ../test/partials
	strcat(cwd, "/../test/partials");

	// No Interpolation
	// Mustache-free templates should render as-is.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Hello from {Mustache}!\n");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "Hello from {Mustache}!\n", out->str);

	// Basic Interpolation
	// Unadorned tags should interpolate content into the template.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Hello, {{subject}}!\n");
	magnum_populate_from_string(source, "{\"subject\":\"world\"}", out, cwd);
	CuAssertStrEquals(tc, "Hello, world!\n", out->str);

	// HTML Escaping
	// Basic interpolation should be HTML escaped.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "These characters should be HTML escaped: {{forbidden}}\n");
	magnum_populate_from_string(source, "{\"forbidden\":\"& \\\" < >\"}", out, cwd);
	CuAssertStrEquals(tc, "These characters should be HTML escaped: &amp; &quot; &lt; &gt;\n", out->str);

	// Triple Mustache
	// Triple mustaches should interpolate without HTML escaping.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "These characters should not be HTML escaped: {{{forbidden}}}\n");
	magnum_populate_from_string(source, "{\"forbidden\":\"& \\\" < >\"}", out, cwd);
	CuAssertStrEquals(tc, "These characters should not be HTML escaped: & \" < >\n", out->str);

	// Ampersand
	// Ampersand should interpolate without HTML escaping.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "These characters should not be HTML escaped: {{&forbidden}}\n");
	magnum_populate_from_string(source, "{\"forbidden\":\"& \\\" < >\"}", out, cwd);
	CuAssertStrEquals(tc, "These characters should not be HTML escaped: & \" < >\n", out->str);

	// Basic Integer Interpolation
	// Integers should interpolate seamlessly.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{mph}} miles an hour!\"");
	magnum_populate_from_string(source, "{\"mph\":85}", out, cwd);
	CuAssertStrEquals(tc, "\"85 miles an hour!\"", out->str);

	// Triple Mustache Integer Interpolation
	// Integers should interpolate seamlessly.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{{mph}}} miles an hour!\"");
	magnum_populate_from_string(source, "{\"mph\":85}", out, cwd);
	CuAssertStrEquals(tc, "\"85 miles an hour!\"", out->str);

	// Ampersand Integer Interpolation
	// Integers should interpolate seamlessly.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{&mph}} miles an hour!\"");
	magnum_populate_from_string(source, "{\"mph\":85}", out, cwd);
	CuAssertStrEquals(tc, "\"85 miles an hour!\"", out->str);

	// Basic Decimal Interpolation
	// Decimals should interpolate seamlessly with proper significance.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{power}} jiggawatts!\"");
	magnum_populate_from_string(source, "{\"power\":1.210000}", out, cwd);
	CuAssertStrEquals(tc, "\"1.21 jiggawatts!\"", out->str);

	// Triple Mustache Decimal Interpolation
	// Decimals should interpolate seamlessly with proper significance.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{{power}}} jiggawatts!\"");
	magnum_populate_from_string(source, "{\"power\":1.210000}", out, cwd);
	CuAssertStrEquals(tc, "\"1.21 jiggawatts!\"", out->str);

	// Ampersand Decimal Interpolation
	// Decimals should interpolate seamlessly with proper significance.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{&power}} jiggawatts!\"");
	magnum_populate_from_string(source, "{\"power\":1.210000}", out, cwd);
	CuAssertStrEquals(tc, "\"1.21 jiggawatts!\"", out->str);

	// Basic Context Miss Interpolation
	// Failed context lookups should default to empty strings.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "I ({{cannot}}) be seen!");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "I () be seen!", out->str);

	// Triple Mustache Context Miss Interpolation
	// Failed context lookups should default to empty strings.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "I ({{{cannot}}}) be seen!");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "I () be seen!", out->str);

	// Ampersand Context Miss Interpolation
	// Failed context lookups should default to empty strings.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "I ({{&cannot}}) be seen!");
	magnum_populate_from_string(source, "{}", out, cwd);
	CuAssertStrEquals(tc, "I () be seen!", out->str);

	// Dotted Names - Basic Interpolation
	// Dotted names should be considered a form of shorthand for sections.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{person.name}}\" == \"{{#person}}{{name}}{{/person}}\"");
	magnum_populate_from_string(source, "{\"person\":{\"name\":\"Joe\"}}", out, cwd);
	CuAssertStrEquals(tc, "\"Joe\" == \"Joe\"", out->str);

	// Dotted Names - Triple Mustache Interpolation
	// Dotted names should be considered a form of shorthand for sections.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{{person.name}}}\" == \"{{#person}}{{{name}}}{{/person}}\"");
	magnum_populate_from_string(source, "{\"person\":{\"name\":\"Joe\"}}", out, cwd);
	CuAssertStrEquals(tc, "\"Joe\" == \"Joe\"", out->str);

	// Dotted Names - Ampersand Interpolation
	// Dotted names should be considered a form of shorthand for sections.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{&person.name}}\" == \"{{#person}}{{&name}}{{/person}}\"");
	magnum_populate_from_string(source, "{\"person\":{\"name\":\"Joe\"}}", out, cwd);
	CuAssertStrEquals(tc, "\"Joe\" == \"Joe\"", out->str);

	// Dotted Names - Arbitrary Depth
	// Dotted names should be functional to any level of nesting.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{a.b.c.d.e.name}}\" == \"Phil\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"name\":\"Phil\"}}}}}}", out, cwd);
	CuAssertStrEquals(tc, "\"Phil\" == \"Phil\"", out->str);

	// Dotted Names - Broken Chains
	// Any falsey value prior to the last part of the name should yield ''.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{a.b.c}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{}}", out, cwd);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Dotted Names - Broken Chain Resolution
	// Each part of a dotted name should resolve only against its parent.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{a.b.c.name}}\" == \"\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{}},\"c\":{\"name\":\"Jim\"}}", out, cwd);
	CuAssertStrEquals(tc, "\"\" == \"\"", out->str);

	// Dotted Names - Initial Resolution
	// The first part of a dotted name should resolve as any other name.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "\"{{#a}}{{b.c.d.e.name}}{{/a}}\" == \"Phil\"");
	magnum_populate_from_string(source, "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"name\":\"Phil\"}}}}},\"b\":{\"c\":{\"d\":{\"e\":{\"name\":\"Wrong\"}}}}}", out, cwd);
	CuAssertStrEquals(tc, "\"Phil\" == \"Phil\"", out->str);

	// Interpolation - Surrounding Whitespace
	// Interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{string}} |");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "| --- |", out->str);

	// Triple Mustache - Surrounding Whitespace
	// Interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{{string}}} |");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "| --- |", out->str);

	// Ampersand - Surrounding Whitespace
	// Interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "| {{&string}} |");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "| --- |", out->str);

	// Interpolation - Standalone
	// Standalone interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{string}}\n");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "  ---\n", out->str);

	// Triple Mustache - Standalone
	// Standalone interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{{string}}}\n");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "  ---\n", out->str);

	// Ampersand - Standalone
	// Standalone interpolation should not alter surrounding whitespace.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "  {{&string}}\n");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "  ---\n", out->str);

	// Interpolation With Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{ string }}|");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "|---|", out->str);

	// Triple Mustache With Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{{ string }}}|");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "|---|", out->str);

	// Ampersand With Padding
	// Superfluous in-tag whitespace should be ignored.
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "|{{& string }}|");
	magnum_populate_from_string(source, "{\"string\":\"---\"}", out, cwd);
	CuAssertStrEquals(tc, "|---|", out->str);

	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

