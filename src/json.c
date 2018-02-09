/**

	@file json.c

	@brief Utility functions when working with parson for JSON parsing

	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2016-2018 Fletcher T. Penney.

	d_string.c and d_string.h are from the MultiMarkdown v6 project:

		https://github.com/fletcher/MultiMarkdown-6/

	MMD 6 is released under both the MIT License.


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
#include <string.h>

#include "d_string.h"
#include "json.h"


#define kBUFFERSIZE 4096	// How many bytes to read at a time


/// Load JSON from string
/// JSON_Value will need to be freed
JSON_Value * json_from_string(const char * string) {
	// Parse JSON source string
	JSON_Value *	root_value;

	root_value = json_parse_string(string);

	if (json_value_get_type(root_value) == JSONError) {
		fprintf(stderr, "Invalid JSON...\n");
		json_value_free(root_value);
		return NULL;
	}

	return root_value;
}


/// Load JSON from file
/// JSON_Value will need to be freed
JSON_Value * json_from_file(const char * fname) {
	/* Read from file */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	FILE * file;

	if ((file = fopen(fname, "r")) == NULL ) {
		fprintf(stderr, "Error reading file...\n");
		return NULL;
	}

	DString * buffer = d_string_new("");

	while ((bytes = fread(chunk, 1, kBUFFERSIZE, file)) > 0) {
		d_string_append_c_array(buffer, chunk, bytes);
	}

	fclose(file);

	JSON_Value * root_value = json_from_string(buffer->str);

	d_string_free(buffer, true);

	return root_value;
}


#ifdef TEST
void Test_json_from_string(CuTest* tc) {
	JSON_Value * root;
	JSON_Object * object;
	const char * string;

	// TODO: Looks like this generates a memory leak in parson
	root = json_from_string("{ \"test\"");
	CuAssertPtrEquals(tc, NULL, root);
	json_value_free(root);


	root = json_from_string("{ \"test\" : \"value\" }");
	CuAssertPtrNotNull(tc, root);

	object = json_object(root);
	string = json_object_get_string(object, "test");
	CuAssertStrEquals(tc, "value", string);

	json_value_free(root);
}
#endif

