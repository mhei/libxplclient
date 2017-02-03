/*
 * Copyright © 2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <string.h>

#include <json.h>

#include "xplclient.h"

struct json_object *xplclient_json_object_get_by_key(struct json_object *root, const char *key)
{
	int level = XPLCLIENT_JSON_OBJECT_GET_BY_KEY_MAXDEPTH;
	struct json_object *p = root;
	char *k, *d, *s;

	/* we modify key so create a scratch copy */
	k = strdup(key);
	if (!k)
		return NULL;

	/* s points to start of "path" element and we start at the beginning */
	s = k;

	/* now loop through the key "path" */
	while (level-- && (d = strchr(s, '/'))) {
		*d = '\0';

		/* p points to current json tree object */
#if JSON_C_MINOR_VERSION > 10
		if (!json_object_object_get_ex(p, s, &p)) {
			p = NULL;
			goto free_out; /* not found, so leave */
		}
#else
		p = json_object_object_get(p, s);
		if (p == NULL)
			goto free_out; /* not found, so leave */
#endif

		/* adjust new start */
		s = d + 1;
	}

#if JSON_C_MINOR_VERSION > 10
	if (!json_object_object_get_ex(p, s, &p))
		p = NULL; /* not found */
#else
	p = json_object_object_get(p, s);
#endif

free_out:
	free(k);
	return p;
}
