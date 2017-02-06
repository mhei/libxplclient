/*
 * Copyright © 2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <json.h>

#include "xplclient.h"
#include "config.h"

int main(int argc, char *argv[])
{
	xplclient_t xpl;
	struct json_object *root, *value;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <host> <path> <key>\n", argv[0]);
		return 1;
	}

	xplclient_global_init();

	xpl = xplclient_new_by_url(argv[1]);
	if (!xpl) {
		perror("xplclient_new_by_url");
		return 1;
	}

	root = xplclient_url_get(xpl, argv[2]);
	if (!root) {
		fprintf(stderr, "Error accessing '%s'.\n", argv[2]);
		return 1;
	}

	value = xplclient_json_object_get_by_key(root, argv[3]);
	if (!value) {
		fprintf(stderr, "Key '%s' does not exist.\n", argv[3]);
		return 1;
	}

	switch (json_object_get_type(value)) {
	case json_type_string:
		/* representation of strings include quotation marks so handle this extra */
		printf("%s\n", json_object_get_string(value));
		break;
	default:
#if JSON_C_MINOR_VERSION > 10
		printf("%s\n", json_object_to_json_string_ext(value, JSON_C_TO_STRING_PRETTY));
#else
		printf("%s\n", json_object_to_json_string(value));
#endif
	}

	json_object_put(root);
	xplclient_free(xpl);

	return 0;
}
