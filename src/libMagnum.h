/**

	Magnum -- C implementation of Mustache logic-less templates

	@file libMagnum.h

    @brief Combine a source template using the Mustache syntax and JSON data
	in order to generate a finished text string.

	Often used to generate HTML, but can be used to generate anything,
	including source code.


	See <http://mustache.github.io/> for more information.


	See <https://github.com/mustache/spec> for the formal spec.


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2017-2018 Fletcher T. Penney.

	The `magnum` project is released under the MIT License.

	d_string.c and d_string.h are from the MultiMarkdown v6 project:

		https://github.com/fletcher/MultiMarkdown-6/

	MMD 6 is released under both the MIT License.

	parson is Copyright (c) 2012 - 2017 Krzysztof Gabis and is licensed under
	the MIT License.

	Portions inspired by `mustach` by José Bollo <jobol@nonadev.net>, licensed
	under the Apache License, Version 2.0:

		http://www.apache.org/licenses/LICENSE-2.0


	CuTest is released under the zlib/libpng license. See CuTest.c for the text
	of the license.


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


#ifndef LIBMAGNUM_MAGNUM_H
#define LIBMAGNUM_MAGNUM_H

/// From d_string.h:
typedef struct DString DString;

// From parson.h
typedef struct json_value_t  JSON_Value;


typedef struct closure closure;

/// Given a source string, populate it using data from a JSON value.
/// The resulting text will be appended to `out`.
/// Pass NULL as `load_p` to use the default load_partial function.
int magnum_populate_from_json(DString * source, JSON_Value * json, DString * out, const char * search_directory, int (*load_p)(char *, DString *, closure *, char **));


/// Given a source string, populate it using data from a JSON string.
/// The resulting text will be appended to `out`.
int magnum_populate_from_string(DString * source, const char * string, DString * out, const char * search_directory);


/// Given a source string, populate it using data from a JSON string, using a custom load_partial routine
/// The resulting text will be appended to `out`.
int magnum_populate_from_string_custom_partial(DString * source, const char * string, DString * out, const char * search_directory, int (*load_p)(char *, DString *, struct closure *, char **));


/// Given a source string, populate it using data from a JSON file.
/// The resulting text will be appended to `out`.
int magnum_populate_from_file(DString * source, const char * fname, DString * out, const char * search_directory);


/// Simplified method to allow use without any other included files.
/// Useful if you have no other need for parson or d_string
int magnum_populate_char_only(const char * source, const char * string, char ** out, const char * search_directory);


#endif
