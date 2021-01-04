#include "buffer.h"
#include "output.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*!
** @file
**
** @copyright
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
*/

bool write_standard_input(CURL * session, const struct command_options * options) {
	char buffer[512];
	CURLcode status;

	size_t stamp = 0;
	while (true) {
		++stamp;

		ssize_t read_bytes = read(STDIN_FILENO, buffer, 512);
		if (read_bytes == 0) {
			break;
		}

		if (read_bytes < 0) {
			fprintf(stderr,
				"Could not read input: %s\n"
					, strerror(errno)
			);

			return (false);
		}

		//  1 byte for the map
		// 14 bytes for "action": "output"
		// 15 bytes for "stamp": <integer>
		//  6 bytes for "bytes": ...
		// 11 bytes for "access_key": ...
		// 11 bytes for "secret_key": ...
		size_t msgpack_length = 1 + 14 + 15 + 6 + 11 + 11;

		if (read_bytes < 256) {
			msgpack_length += 2 + read_bytes;
		} else if (read_bytes < 65536) {
			msgpack_length += 3 + read_bytes;
		} else {
			msgpack_length += 5 + read_bytes;
		}

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

		uint8_t * msgpack_buffer = resize_buffer(session, msgpack_length);
		uint8_t * buffer_cursor = msgpack_buffer;

		if (msgpack_buffer == NULL) {
				return (false);
		}

		static
		const char BUFFER_CHUNK1[] = {
			0x85, 0xA6, 0x61, 0x63, 0x74, 0x69, 0x6F, 0x6E,
			0xA6, 0x6F, 0x75, 0x74, 0x70, 0x75, 0x74, 0xA5,
			0x73, 0x74, 0x61, 0x6D, 0x70,
		};

		static
		const char BUFFER_CHUNK2[] = {
			0xA5, 0x62, 0x79, 0x74, 0x65, 0x73,
		};

		static
		const char BUFFER_CHUNK3[] = {
			0xAA, 0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x5F,
			0x6B, 0x65, 0x79,
		};

		static
		const char BUFFER_CHUNK4[] = {
			0xAA, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74, 0x5F,
			0x6B, 0x65, 0x79,
		};

		memcpy(buffer_cursor, BUFFER_CHUNK1, sizeof(BUFFER_CHUNK1));
		buffer_cursor += sizeof(BUFFER_CHUNK1);

		*buffer_cursor++ = 0xCF;
		*buffer_cursor++ = (stamp >> 56) & 0xFF;
		*buffer_cursor++ = (stamp >> 48) & 0xFF;
		*buffer_cursor++ = (stamp >> 40) & 0xFF;
		*buffer_cursor++ = (stamp >> 32) & 0xFF;
		*buffer_cursor++ = (stamp >> 24) & 0xFF;
		*buffer_cursor++ = (stamp >> 16) & 0xFF;
		*buffer_cursor++ = (stamp >> 8) & 0xFF;
		*buffer_cursor++ = stamp & 0xFF;

		memcpy(buffer_cursor, BUFFER_CHUNK2, sizeof(BUFFER_CHUNK2));
		buffer_cursor += sizeof(BUFFER_CHUNK2);

		if (read_bytes < 256) {
			*buffer_cursor++ = 0xC4;
			*buffer_cursor++ = read_bytes & 0xFF;
		} else if (read_bytes < 65536) {
			*buffer_cursor++ = 0xC5;
			*buffer_cursor++ = (read_bytes >> 8) & 0xFF;
			*buffer_cursor++ = read_bytes & 0xFF;
		} else {
			*buffer_cursor++ = 0xC6;
			*buffer_cursor++ = (read_bytes >> 24) & 0xFF;
			*buffer_cursor++ = (read_bytes >> 16) & 0xFF;
			*buffer_cursor++ = (read_bytes >> 8) & 0xFF;
			*buffer_cursor++ = read_bytes & 0xFF;
		}

		memcpy(buffer_cursor, buffer, read_bytes);
		buffer_cursor += read_bytes;

		memcpy(buffer_cursor, BUFFER_CHUNK3, sizeof(BUFFER_CHUNK3));
		buffer_cursor += sizeof(BUFFER_CHUNK3);

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

		memcpy(buffer_cursor, BUFFER_CHUNK4, sizeof(BUFFER_CHUNK4));
		buffer_cursor += sizeof(BUFFER_CHUNK4);

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

		if (http_response_code != 200) {
			return (false);
		}
	}

	return (true);
}

/*
** Copyright (c) Massive Bioinformatics ArGe Teknolojileri A.S.
** All rights reserved.
**
** Distributed under the Boost Software License, Version 1.0. See accompanying
** file LICENSE.txt or online copy at https://www.boost.org/LICENSE_1_0.txt.
*/
