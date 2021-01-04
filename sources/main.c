#include "events.h"
#include "options.h"
#include "output.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

/*!
** @file
**
** @copyright
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
*/

#ifdef RELEASE_AND_RETURN
#	undef RELEASE_AND_RETURN
#endif

#define RELEASE_AND_RETURN(___exit_code___, ___message___, ...) do {           \
    void * data;                                                               \
    if (curl_easy_getinfo(session, CURLINFO_PRIVATE, &data) == CURLE_OK) {     \
        free(data);                                                            \
    }                                                                          \
                                                                               \
    curl_slist_free_all(headers);                                              \
    curl_easy_cleanup(session);                                                \
    curl_global_cleanup();                                                     \
                                                                               \
    const char * message = (___message___);                                    \
    if (message != NULL) {                                                     \
        fprintf(stderr, message, __VA_ARGS__);                                 \
    }                                                                          \
                                                                               \
    return (___exit_code___);                                                  \
} while(0)                                                             /* END */

/*!
** @internal
**
** @brief
** ...TODO...
*/
static
size_t echo_dev_null(char * output, size_t unit_size, size_t length, void * user_data);

int main(int argc, const char * argv[]) {
	CURL * session;
	CURLcode status;

	struct command_options options;
	if (!process_options(argc, argv, &options)) {
		return (EXIT_FAILURE);
	}

	status = curl_global_init(CURL_GLOBAL_ALL);
	if (status != CURLE_OK) {
		fprintf(stderr,
			"Could not initialise the CURL library: %s\n"
				, curl_easy_strerror(status)
		);

		return (EXIT_FAILURE);
	}

	session = curl_easy_init();
	if (session == NULL) {
		curl_global_cleanup();

		fprintf(stderr,
			"Could not initialise CURL handle. Aborting...\n"
		);

		return (EXIT_FAILURE);
	}

	struct curl_slist * headers = curl_slist_append(NULL, "Content-Type: application/msgpack");
	if (headers == NULL) {
		RELEASE_AND_RETURN(EXIT_FAILURE,
			"Could not allocate HTTP headers.\n", 0
		);
	}

	status = curl_easy_setopt(session, CURLOPT_TCP_KEEPALIVE, 1L);
	if (status != CURLE_OK) {
		RELEASE_AND_RETURN(EXIT_FAILURE,
			"Could not enable TCP keep-alive: %s\n"
				, curl_easy_strerror(status)
		);
	}

	status = curl_easy_setopt(session, CURLOPT_WRITEFUNCTION, echo_dev_null);
	if (status != CURLE_OK) {
		RELEASE_AND_RETURN(EXIT_FAILURE,
			"Could not set output callback: %s\n"
				, curl_easy_strerror(status)
		);
	}

	status = curl_easy_setopt(session, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
	if (status != CURLE_OK) {
		RELEASE_AND_RETURN(EXIT_FAILURE,
			"Could not limit allowed protocols: %s\n"
				, curl_easy_strerror(status)
		);
	}

	status = curl_easy_setopt(session, CURLOPT_POST, 1L);
	if (status != CURLE_OK) {
		RELEASE_AND_RETURN(EXIT_FAILURE,
			"Could not set request method: %s\n"
				, curl_easy_strerror(status)
		);
	}

	status = curl_easy_setopt(session, CURLOPT_HTTPHEADER, headers);
	if (status != CURLE_OK) {
		RELEASE_AND_RETURN(EXIT_FAILURE,
			"Could not set request headers: %s\n"
				, curl_easy_strerror(status)
		);
	}

	status = curl_easy_setopt(session, CURLOPT_URL, options.backend_endpoint);
	if (status != CURLE_OK) {
		RELEASE_AND_RETURN(EXIT_FAILURE,
			"Could not set backend URL: %s\n"
				, curl_easy_strerror(status)
		);
	}

	if (options.emit_start && !write_start_event(session, &options)) {
		RELEASE_AND_RETURN(EXIT_FAILURE, NULL, 0);
	}

	if (options.write_output && !write_standard_input(session, &options)) {
		RELEASE_AND_RETURN(EXIT_FAILURE, NULL, 0);
	}

	if (options.emit_finish && !write_finish_event(session, &options)) {
		RELEASE_AND_RETURN(EXIT_FAILURE, NULL, 0);
	}

	RELEASE_AND_RETURN(EXIT_SUCCESS, NULL, 0);
}

static
size_t echo_dev_null(char * output, size_t unit_size, size_t length, void * user_data) {
	return (length);
}

/*
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
**
** Distributed under the Boost Software License, Version 1.0. See accompanying
** file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
*/
