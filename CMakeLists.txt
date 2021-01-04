cmake_minimum_required (VERSION 3.13 FATAL_ERROR)

project (sprogress
	VERSION 1.0.0
)

include ("${CMAKE_CURRENT_SOURCE_DIR}/etc/build/bootstrap.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/etc/build/standard.cmake")
include ("${CMAKE_CURRENT_SOURCE_DIR}/etc/build/toolchain.cmake")

find_package(CURL)
if (NOT CURL_FOUND)
	message(FATAL_ERROR
		"Could not find the CURL library and development files. Aborting..."
	)
endif ()

add_executable (sprogress
	"${CMAKE_CURRENT_SOURCE_DIR}/sources/buffer.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/sources/events.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/sources/main.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/sources/options.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/sources/output.c"
)

target_link_libraries (sprogress
	PRIVATE
		CURL::libcurl
)

target_include_directories (sprogress
	PRIVATE
		$<BUILD_INTERFACE:
			"${CMAKE_CURRENT_SOURCE_DIR}/headers"
			"${CMAKE_CURRENT_SOURCE_DIR}/sources"
		>
)

##
## Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
## All rights reserved.
##
## Distributed under the Boost Software License, Version 1.0. See accompanying
## file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
##
