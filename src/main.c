/*

	Magnum -- C implementation of Mustache logic-less templates

	main.c -- Magnum main()

	Copyright © 2017-2018 Fletcher T. Penney.

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

#include <stdio.h>
#include <stdlib.h>

#include "d_string.h"
#include "file.h"
#include "json.h"
#include "libMagnum.h"


int main( int argc, char** argv ) {
	if (argc > 2) {
		argv++;

		JSON_Value * j = json_from_file(*argv++);
		DString * template;
		DString * out = d_string_new("");

		char * dir, * file, * absolute;

		while (j && *argv) {
			absolute = absolute_path_for_argument(*argv);

			split_path_file(&dir, &file, absolute);

			template = scan_file(*argv++);

			magnum_populate_from_json(template, j, out, dir, NULL);

			d_string_free(template, true);
			free(dir);
			free(file);
			free(absolute);
		}

		fprintf(stdout, "%s", out->str);

		d_string_free(out, true);
	}
}
