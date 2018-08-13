## About ##

|	|	|  
| ----------	| -------------------------	|  
| Title:	| Magnum	|  
| Author:	| Fletcher T. Penney	|  
| Date:	| 2018-08-13	|  
| Copyright:	| Copyright © 2017-2018 Fletcher T. Penney.	|  
| Version:	| 1.2.0	|  


## Introduction ##

[Mustache] is a syntax for creating "logic-less templates". For example:

	Hello {{name}}
	You have just won {{value}} dollars!
	{{#in_ca}}
	Well, {{taxed_value}} dollars, after taxes.
	{{/in_ca}}

can be combined with some data:

	{
	  "name": "Chris",
	  "value": 10000,
	  "taxed_value": 6000,
	  "in_ca": true
	}

to generate the following:

	Hello Chris
	You have just won 10000 dollars!
	Well, 6000.0 dollars, after taxes.


The Mustache system is described as "logic-less", but in reality the
functionality provides for a great deal of flexibility.

Read more about the Mustache [syntax] here.


## Magnum ##

Magnum is a C implementation of a Mustache parser.  It uses JSON to provide
the data, which can be provided in the form of a text file, a text string, or
a [parson] `JSON_Value`.  (The command line implementation requires that the
JSON data and template both come from text files.)

The `magnum` command line utility expects two or more arguments.  The first is
a JSON file to provide the data.  The remaining are source template files,
which are processed in order.  The output is sent to stdout.

	magnum data.json source.txt > output.txt

Magnum was inspired by another C implementation of Mustache,
<https://gitlab.com/jobol/mustach>.  `mustach` is licensed  under the Apache
License, version 2.0:

<https://gitlab.com/jobol/mustach/blob/master/LICENSE-2.0.txt>


Magnum was written in order to:

*   Fix several issues in order to provide compliance with the formal
	specification

*	Remove dependencies on external libraries -- [parson] is included to provide
	JSON parsing rather than using an external library

*	Add the test suite from the formal spec to ensure compliance


## Differences From the Specification ##

There are (currently) a few divergences from the formal Mustache
specification:

*	Since Magnum is written in C (and for security reasons), it does not support
	lambdas (an optional, not required, feature of Mustache.)

*	Tags that start with `:` will treat the remainder of the tag as literal
	characters -- this provides `escaping` when a tag name requires a character
	that is otherwise used for the Mustache syntax.  This feature was developed
	in `mustach`.

*   Tags that start with `$` will insert the raw JSON as a text string. This
	is used in the `make_spec_test.must` file in order to generate the test
	suite for example. It may not otherwise be a widely useful function. This
	may require additional cleaning up of the resulting text depending on the
	JSON content and the target language.  The syntax for this feature may
	change in the future -- input welcome.


## FAQ ##

### Where Can I Learn More About the Mustache Syntax? ###

The description of the Mustache syntax is here:

<http://mustache.github.io/mustache.5.html>

The formal specification is here:

<https://github.com/mustache/spec>


### Why "Magnum"? ###

Because he had the coolest mustache of all time, of course! ;)


### How Do I Use Magnum in Conjunction With MultiMarkdown? ###

For now you can use `magnum` to preprocess a document before running it
through MultiMarkdown.

	magnum data.json template.mustache > text.mmd
	multimarkdown text.mmd > text.html

**CAUTION:**  The Mustache syntax `{{foo}}` is the same as the MultiMarkdown
syntax for transclusion.  Because the Mustache spec indicates that non-
matching tags will be removed, running text through `magnum` will remove any
transclusion tags from the MultiMarkdown text.  In this case, you should run
the file through MultiMarkdown first in order to perform transclusion:

	multimarkdown -t mmd text.mmd > text.mustache
	magnum data.json text.mustache > text2.mmd
	multimarkdown text2.mmd > text.html

(When MMD is unable to match a transclusion command to a file, it leaves the
request intact.  Therefore it will not remove Mustache syntax from the file,
unless the Mustache tag happens to match an existing file.)

If needed, Mustache includes the ability to change the tag delimiters to
something else.  This would remove the conflict with MMD transclusion.

	{{=<% %>=}}
	<% mustache tag %>


After some additional testing of Magnum, and experimenting with different
uses, I may incorporate Magnum directly into MultiMarkdown.


### How Do Partials Work in Magnum? ###

"Partials" allow you to reference other Mustache templates, much like
transclusion works in
[MultiMarkdown](https://github.com/fletcher/MultiMarkdown-6).

For example, including `{{> user.mustache}}` in a template, would then cause
the contents of the file `user.mustache` to be inserted, and then parsed for
additional Mustache syntax.

This allows you split complex templates across multiple reusable files.


### How Do I Iterate Over a Top-Level Array? ###

Given the data:

	[
		"foo",
		"bar",
		"baz"
	]

You can use the following Mustache template:

	{{#.}}
	* {{.}}
	{{/.}}

And generate this output:

	* foo
	* bar
	* baz


### What are the security implications of partials? ###

Partials (`{{> foo.mustache}}`) allow arbitrary read access to the file
system, IF the command line implementation is used, OR if a `search_directory`
is passed to one of the `magnum_populate` commands.  `magnum` is limited to
whatever system access privileges it is provided, so it cannot read anything
that you could not access via the command line already.

So the key thing is that if you are using `libMagnum` somewhere, pass NULL as
the `search_directory` parameter to prevent partials from being enabled, if
that is the behavior you desire.


## License ##

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

[parson]:	https://github.com/kgabis/parson
[Mustache]:	http://mustache.github.io/
[spec]: 	https://github.com/mustache/spec
[syntax]:	http://mustache.github.io/mustache.5.html
