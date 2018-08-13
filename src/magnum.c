/**

	Magnum -- C implementation of Mustache logic-less templates

	@file magnum.c

	@brief


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


#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "d_string.h"
#include "file.h"
#include "json.h"
#include "libMagnum.h"
#include "parson.h"


#ifdef TEST
	#include "CuTest.h"
#endif


#define kMaxKeyLength			1024
#define kMaxDepth				256
#define kMaxDelimiterLength		16

#if !defined(MAX)
	#define MAX(A,B) ((A) >= (B) ? (A) : (B))
#endif

#if !defined(MIN)
	#define MIN(A,B) ((A) <= (B) ? (A) : (B))
#endif


/// Track JSON data and pointer to current object
struct closure {
	JSON_Value *		root;		//!< Root data value
	int					depth;		//!< Depth in stack
	DString *			out;		//!< Output destination

	const char *		directory;	//!< Initial search directory for partials

	int (*load_partial)(char *, DString *, struct closure *, char **);

	struct {
		JSON_Value *	container;
		JSON_Value *	val;
		int				index;
		int				count;
	} stack[kMaxDepth];
};


/// strdup() not available on all platforms
static char * my_strdup(const char * source) {
	if (source == NULL) {
		return NULL;
	}

	char * result = malloc(strlen(source) + 1);

	if (result) {
		strcpy(result, source);
	}

	return result;
}


// Resolve `name` to find the proper value
static JSON_Value * find(struct closure * c, const char * name) {
	JSON_Object * o;
	JSON_Value * v = NULL;
	int i;

	if (name[0] == '.' && name[1] == '\0') {
		// {{.}} means we use the current value
		v = c->stack[c->depth].val;
	} else {
		// Break name into key/value

		// Iterate
		i = c->depth;
		v = c->stack[i].val;
		o = json_value_get_object(v);

		while (i > 0 && !(json_object_dothas_value(o, name))) {
			i--;
			v = c->stack[i].val;

			if (v) {
				o = json_value_get_object(v);
			}
		}

		// Not found
		if (i < 0) {
			return NULL;
		}

		v = json_object_dotget_value(o, name);
	}

	return v;
}


// Indent each line of partial
void indent_text(DString * text, const char * indent, size_t indent_len) {
	if (indent && indent_len) {
		DString * replace = d_string_new("");
		d_string_append_c_array(replace, indent, indent_len);

		// Preserve Windows line endings
		d_string_replace_text_in_range(text, 0, -1, "\r\n", "!!MAGNUMWINDOWSLINEENDING!!");

		// Mac Classic line endings
		d_string_prepend(replace, "\r");
		d_string_replace_text_in_range(text, 0, -1, "\r", replace->str);

		// Unix line endings
		d_string_erase(replace, 0, 1);
		d_string_prepend(replace, "\n");
		d_string_replace_text_in_range(text, 0, -1, "\n", replace->str);

		// Windows line endings
		d_string_prepend(replace, "\r");
		d_string_replace_text_in_range(text, 0, -1, "!!MAGNUMWINDOWSLINEENDING!!", replace->str);

		d_string_free(replace, true);
	}
}


// Load partial
static int load_partial(char * name, DString * partial, struct closure * c, char ** search_directory) {
	// Require search_directory to enable partials
	if (*search_directory == NULL) {
		return -1;
	}

	DString * load;

	// Check for partial from current directory
	char * target = path_from_dir_base(*search_directory, name);
	load = scan_file(target);

	if ((load == NULL) && (c->directory)) {
		// Check at starting directory
		free(target);

		target = path_from_dir_base(c->directory, name);
		load = scan_file(target);
	}

	if (load == NULL) {
		free(target);
		return -1;
	}

	// Adjust search_directory
	free(*search_directory);
	split_path_file(search_directory, NULL, target);
	free(target);

	// Copy contents to partial DString
	free(partial->str);
	partial->str = load->str;
	partial->currentStringLength = load->currentStringLength;

	d_string_free(load, false);

	return 0;
}


// Print raw JSON
static int print_raw(const char * name, struct closure * closure) {
	JSON_Value * v = find(closure, name);

	if (v) {
		char * string = json_serialize_to_string(v);
		char * c = string;

		if (*c == '"') {
			c++;

			string[strlen(string) - 1] = '\0';
		}

		while (*c) {
			switch (*c) {
				case '\"':
					d_string_append_c(closure->out, '\\');
					d_string_append_c(closure->out, '\"');
					break;

				case '\\':
					if (*++c == '/') {

					} else {
						d_string_append_c(closure->out, '\\');
					}

				default:
					d_string_append_c(closure->out, *c);
					break;
			}

			if (*c) {
				c++;
			}
		}

		free(string);
	}

	return 0;
}


/// Replace designated range in source with value of `name`
static int print(const char * name, struct closure * c, int escape) {
	JSON_Value * v = find(c, name);
	const char * s;

	if (v) {
		switch (json_value_get_type(v)) {
			case JSONString:
				if (escape) {
					s = json_value_get_string(v);

					do {
						switch (*s) {
							case '>':
								d_string_append_c_array(c->out, "&gt;", 4);
								break;

							case '<':
								d_string_append_c_array(c->out, "&lt;", 4);
								break;

							case '&':
								d_string_append_c_array(c->out, "&amp;", 5);
								break;

							case '\"':
								d_string_append_c_array(c->out, "&quot;", 6);
								break;

							case '\0':
								// Empty string
								s--;
								break;

							default:
								d_string_append_c(c->out, *s);
								break;
						}
					} while (*++s);
				} else {
					d_string_append(c->out, json_value_get_string(v));
				}

				break;

			case JSONNumber:
				d_string_append_printf(c->out, "%g", json_value_get_number(v));
				break;

			default:
				break;
		}
	}

	return 0;
}


// Iterate to next instance of array
static int json_next(struct closure * c) {
	if (c->depth <= 0) {
		return -1;
	}

	c->stack[c->depth].index++;

	if (c->stack[c->depth].index >= c->stack[c->depth].count) {
		// Last one
		return 0;
	}

	// Move to next item in array
	JSON_Array * a = json_value_get_array(c->stack[c->depth].container);
	c->stack[c->depth].val = json_array_get_value(a, c->stack[c->depth].index);

	return 1;
}


// Move up one level in the object hierarchy
static int json_leave(struct closure * c) {
	if (c->depth <= 0) {
		return -1;
	}

	c->depth--;

	return 0;
}


static int json_enter(const char * name, struct closure * c) {
	JSON_Value * v = find(c, name);
	JSON_Array * a;

	if (c->depth == kMaxDepth) {
		return -1;
	}

	if (v == NULL) {
		return 0;
	}

	c->depth++;

	switch (json_value_get_type(v)) {
		case JSONArray:
			a = json_value_get_array(v);
			c->stack[c->depth].count = json_array_get_count(a);

			if (c->stack[c->depth].count == 0) {
				// Nothing to do
				c->depth--;
				return 0;
			}

			c->stack[c->depth].container = v;
			c->stack[c->depth].val = json_array_get_value(a, 0);
			c->stack[c->depth].index = 0;
			break;

		case JSONBoolean:
			if (!json_value_get_boolean(v)) {
				c->depth--;
				return 0;
			}

			c->stack[c->depth].count = 1;
			c->stack[c->depth].container = NULL;
			c->stack[c->depth].val = v;
			c->stack[c->depth].index = 0;
			break;

		case JSONNumber:
			if (!json_value_get_number(v)) {
				c->depth--;
				return 0;
			}

			c->stack[c->depth].count = 1;
			c->stack[c->depth].container = NULL;
			c->stack[c->depth].val = v;
			c->stack[c->depth].index = 0;
			break;

		case JSONString:
		case JSONObject:
			c->stack[c->depth].count = 1;
			c->stack[c->depth].container = NULL;
			c->stack[c->depth].val = v;
			c->stack[c->depth].index = 0;
			break;

		default:
			c->depth--;
			return 0;
	}

	return 1;
}


static int parse(DString * source, const char * opener, const char * closer, struct closure * closure, const char * search_directory) {
	if (source == NULL) {
		return -1;
	}

	int rc = 0;

	const char * start, * stop, * key;

	// Track breadcrumbs
	struct {
		const char *	key;
		size_t			key_len;
		const char * 	again;
		int 			entered;
		int				visible;
	} stack[kMaxDepth];

	int depth = 0;
	int visible = 1;

	char c;

	char op[kMaxDelimiterLength + 1] = {0};
	char cl[kMaxDelimiterLength + 1] = {0};

	strncpy(op, opener, MIN(kMaxDelimiterLength, strlen(opener)));
	strncpy(cl, closer, MIN(kMaxDelimiterLength, strlen(opener)));

	size_t open_len = strlen(op);
	size_t close_len = strlen(cl);

	char key_name[kMaxKeyLength + 1];
	size_t key_len;

	size_t l;

	DString * partial;
	char * dir;
	const char * indent;
	size_t indent_len;

	int standalone;

	// Find first tag
	start = strstr(source->str, op);
	stop = source->str;

	while (start) {
		// Copy anything before tag
		if (visible) {
			d_string_append_c_array(closure->out, stop, start - stop);
		}

		// Find end of tag
		stop = strstr(start + open_len, cl);

		if (stop == NULL) {
			// No end to this possible tag
			return -1;
		}

		// Is this a "standalone" tag? (e.g. on a line by itself)
		standalone = 0;
		key = start;

		while ((key > source->str) &&
				((*(key - 1) == ' ') || (*(key - 1) == '\t'))) {
			key--;
		}

		if ((key == source->str) ||
				((*(key - 1) == '\n') || (*(key - 1) == '\r'))) {
			// Check after tag
			key = stop + close_len;

			while ((*key == ' ') || (*key == '\t')) {
				key++;
			}

			if ((*key == '\n') || (*key == '\r') || (*key == '\0')) {
				standalone = 1;
			}
		}

		// Get key from contents of tag
		key = start + open_len;
		key_len = stop - key;

		c = *key;

		// What sort of key is it?
		switch (c) {
			case '!':
			case '=':
				break;

			case '{':

				// Ensure proper {{{foo}}} config
				for (l = 0; cl[l] == '}'; l++);

				if (cl[l]) {
					if (!key_len || key[key_len - 1] != '}') {
						return -1;
					}

					key_len--;
				} else {
					if (stop[l] != '}') {
						return -1;
					}

					stop++;
				}

				c = '&';

			case '#':
			case '/':
			case '&':
			case '^':
			case '>':
			case ':':	// Indicate that rest of name should be used verbatim -- implemented in <https://gitlab.com/jobol/mustach>
			case '$':
				// Remove leading character from key name
				key++;
				key_len--;

			default:

				// Get text of key
				// Trim whitespace
				while (key_len && isspace(key[0])) {
					key++;
					key_len--;
				}

				while (key_len && isspace(key[key_len - 1])) {
					key_len--;
				}

				if (key_len > kMaxKeyLength) {
					return -1;
				}

				memcpy(key_name, key, key_len);
				key_name[key_len] = '\0';

				break;
		}

		// Do something with this key
		switch (c) {
			case '!':
				// Comment

				break;

			case '=':

				// Set Delimiter
				if (key_len < 5 || key[key_len - 1] != '=') {
					return -1;
				}

				key++;
				key_len -= 2;

				for (l = 0; l < key_len && isspace(key[l]); l++);

				key += l;
				key_len -= l;

				for (l = 0; l < key_len && !isspace(key[l]); l++);

				if (l == key_len) {
					return -1;
				}

				strncpy(op, key, MIN(kMaxDelimiterLength, l));
				op[MIN(kMaxDelimiterLength, l)] = '\0';

				while (l < key_len && isspace(key[l])) {
					l++;
				}

				while (isspace(key[key_len - 1])) {
					key_len--;
				}

				if (l == key_len) {
					return -1;
				}

				open_len = strlen(op);

				strncpy(cl, key + l, MIN(kMaxDelimiterLength, key_len - l));
				cl[MIN(kMaxDelimiterLength, key_len - l)] = '\0';

				// Adjust stop for change in closer length
				stop += close_len;
				close_len = strlen(cl);
				stop -= close_len;

				break;

			case '^':
			case '#':

				// Begin section
				if (depth == kMaxDepth) {
					return -1;
				}

				rc = visible;

				if (visible) {
					if ((rc = json_enter(key_name, closure)) < 0) {
						// Error
					}
				}

				// Leave breadcrumbs so we can return
				stack[depth].key = key;
				stack[depth].key_len = key_len;
				stack[depth].again = stop;
				stack[depth].entered = rc;
				stack[depth].visible = visible;

				if ((c == '#') == (rc == 0)) {
					visible = 0;
				}

				depth++;

				break;

			case '/':

				// End section
				if ((depth-- == 0) ||
						(key_len != stack[depth].key_len) ||
						(memcmp(stack[depth].key, key, key_len))) {
					// Doesn't match breadcrumb
					return -1;
				}

				rc = visible && (stack[depth].entered ? json_next(closure) : 0);

				if (rc < 0) {
					return rc;
				}

				if (rc) {
					stop = stack[depth++].again;
				} else {
					visible = stack[depth].visible;

					if (visible && stack[depth].entered) {
						if ((rc = json_leave(closure)) < 0) {

						}
					}
				}

				break;

			case '>':

				//  Partial
				if (visible) {
					partial = d_string_new("");
					dir = my_strdup(search_directory);

					rc = (*(closure->load_partial))(key_name, partial, closure, &dir);

					if (standalone) {
						// Determine leading whitespace
						indent = start;
						indent_len = 0;

						while ((indent > source->str) &&
								((*(indent - 1) == ' ') || (*(indent - 1) == '\t'))) {
							indent--;
							indent_len++;
						}

						indent_text(partial, indent, indent_len);
					}

					if (rc == 0) {
						rc = parse(partial, "{{", "}}", closure, dir);
					}

					free(dir);
					d_string_free(partial, true);
				}

				break;

			case '$':

				// Get literal JSON
				if (visible) {
					print_raw(key_name, closure);
				}

				break;

			default:

				// Basic replacement
				if (visible) {
					print(key_name, closure, c != '&');
					standalone = 0;
				}

				break;
		}

		// Find next tag
		stop += close_len;
		start = strstr(stop, op);


		if (standalone) {
			// Trim leading whitespace
			if (closure->out->currentStringLength) {
				while (closure->out->currentStringLength &&
						((closure->out->str[closure->out->currentStringLength - 1] == ' ') ||
						 (closure->out->str[closure->out->currentStringLength - 1] == '\t'))) {
					d_string_erase(closure->out, closure->out->currentStringLength - 1, 1);
				}
			}

			// Trim trailing space
			while ((*stop == ' ') || (*stop == '\t')) {
				stop++;
			}

			if (*stop == '\r') {
				stop++;
			}

			if (*stop == '\n') {
				stop++;
			}
		}
	}

	// Copy anything after last tag
	if (visible && stop) {
		if (stop - source->str < source->currentStringLength) {
			d_string_append_c_array(closure->out, stop, source->currentStringLength - (stop - source->str));
		}
	}


	return rc;
}


/// Given a source string, populate it using data from a JSON value.
/// The resulting text will be appended to `out`.
int magnum_populate_from_json(DString * source, JSON_Value * json, DString * out, const char * search_directory, int (*load_p)(char *, DString *, struct closure *, char **)) {
	int rc;

	struct closure c;

	c.root = json;
	c.depth = 0;
	c.out = out;
	c.directory = search_directory;
	c.stack[0].container = NULL;
	c.stack[0].val = json;
	c.stack[0].index = 0;
	c.stack[0].count = 1;

	if (load_p) {
		c.load_partial = load_p;
	} else {
		c.load_partial = &load_partial;
	}

	rc = parse(source, "{{", "}}", &c, search_directory);

	if (rc < 0) {
		fprintf(stderr, "Error parsing Mustache templates\n");
	}

	return rc;
}


/// Given a source string, populate it using data from a JSON string.
/// The resulting text will be appended to `out`.
int magnum_populate_from_string(DString * source, const char * string, DString * out, const char * search_directory) {
	JSON_Value * v = json_parse_string(string);

	int rc = magnum_populate_from_json(source, v, out, search_directory, NULL);

	json_value_free(v);

	return rc;
}



/// Given a source string, populate it using data from a JSON string, using a custom load_partial routine
/// The resulting text will be appended to `out`.
int magnum_populate_from_string_custom_partial(DString * source, const char * string, DString * out, const char * search_directory, int (*load_p)(char *, DString *, struct closure *, char **)) {
	JSON_Value * v = json_parse_string(string);

	int rc = magnum_populate_from_json(source, v, out, search_directory, load_p);

	json_value_free(v);

	return rc;
}


/// Given a source string, populate it using data from a JSON file.
/// The resulting text will be appended to `out`.
int magnum_populate_from_file(DString * source, const char * fname, DString * out, const char * search_directory) {
	JSON_Value * v = json_parse_file(fname);

	int rc = magnum_populate_from_json(source, v, out, search_directory, NULL);

	json_value_free(v);

	return rc;
}


/// Simplified method to allow use without any other included files.
/// Useful if you have no other need for parson or d_string
int magnum_populate_char_only(const char * source, const char * string, char ** out, const char * search_directory) {
	JSON_Value * v = json_parse_string(string);

	DString * d_source = d_string_new("");
	free(d_source->str);
	d_source->str = (char *) source;

	DString * temp = d_string_new("");

	int rc = magnum_populate_from_json(d_source, v, temp, search_directory, NULL);

	json_value_free(v);

	*out = temp->str;
	d_string_free(temp, false);

	d_string_free(d_source, false);

	return rc;
}


#ifdef TEST
void Test_magnum(CuTest* tc) {
	DString * source = d_string_new("");
	DString * out = d_string_new("");

	// Basic flat test
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "A\n\n{{ foo }}\n\n{{bar}}\n\nB\n");
	magnum_populate_from_string(source, "{ \"foo\" : \"one\", \"bar\" : 42 }", out, NULL);
	CuAssertStrEquals(tc, "A\n\none\n\n42\n\nB\n", out->str);


	// Simple array
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{#items}}\n    <li><a href=\"{{url}}\">{{name}}</a></li>\n{{/items}}");
	magnum_populate_from_string(source, "{\n  \"items\": [\n      {\"name\": \"red\", \"first\": true, \"url\": \"#Red\"},\n      {\"name\": \"green\", \"link\": true, \"url\": \"#Green\"},\n      {\"name\": \"blue\", \"link\": true, \"url\": \"#Blue\"}\n  ]\n}",
								out, NULL);
	CuAssertStrEquals(tc, "    <li><a href=\"#Red\">red</a></li>\n    <li><a href=\"#Green\">green</a></li>\n    <li><a href=\"#Blue\">blue</a></li>\n",
					  out->str);

	// Array with value testing
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "<h1>{{header}}</h1>\n{{#bug}}\n{{/bug}}\n\n{{#items}}\n  {{#first}}\n    <li><strong>{{name}}</strong></li>\n  {{/first}}\n  {{#link}}\n    <li><a href=\"{{url}}\">{{name}}</a></li>\n  {{/link}}\n{{/items}}\n\n{{#empty}}\n  <p>The list is empty.</p>\n{{/empty}}\n\n");
	magnum_populate_from_string(source, "{\n  \"header\": \"Colors\",\n  \"items\": [\n      {\"name\": \"red\", \"first\": true, \"url\": \"#Red\"},\n      {\"name\": \"green\", \"link\": true, \"url\": \"#Green\"},\n      {\"name\": \"blue\", \"link\": true, \"url\": \"#Blue\"}\n  ],\n  \"empty\": false\n}\n",
								out, NULL);
	CuAssertStrEquals(tc, "<h1>Colors</h1>\n\n    <li><strong>red</strong></li>\n    <li><a href=\"#Green\">green</a></li>\n    <li><a href=\"#Blue\">blue</a></li>\n\n\n",
					  out->str);

	// Array with value testing
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{#empty}}\n  <p>The list is empty.</p>\n{{/empty}}\n\n");
	magnum_populate_from_string(source, "{\n  \"header\": \"Colors\",\n  \"items\": [\n      {\"name\": \"red\", \"first\": true, \"url\": \"#Red\"},\n      {\"name\": \"green\", \"link\": true, \"url\": \"#Green\"},\n      {\"name\": \"blue\", \"link\": true, \"url\": \"#Blue\"}\n  ],\n  \"empty\": false\n}\n",
								out, NULL);
	CuAssertStrEquals(tc, "\n",
					  out->str);

	// Non-false values
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{#person?}}Hi {{name}}!{{/person?}}");
	magnum_populate_from_string(source, "{\"person?\": { \"name\": \"Jon\" }}", out, NULL);
	CuAssertStrEquals(tc, "Hi Jon!", out->str);

	// Comments
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "Foo{{! This is a comment}}");
	magnum_populate_from_string(source, "{}", out, NULL);
	CuAssertStrEquals(tc, "Foo", out->str);


	// Change delimiter
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "* {{default_tags}}\n{{=<% %>=}}\n* <% erb_style_tags %>\n<%={{ }}=%>\n* {{ default_tags_again }}");
	magnum_populate_from_string(source, "{}", out, NULL);
	CuAssertStrEquals(tc, "* \n* \n* ", out->str);


	// Section for number
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{#value}}The value is {{value}}{{/value}}");
	magnum_populate_from_string(source, "{ \"value\" : 50 }", out, NULL);
	CuAssertStrEquals(tc, "The value is 50", out->str);

	d_string_erase(out, 0, -1);
	magnum_populate_from_string(source, "{ \"value\" : 0.0 }", out, NULL);
	CuAssertStrEquals(tc, "", out->str);


	// Section for string
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "{{#value}}The value is {{value}}{{/value}}");
	magnum_populate_from_string(source, "{ \"value\" : \"50\" }", out, NULL);
	CuAssertStrEquals(tc, "The value is 50", out->str);

	d_string_erase(out, 0, -1);
	magnum_populate_from_string(source, "{ \"value\" : \"\" }", out, NULL);
	CuAssertStrEquals(tc, "The value is ", out->str);

	d_string_erase(out, 0, -1);
	magnum_populate_from_string(source, "{  }", out, NULL);
	CuAssertStrEquals(tc, "", out->str);

	// Empty string
	d_string_erase(source, 0, -1);
	d_string_erase(out, 0, -1);
	d_string_append(source, "foo = \"{{foo}}\"\nbar = \"{{bar}}\"\n");
	magnum_populate_from_string(source, "{ \"foo\": \"\", \"bar\": \"BAR\"}", out, NULL);
	CuAssertStrEquals(tc, "foo = \"\"\nbar = \"BAR\"\n", out->str);


	d_string_free(source, true);
	d_string_free(out, true);
}
#endif

