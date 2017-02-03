/*
 * Copyright © 2016-2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <json.h>

#include "xplclient.h"

struct sbs_ctx {
	char *serial;
	struct sockaddr *addr;
	socklen_t *addrlen;
	int found;
};

/* trim away whitespace a leading zeros */
static char *trim_serial(char *str)
{
	char *p = str;
	int l = strlen(p);

	while (isspace(p[l - 1]))
		p[--l] = 0;

	while (*p && (isspace(*p) || *p == '0'))
		++p, --l;

	memmove(str, p, l + 1);
	return str;
}

static int sbs_cb(void *ctx, const struct sockaddr *address, socklen_t addrlen, struct json_object *deviceinfo)
{
	struct sbs_ctx *sbs_ctx = (struct sbs_ctx *)ctx;
	struct json_object *serial = NULL;

#if JSON_C_MINOR_VERSION > 10
	json_object_object_get_ex(deviceinfo, "serial", &serial);
#else
	serial = json_object_object_get(deviceinfo, "serial");
#endif

	if (serial) {
		char *json_serial = strdup(json_object_get_string(serial));

		if (!json_serial)
			goto free_out;

		trim_serial(json_serial);

		if (strcasecmp(sbs_ctx->serial, json_serial) == 0) {
			/* count every matching device */
			sbs_ctx->found++;

			/* but only the first found wins - at least for this convinience helper */
			if (sbs_ctx->found == 1) {
				if (sbs_ctx->addr)
					memcpy(sbs_ctx->addr, address, addrlen);
				if (sbs_ctx->addrlen)
					*sbs_ctx->addrlen = addrlen;
			}
		}

		free(json_serial);
	}

free_out:
	json_object_put(deviceinfo);

	return 0;
}

int xplclient_search_by_serial(const char *serial, struct sockaddr *addr, socklen_t *addrlen)
{
	struct sbs_ctx ctx;
	int rv;

	ctx.serial = strdup(serial);
	if (!ctx.serial)
		return -1;

	trim_serial(ctx.serial);

	ctx.addr = addr;
	ctx.addrlen = addrlen;
	ctx.found = 0;

	rv = xplclient_search_devices(sbs_cb, (void *)&ctx, NULL, NULL, 0, 0);

	free(ctx.serial);

	if (rv)
		return rv;

	return ctx.found;
}
