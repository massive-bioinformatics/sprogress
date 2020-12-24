if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/etc/build")
	list(APPEND CMAKE_MODULE_PATH
		"${CMAKE_CURRENT_SOURCE_DIR}/etc/build")
endif ()

# This option is actually declared in CTest module with default value of `ON`.
# Unless user wants to execute tests, it's not optimal to build the test suite.
option (BUILD_TESTING
	"Build the project test suite."
		NO
)

option (ENABLE_COVERAGE
	"Enable code coverage analysis and generate reports."
		NO
)

if (ENABLE_COVERAGE AND NOT BUILD_TESTING)
	message(FATAL_ERROR
		"ENABLE_COVERAGE option requires BUILD_TESTING turned on. Aborting..."
	)
endif ()

##
## Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
## All rights reserved.
##
## Distributed under the Boost Software License, Version 1.0. See accompanying
## file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
##
