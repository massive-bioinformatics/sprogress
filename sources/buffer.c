#include "buffer.h"
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

void * create_buffer(CURL * session, size_t size) {
	CURLcode status;
	void * buffer;

	status = curl_easy_getinfo(session, CURLINFO_PRIVATE, &buffer);
	if (status != CURLE_OK) {
		fprintf(stderr,
			"Could not read private data: %s\n"
				, curl_easy_strerror(status)
		);

		return (NULL);
	}

	if (buffer != NULL) {
		return (resize_buffer(session, size));
	}

	buffer = malloc(size);
	if (buffer == NULL) {
		fprintf(stderr,
			"Could not allocate memory.\n"
		);

		return (NULL);
	}

	status = curl_easy_setopt(session, CURLOPT_PRIVATE, buffer);
	if (status != CURLE_OK) {
		free(buffer);

		fprintf(stderr,
			"Could not write private data: %s\n"
				, curl_easy_strerror(status)
		);

		return (NULL);
	}

	return (buffer);
}

void * resize_buffer(CURL * session, size_t size) {
	CURLcode status;
	void * buffer;

	status = curl_easy_getinfo(session, CURLINFO_PRIVATE, &buffer);
	if (status != CURLE_OK) {
		fprintf(stderr,
			"Could not read private data: %s\n"
				, curl_easy_strerror(status)
		);

		return (NULL);
	}

	if (buffer == NULL) {
		return (create_buffer(session, size));
	}

	void * sized_buffer = realloc(buffer, size);
	if (sized_buffer == NULL) {
		fprintf(stderr,
			"Could not re-allocate memory.\n"
		);

		return (NULL);
	}

	status = curl_easy_setopt(session, CURLOPT_PRIVATE, sized_buffer);
	if (status != CURLE_OK) {
		if (sized_buffer != buffer) {
			free(sized_buffer);
		}

		fprintf(stderr,
			"Could not write private data: %s\n"
				, curl_easy_strerror(status)
		);

		return (NULL);
	}

	return (sized_buffer);
}

/*
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
**
** Distributed under the Boost Software License, Version 1.0. See accompanying
** file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
*/
