if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set (CMAKE_INSTALL_PREFIX "/opt/mbi"
		CACHE PATH
			"Install path prefix, prepended onto all install directories."

		FORCE
	)
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT GENERATOR_IS_MULTI_CONFIG)
	set (CMAKE_BUILD_TYPE "Debug"
		CACHE STRING
			"Build type (configuration) of this build tree, possible values are: Debug Release RelWithDebInfo MinSizeRel."

		FORCE
	)
endif ()

##
## Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
## All rights reserved.
##
## Distributed under the Boost Software License, Version 1.0. See accompanying
## file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
##
