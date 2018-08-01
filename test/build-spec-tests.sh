#!/bin/sh

# Build the spec_x.c test suite from the Mustache Spec
#
# https://github.com/mustache/spec

magnum comments.json make-spec-tests.mustache > ../src/spec_comments.c
magnum delimiters.json make-spec-tests.mustache > ../src/spec_delimiters.c
magnum interpolation.json make-spec-tests.mustache > ../src/spec_interpolation.c
magnum inverted.json make-spec-tests.mustache > ../src/spec_inverted.c
magnum magnum.json make-spec-tests.mustache > ../src/spec_magnum.c
magnum partials.json make-spec-tests.mustache > ../src/spec_partials.c
magnum sections.json make-spec-tests.mustache > ../src/spec_sections.c
