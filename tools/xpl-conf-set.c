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
	struct json_object *root, *value, *data;
	int i;

	if ((argc < 5) || ((argc - 5) % 2 != 0)) {
		fprintf(stderr, "Usage: %s <host> <path> <key> <value> [<key> <value>...]\n", argv[0]);
		return 1;
	}

	xplclient_global_init();

	xpl = xplclient_new_by_url(argv[1]);
	if (!xpl) {
		perror("xplclient_new_by_url");
		return 1;
	}

	data = json_object_new_object();
	if (!data) {
		perror("json_object_new_object");
		return 1;
	}

	for (i = 3; i < argc; i += 2) {
		long long int ll;
		char *endptr;

		ll = strtoll(argv[i + 1], &endptr, 0);
		if (*endptr == '\0')
			json_object_object_add(data, argv[i], json_object_new_int64(ll));
		else
			json_object_object_add(data, argv[i], json_object_new_string(argv[i + 1]));
	}

	root = xplclient_url_set(xpl, argv[2], data);

	json_object_put(data);

	if (!root) {
		fprintf(stderr, "Error accessing '%s'.\n", argv[2]);
		return 1;
	}

	if (argc == 5) {
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
			printf("%s\n", json_object_to_json_string_ext(value, JSON_C_TO_STRING_PRETTY));
		}
	} else {
		/* print whole object */
		printf("%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
	}

	json_object_put(root);
	xplclient_free(xpl);

	return 0;
}
