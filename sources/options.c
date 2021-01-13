#include "options.h"
#include <stdio.h>

/*!
** @file
**
** @copyright
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
*/

bool process_options(int argc, const char * argv[], struct command_options * options) {
	options->retry_count = 3;
	options->exit_code = 0;
	options->write_output = true;
	options->emit_start = true;
	options->emit_finish = true;

	if (argc != 4) {
		fprintf(stderr,
			"Usage:\n"
			"    %s <endpoint> <identifier> <access_secret>\n"
			"\n"
			"Arguments:\n"
			"    <endpoint>       absolute HTTP(S) URL of the backend\n"
			"    <identifier>     backend-specific id. of job/task\n"
			"    <access_secret>  backend-provided access secret\n"
				, argv[0]
		);

		return (false);
	}

	options->backend_endpoint = argv[1];
	options->identifier = argv[2];
	options->access_secret = argv[3];

	return (true);
}

/*
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
**
** Distributed under the Boost Software License, Version 1.0. See accompanying
** file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
*/
