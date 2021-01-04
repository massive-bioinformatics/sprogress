#ifndef ___HOPE_9F38767EC43D4572B0AD7D097DF5CB93_UUID___
#define ___HOPE_9F38767EC43D4572B0AD7D097DF5CB93_UUID___

#include <stdbool.h>
#include <stddef.h>

/*!
** @file
**
** @copyright
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
*/

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @internal
**
** @brief
** ...TODO...
*/
struct command_options {
	/*!
	**
	** @brief
	** ...TODO...
	*/
	const char * backend_endpoint;

	/*!
	**
	** @brief
	** ...TODO...
	*/
	const char * identifier;

	/*!
	**
	** @brief
	** ...TODO...
	*/
	const char * access_secret;

	/*!
	**
	** @brief
	** ...TODO...
	*/
	size_t retry_count;

	/*!
	**
	** @brief
	** ...TODO...
	*/
	bool write_output;

	/*!
	**
	** @brief
	** ...TODO...
	*/
	bool emit_start;

	/*!
	**
	** @brief
	** ...TODO...
	*/
	bool emit_finish;
};

/*!
** @internal
**
** @brief
** ...TODO...
*/
bool process_options(int argc, const char * argv[], struct command_options * options);

#ifdef __cplusplus
} /* extern */
#endif

/*
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
**
** Distributed under the Boost Software License, Version 1.0. See accompanying
** file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
*/
#endif
