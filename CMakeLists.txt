#
#	Boilerplate CMake configuration for a cross-platform library, 
#	including configuration for a static framework on Xcode/macOS/iOS
#
#	Designed by Fletcher T. Penney
#	See the LICENSE file for copyright and licensing information.
#

cmake_minimum_required(VERSION 3.29)


# ===================
# Customize This Part
# ===================

# Project Description

set (My_Project_Title "libMagnum")
set (My_Project_Description "C implementation of Mustache logic-less templates")
set (My_Project_Author "Fletcher T. Penney")
set (My_Project_Revised_Date "2024-10-11")
set (My_Project_Version_Major 1)
set (My_Project_Version_Minor 3)
set (My_Project_Version_Patch 0)

set (My_Project_Copyright_Date "2017-2024")

set (My_Project_Identifier "net.multimarkdown.magnum")

string(TIMESTAMP My_Build_Version "%Y%m%d.%H.%M" UTC)
string(TIMESTAMP My_Build_Date "%Y-%m-%d" UTC)


# Source files and headers

set(src_files
	src/magnum.c

	src/d_string.c
	src/file.c
	src/json.c
	src/parson.c
)

set(public_headers
	src/libMagnum.h
)

set(private_headers
	src/d_string.h
	src/file.h
	src/json.h
	src/parson.h

	version.h
)

set(resources
)

source_group("Resources" FILES ${resources})

set(main_storyboard
)

# Include other CMake projects?
# add_subdirectory(submodules/foo)

# Libraries for linking
# on macOS can link to frameworks using: "-framework QuartzCore", for example
set(libraries_to_link
)

# This is an ugly hack, but it seems to work.
# When linking to a framework in Xcode, we need to be able to
# search for public headers within that framework
# "$SRCROOT/build-xcode/submodules/libFoo/$<CONFIG>/libFoo.framework/.."
# 
set(framework_search_paths
)

# Link to Cocoa Framework?
# set (link_to_cocoa 1)

# Use Assets for app icon set for iOS or macOS application?
# (Assets stored in resources/Assets.xcassets)
# set (use_assets 1)

# Enable ARC for Obj-C?
# set (enable_arc 1)


# =======================
# Leave the rest alone...
# =======================

# Configuration

set (My_Project_Version "${My_Project_Version_Major}.${My_Project_Version_Minor}.${My_Project_Version_Patch}")
set (My_Project_Copyright "Copyright © ${My_Project_Copyright_Date} ${My_Project_Author}.")

string(TOUPPER ${My_Project_Title} My_Project_Title_Caps  )
string(REGEX REPLACE " " "_" My_Project_Title_Caps ${My_Project_Title_Caps} )

string(REGEX REPLACE " " "" My_Project_Title_Clean ${My_Project_Title} )

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project (${My_Project_Title} VERSION "${My_Project_Version}")

if (CMAKE_GENERATOR MATCHES "Xcode")
	set (CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
endif()

# from http://stackoverflow.com/questions/25199677/how-to-detect-if-current-scope-has-a-parent-in-cmake
get_directory_property(hasParent PARENT_DIRECTORY)

if (hasParent)
else()
	# If building the framework independently, we can set a deployment target
	# set (CMAKE_OSX_DEPLOYMENT_TARGET "10.13")
endif()


# Search source directory
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/test)


# Search build directory (for version.h)
include_directories(${PROJECT_BINARY_DIR})


# Configure library/framework

add_library("${My_Project_Title_Clean}"
	STATIC
	${src_files}
	${private_headers}
	${public_headers}
	${resources}
)

string (REPLACE ";" " " framework_search_paths_string "${framework_search_paths}")

set_target_properties("${My_Project_Title_Clean}"
	PROPERTIES
	PREFIX ""
	FRAMEWORK_VERSION A
	MACOSX_FRAMEWORK_IDENTIFIER "${My_Project_Identifier}"
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${My_Project_Identifier}"
	XCODE_ATTRIBUTE_MARKETING_VERSION ${My_Project_Version}
	RESOURCE "${resources}"
	XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS} ${framework_search_paths_string}"
	PUBLIC_HEADER "${public_headers}"
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}

	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] YES
	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=MinSizeRel] NO
	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=RelWithDebInfo] NO
	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Release] NO

	XCODE_ATTRIBUTE_SDKROOT auto
	XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphoneos iphonesimulator macosx"
	XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
)

# Link to other libraries
target_link_libraries("${My_Project_Title_Clean}"
	${libraries_to_link}
)

# Link to Apple Cocoa Framework?
if (link_to_cocoa)
	target_link_libraries("${My_Project_Title_Clean}" "-framework Cocoa")
endif (link_to_cocoa)

if (hasParent)
else()
	# Use assets for application icons?
	if (use_assets)
		target_sources("${My_Project_Title_Clean}"
			PRIVATE
			resources/Assets.xcassets
		)

		set_source_files_properties(resources/Assets.xcassets
			PROPERTIES
			MACOSX_PACKAGE_LOCATION Resources
		)

		set_target_properties("${My_Project_Title_Clean}"
			PROPERTIES
			XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
		)
	endif (use_assets)

	# Enable ARC
	if (enable_arc)
		set_property (TARGET "${My_Project_Title_Clean}"
			APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc"
		)
	endif (enable_arc)
endif()


# ======================
# Process Template Files
# ======================

file(READ ${PROJECT_SOURCE_DIR}/LICENSE My_Project_License)

string(REGEX REPLACE "\n" "\n\t" My_Project_License_Indented ${My_Project_License})

string(REGEX REPLACE "\"" "\\\\\"" My_Project_License_Escaped ${My_Project_License_Indented})

string(REGEX REPLACE "\n" "\\\\n\"\\\\\n\"" My_Project_License_Literal ${My_Project_License_Escaped})


configure_file (
	"${PROJECT_SOURCE_DIR}/templates/template.c.in"
	"${PROJECT_BINARY_DIR}/template.c"
)

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/template.h.in"
	"${PROJECT_BINARY_DIR}/template.h"
)

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/main.c.in"
	"${PROJECT_BINARY_DIR}/main.c"
)

# Update the project README, to pull in new version #, etc.
configure_file (
	"${PROJECT_SOURCE_DIR}/templates/README.md.in"
	"${CMAKE_CURRENT_LIST_DIR}/README.md"
)

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/version.h.in"
	"${PROJECT_BINARY_DIR}/version.h"
)

configure_file (
	"${PROJECT_SOURCE_DIR}/templates/archive.sh.in"
	"${PROJECT_BINARY_DIR}/archive.sh"
)


# ===========================================
# Build Test Suite with CuTest (unit testing)
# ===========================================

# from http://stackoverflow.com/questions/25199677/how-to-detect-if-current-scope-has-a-parent-in-cmake
get_directory_property(hasParent PARENT_DIRECTORY)


set(test_files
	src/spec_comments.c
	src/spec_delimiters.c
	src/spec_interpolation.c
	src/spec_inverted.c
	src/spec_magnum.c
	src/spec_partials.c
	src/spec_sections.c

	test/CuTest.c
	test/CuTest.h
	${PROJECT_BINARY_DIR}/AllTests.c
)


# Process source files to look for tests to run
add_custom_command (
	OUTPUT ${PROJECT_BINARY_DIR}/AllTests.c
	COMMAND bash ${PROJECT_SOURCE_DIR}/test/make-tests.sh ${PROJECT_SOURCE_DIR}/src/*.c > ${PROJECT_BINARY_DIR}/AllTests.c
)


if(hasParent)
	# Don't create "run_tests" when we're a sub-project for something else
else()
	# Only create framework if we are the parent project
	set_target_properties("${My_Project_Title_Clean}"
		PROPERTIES
		FRAMEWORK TRUE
	)

	if (DEFINED TEST)
		add_definitions(-DTEST)

		add_executable(run_tests
			${src_files}
			${private_headers}
			${public_headers}
			${test_files}
		)

		# Link to other libraries
		target_link_libraries(run_tests
			${libraries_to_link}
		)

		# Link to Apple Cocoa Framework?
		if (link_to_cocoa)
			target_link_libraries(run_tests "-framework Cocoa")
		endif (link_to_cocoa)

		# Enable ARC
		if (enable_arc)
			set_property (TARGET run_tests
				APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc"
			)
		endif (enable_arc)

		enable_testing()

		add_test( test ${PROJECT_BINARY_DIR}/run_tests)

		# valgrind memory testing
	#	find_program (MEMORYCHECK_COMMAND valgrind)
	#	SET (MEMORYCHECK_COMMAND_OPTIONS --leak-check=full --error-exitcode=1)
	#
	#	add_test( memory_test ${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS} ${PROJECT_BINARY_DIR}/run_tests)

	else()
		add_executable(magnum
			src/main.c
		)

		target_link_libraries(magnum PRIVATE "${My_Project_Title}")

		set_target_properties(magnum
			PROPERTIES
			PREFIX ""
			XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${My_Project_Identifier}"
			XCODE_ATTRIBUTE_MARKETING_VERSION ${My_Project_Version}
			RESOURCE "${resources}"

			XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS} ${framework_search_paths_string}"

			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] YES
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=MinSizeRel] NO
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=RelWithDebInfo] NO
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Release] NO
		)
	endif()
endif()


# ==========================
# Build Installer with CPack
# ==========================

set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "${My_Project_Description}")
set (CPACK_PACKAGE_VERSION "${My_Project_Version_Major}.${My_Project_Version_Minor}.${My_Project_Version_Patch}")
set (CPACK_PACKAGE_VERSION_MAJOR "${My_Project_Version_Major}")
set (CPACK_PACKAGE_VERSION_MINOR "${My_Project_Version_Minor}")
set (CPACK_PACKAGE_VERSION_PATCH "My_Project_Version_Patch")

configure_file (
	"${PROJECT_SOURCE_DIR}/LICENSE"
	"${PROJECT_BINARY_DIR}/LICENSE.txt"
)
configure_file (
	"${PROJECT_SOURCE_DIR}/README.md"
	"${PROJECT_BINARY_DIR}/README.txt"
)

set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/LICENSE.txt")
set (CPACK_RESOURCE_FILE_README "${PROJECT_BINARY_DIR}/README.txt")

if (DEFINED TEST)
	# Don't install the test suite
else()
	if (APPLE)
		# Apple Builds

		if (DEFINED ZIP)
			set (CPACK_GENERATOR ZIP)
		else (DEFINED ZIP)
			set (CPACK_GENERATOR productbuild)
		endif (DEFINED ZIP)
	else (APPLE)
		if (WIN32)
			# Windows Builds

			if (DEFINED ZIP)
				set (CPACK_GENERATOR ZIP)
			else (DEFINED ZIP)
				set (CPACK_GENERATOR NSIS)
			endif (DEFINED ZIP)
		elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			# Linux
			set (CPACK_GENERATOR ZIP)
		endif (WIN32)
	endif (APPLE)

	# Install Target
	install (TARGETS magnum
		DESTINATION bin
		COMPONENT Main
	)
	set (CPACK_COMPONENT_MAIN_DISPLAY_NAME "Magnum")
	set (CPACK_COMPONENT_MAIN_DESCRIPTION "Install magnum.")

	set (CPACK_COMPONENTS_ALL Main)

	# Install Documentation
	install(FILES
		"${PROJECT_BINARY_DIR}/LICENSE.txt"
		"${PROJECT_BINARY_DIR}/README.txt"
		COMPONENT Docs
		DESTINATION share/doc/${My_Project_Title}
	)
	set (CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation")
	set (CPACK_COMPONENT_DOCS_DESCRIPTION "Install README and LICENSE.")

endif (DEFINED TEST)

include(CPack)
