/*
 * Copyright © 2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <json.h>
#include <curl/curl.h>

#include "xplclient.h"

struct curl_recv_data {
	size_t size;
	char *payload;
};

static size_t curl_recv_cb(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct curl_recv_data *d = (struct curl_recv_data *)userdata;
	size_t len = size * nmemb; /* data length */
	char *new_payload;

	/* try to expand buffer */
	new_payload = (char *)realloc(d->payload, d->size + len);
	if (!new_payload) {
		/* return with the data we have */
		return -1;
	} else {
		d->payload = new_payload;
	}

	/* append new data to now increased buffer */
	memcpy(&d->payload[d->size], ptr, len);

	d->size += len;

	return len;
}

struct json_object *xplclient_url_get(xplclient_t ctx, const char *path)
{
	struct json_object *root = NULL;
	struct json_tokener *tok;
	struct curl_recv_data data;
	char url[128];

	if (snprintf(url, sizeof(url), "%s%s", ctx->url_prefix, path) >= sizeof(url))
		return NULL;

	data.size = 0;
	data.payload = NULL;

	if (curl_easy_setopt(ctx->curl, CURLOPT_URL, url) != CURLE_OK)
		goto err_out;

	if (curl_easy_setopt(ctx->curl, CURLOPT_WRITEFUNCTION, curl_recv_cb) != CURLE_OK)
		goto err_out;

	if (curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, (void *)&data) != CURLE_OK)
		goto err_out;

	if (curl_easy_perform(ctx->curl) != CURLE_OK)
		goto err_out;

	if (!data.payload)
		goto err_out;

	tok = json_tokener_new();
	if (!tok)
		goto free_out;

	root = json_tokener_parse_ex(tok, data.payload, data.size);

	json_tokener_free(tok);

free_out:
	free(data.payload);

err_out:
	return root;
}
