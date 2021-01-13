#include "buffer.h"
#include "events.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*!
** @file
**
** @copyright
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
*/

bool write_start_event(CURL * session, const struct command_options * options) {
	CURLcode status;

	//  1 byte for the map
	// 13 bytes for "action": "start"
	// 11 bytes for "access_key": ...
	// 11 bytes for "secret_key": ...
	size_t msgpack_length = 1 + 13 + 11 + 11;

	size_t identifier_length = strlen(options->identifier);
	if (identifier_length < 32) {
		msgpack_length += 1 + identifier_length;
	} else if (identifier_length < 256) {
		msgpack_length += 2 + identifier_length;
	} else if (identifier_length < 65536) {
		msgpack_length += 3 + identifier_length;
	} else {
		msgpack_length += 5 + identifier_length;
	}

	size_t access_secret_length = strlen(options->access_secret);
	if (access_secret_length < 32) {
		msgpack_length += 1 + access_secret_length;
	} else if (access_secret_length < 256) {
		msgpack_length += 2 + access_secret_length;
	} else if (access_secret_length < 65536) {
		msgpack_length += 3 + access_secret_length;
	} else {
		msgpack_length += 5 + access_secret_length;
	}

	uint8_t * msgpack_buffer = create_buffer(session, msgpack_length);
	uint8_t * buffer_cursor = msgpack_buffer;

	if (msgpack_buffer == NULL) {
		return (false);
	}

	static
	const char BUFFER_CHUNK1[] = {
		0x83, 0xA6, 0x61, 0x63, 0x74, 0x69, 0x6F, 0x6E,
		0xA5, 0x73, 0x74, 0x61, 0x72, 0x74, 0xAA, 0x61,
		0x63, 0x63, 0x65, 0x73, 0x73, 0x5F, 0x6B, 0x65,
		0x79,
	};

	static
	const char BUFFER_CHUNK2[] = {
		0xAA, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74, 0x5F,
		0x6B, 0x65, 0x79,
	};

	memcpy(buffer_cursor, BUFFER_CHUNK1, sizeof(BUFFER_CHUNK1));
	buffer_cursor += sizeof(BUFFER_CHUNK1);

	if (identifier_length < 32) {
		*buffer_cursor++ = 0xA0 | (((uint8_t) identifier_length) & 0x1F);
	} else if (identifier_length < 256) {
		*buffer_cursor++ = 0xD9;
		*buffer_cursor++ = identifier_length & 0xFF;
	} else if (identifier_length < 65536) {
		*buffer_cursor++ = 0xDA;
		*buffer_cursor++ = (identifier_length >> 8) & 0xFF;
		*buffer_cursor++ = identifier_length & 0xFF;
	} else {
		*buffer_cursor++ = 0xDB;
		*buffer_cursor++ = (identifier_length >> 24) & 0xFF;
		*buffer_cursor++ = (identifier_length >> 16) & 0xFF;
		*buffer_cursor++ = (identifier_length >> 8) & 0xFF;
		*buffer_cursor++ = identifier_length & 0xFF;
	}

	memcpy(buffer_cursor, options->identifier, identifier_length);
	buffer_cursor += identifier_length;

	memcpy(buffer_cursor, BUFFER_CHUNK2, sizeof(BUFFER_CHUNK2));
	buffer_cursor += sizeof(BUFFER_CHUNK2);

	if (access_secret_length < 32) {
		*buffer_cursor++ = 0xA0 | (((uint8_t) access_secret_length) & 0x1F);
	} else if (access_secret_length < 256) {
		*buffer_cursor++ = 0xD9;
		*buffer_cursor++ = access_secret_length & 0xFF;
	} else if (access_secret_length < 65536) {
		*buffer_cursor++ = 0xDA;
		*buffer_cursor++ = (access_secret_length >> 8) & 0xFF;
		*buffer_cursor++ = access_secret_length & 0xFF;
	} else {
		*buffer_cursor++ = 0xDB;
		*buffer_cursor++ = (access_secret_length >> 24) & 0xFF;
		*buffer_cursor++ = (access_secret_length >> 16) & 0xFF;
		*buffer_cursor++ = (access_secret_length >> 8) & 0xFF;
		*buffer_cursor++ = access_secret_length & 0xFF;
	}

	memcpy(buffer_cursor, options->access_secret, access_secret_length);
	buffer_cursor += access_secret_length;

	status = curl_easy_setopt(session, CURLOPT_POSTFIELDS, msgpack_buffer);
	if (status != CURLE_OK) {
		fprintf(stderr,
			"Could not set request body: %s\n"
				, curl_easy_strerror(status)
		);

		return (false);
	}

	status = curl_easy_setopt(session, CURLOPT_POSTFIELDSIZE, msgpack_length);
	if (status != CURLE_OK) {
		fprintf(stderr,
			"Could not set request body: %s\n"
				, curl_easy_strerror(status)
		);

		return (false);
	}

	long http_response_code = 0;
	size_t remaining_tries = options->retry_count;

	while (remaining_tries > 0 && http_response_code != 200) {
		--remaining_tries;

		status = curl_easy_perform(session);
		if (status != CURLE_OK) {
			if (remaining_tries >= 1) {
				continue;
			}

			fprintf(stderr,
				"Could not POST start event: %s\n"
					, curl_easy_strerror(status)
			);
		}

		status = curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &http_response_code);
		if (status != CURLE_OK) {
			if (remaining_tries >= 1) {
				continue;
			}

			fprintf(stderr,
				"Could not POST start event: %s\n"
					, curl_easy_strerror(status)
			);
		}

		if (remaining_tries >= 1) {
			if (http_response_code == 408 || http_response_code == 502 || http_response_code == 503 || http_response_code == 504) {
				continue;
			}
		}

		if (http_response_code != 200) {
			fprintf(stderr,
				"Could not POST start event: HTTP response code - %li\n"
					, http_response_code
			);

			return (false);
		}
	}

	return (http_response_code == 200);
}

bool write_finish_event(CURL * session, const struct command_options * options) {
	CURLcode status;

	//  1 byte for the map
	// 14 bytes for "action": "finish"
	// 19 bytes for "exit_code": <integer>
	// 11 bytes for "access_key": ...
	// 11 bytes for "secret_key": ...
	size_t msgpack_length = 1 + 14 + 19 + 11 + 11;

	size_t identifier_length = strlen(options->identifier);
	if (identifier_length < 32) {
		msgpack_length += 1 + identifier_length;
	} else if (identifier_length < 256) {
		msgpack_length += 2 + identifier_length;
	} else if (identifier_length < 65536) {
		msgpack_length += 3 + identifier_length;
	} else {
		msgpack_length += 5 + identifier_length;
	}

	size_t access_secret_length = strlen(options->access_secret);
	if (access_secret_length < 32) {
		msgpack_length += 1 + access_secret_length;
	} else if (access_secret_length < 256) {
		msgpack_length += 2 + access_secret_length;
	} else if (access_secret_length < 65536) {
		msgpack_length += 3 + access_secret_length;
	} else {
		msgpack_length += 5 + access_secret_length;
	}

	uint8_t * msgpack_buffer = create_buffer(session, msgpack_length);
	uint8_t * buffer_cursor = msgpack_buffer;

	if (msgpack_buffer == NULL) {
		return (false);
	}

	static
	const char BUFFER_CHUNK1[] = {
		0x84, 0xA6, 0x61, 0x63, 0x74, 0x69, 0x6F, 0x6E,
		0xA6, 0x66, 0x69, 0x6E, 0x69, 0x73, 0x68, 0xA9,
		0x65, 0x78, 0x69, 0x74, 0x5F, 0x63, 0x6F, 0x64,
		0x65,
	};

	static
	const char BUFFER_CHUNK2[] = {
		0xAA, 0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x5F,
		0x6B, 0x65, 0x79,
	};

	static
	const char BUFFER_CHUNK3[] = {
		0xAA, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74, 0x5F,
		0x6B, 0x65, 0x79,
	};

	memcpy(buffer_cursor, BUFFER_CHUNK1, sizeof(BUFFER_CHUNK1));
	buffer_cursor += sizeof(BUFFER_CHUNK1);

	*buffer_cursor++ = 0xD3;
	*buffer_cursor++ = (options->exit_code >> 56) & 0xFF;
	*buffer_cursor++ = (options->exit_code >> 48) & 0xFF;
	*buffer_cursor++ = (options->exit_code >> 40) & 0xFF;
	*buffer_cursor++ = (options->exit_code >> 32) & 0xFF;
	*buffer_cursor++ = (options->exit_code >> 24) & 0xFF;
	*buffer_cursor++ = (options->exit_code >> 16) & 0xFF;
	*buffer_cursor++ = (options->exit_code >> 8) & 0xFF;
	*buffer_cursor++ = options->exit_code & 0xFF;

	memcpy(buffer_cursor, BUFFER_CHUNK2, sizeof(BUFFER_CHUNK2));
	buffer_cursor += sizeof(BUFFER_CHUNK2);

	if (identifier_length < 32) {
		*buffer_cursor++ = 0xA0 | (((uint8_t) identifier_length) & 0x1F);
	} else if (identifier_length < 256) {
		*buffer_cursor++ = 0xD9;
		*buffer_cursor++ = identifier_length & 0xFF;
	} else if (identifier_length < 65536) {
		*buffer_cursor++ = 0xDA;
		*buffer_cursor++ = (identifier_length >> 8) & 0xFF;
		*buffer_cursor++ = identifier_length & 0xFF;
	} else {
		*buffer_cursor++ = 0xDB;
		*buffer_cursor++ = (identifier_length >> 24) & 0xFF;
		*buffer_cursor++ = (identifier_length >> 16) & 0xFF;
		*buffer_cursor++ = (identifier_length >> 8) & 0xFF;
		*buffer_cursor++ = identifier_length & 0xFF;
	}

	memcpy(buffer_cursor, options->identifier, identifier_length);
	buffer_cursor += identifier_length;

	memcpy(buffer_cursor, BUFFER_CHUNK3, sizeof(BUFFER_CHUNK3));
	buffer_cursor += sizeof(BUFFER_CHUNK3);

	if (access_secret_length < 32) {
		*buffer_cursor++ = 0xA0 | (((uint8_t) access_secret_length) & 0x1F);
	} else if (access_secret_length < 256) {
		*buffer_cursor++ = 0xD9;
		*buffer_cursor++ = access_secret_length & 0xFF;
	} else if (access_secret_length < 65536) {
		*buffer_cursor++ = 0xDA;
		*buffer_cursor++ = (access_secret_length >> 8) & 0xFF;
		*buffer_cursor++ = access_secret_length & 0xFF;
	} else {
		*buffer_cursor++ = 0xDB;
		*buffer_cursor++ = (access_secret_length >> 24) & 0xFF;
		*buffer_cursor++ = (access_secret_length >> 16) & 0xFF;
		*buffer_cursor++ = (access_secret_length >> 8) & 0xFF;
		*buffer_cursor++ = access_secret_length & 0xFF;
	}

	memcpy(buffer_cursor, options->access_secret, access_secret_length);
	buffer_cursor += access_secret_length;

	status = curl_easy_setopt(session, CURLOPT_POSTFIELDS, msgpack_buffer);
	if (status != CURLE_OK) {
		fprintf(stderr,
			"Could not set request body: %s\n"
				, curl_easy_strerror(status)
		);

		return (false);
	}

	status = curl_easy_setopt(session, CURLOPT_POSTFIELDSIZE, msgpack_length);
	if (status != CURLE_OK) {
		fprintf(stderr,
			"Could not set request body: %s\n"
				, curl_easy_strerror(status)
		);

		return (false);
	}

	long http_response_code = 0;
	size_t remaining_tries = options->retry_count;

	while (remaining_tries > 0 && http_response_code != 200) {
		--remaining_tries;

		status = curl_easy_perform(session);
		if (status != CURLE_OK) {
			if (remaining_tries >= 1) {
				continue;
			}

			fprintf(stderr,
				"Could not POST finish event: %s\n"
					, curl_easy_strerror(status)
			);
		}

		status = curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &http_response_code);
		if (status != CURLE_OK) {
			if (remaining_tries >= 1) {
				continue;
			}

			fprintf(stderr,
				"Could not POST finish event: %s\n"
					, curl_easy_strerror(status)
			);
		}

		if (remaining_tries >= 1) {
			if (http_response_code == 408 || http_response_code == 502 || http_response_code == 503 || http_response_code == 504) {
				continue;
			}
		}

		if (http_response_code != 200) {
			fprintf(stderr,
				"Could not POST finish event: HTTP response code - %li\n"
					, http_response_code
			);

			return (false);
		}
	}

	return (http_response_code == 200);
}

/*
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
**
** Distributed under the Boost Software License, Version 1.0. See accompanying
** file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
*/
