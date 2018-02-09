/**

	@file json.h

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


#ifndef JSON_DRYDOCK_H
#define JSON_DRYDOCK_H

#include "parson.h"

#ifdef TEST
	#include "CuTest.h"
#endif


/// Load JSON schema from string
// JSON_Value will need to be freed
JSON_Value * json_from_string(const char * string);


/// Load JSON schema from file
// JSON_Value will need to be freed
JSON_Value * json_from_file(const char * fname);


#endif
