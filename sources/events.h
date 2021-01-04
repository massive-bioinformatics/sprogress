#ifndef ___HOPE_B6A2D7D909A44C1FB224DA9542421884_UUID___
#define ___HOPE_B6A2D7D909A44C1FB224DA9542421884_UUID___

#include "options.h"
#include <curl/curl.h>
#include <stdbool.h>

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
bool write_start_event(CURL * session, const struct command_options * options);

/*!
** @internal
**
** @brief
** ...TODO...
*/
bool write_finish_event(CURL * session, const struct command_options * options);

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
