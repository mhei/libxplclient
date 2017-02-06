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

static struct json_object *do_curl_request(xplclient_t ctx, const char *path, struct json_object *data)
{
	struct curl_recv_data recvdata = { 0, NULL };
	struct json_object *root = NULL;
	struct json_tokener *tok;
	char url[128];
	CURL *curl;
	struct curl_slist *headers = NULL;

	if (snprintf(url, sizeof(url), "%s%s", ctx->url_prefix, path) >= sizeof(url))
		return NULL;

	curl = curl_easy_init();
	if (!curl)
		return NULL;

	headers = curl_slist_append(headers, "Accept: application/json");

	if (data) {
		headers = curl_slist_append(headers, "Content-Type: application/json");
	}

	if (!headers)
		goto free_out;

	if (curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers) != CURLE_OK)
		goto free_out;

	if (curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1) != CURLE_OK)
		goto free_out;

	if (curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK)
		goto free_out;

	if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_recv_cb) != CURLE_OK)
		goto free_out;

	if (curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&recvdata) != CURLE_OK)
		goto free_out;

	if (data) {
		if (curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(data)) != CURLE_OK)
			goto free_out;
	}

	if (curl_easy_perform(curl) != CURLE_OK)
		goto free_out;

	if (!recvdata.payload)
		goto free_out;

	tok = json_tokener_new();
	if (!tok)
		goto free_out;

	root = json_tokener_parse_ex(tok, recvdata.payload, recvdata.size);

	json_tokener_free(tok);

free_out:
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	free(recvdata.payload);

	return root;
}

struct json_object *xplclient_url_get(xplclient_t ctx, const char *path)
{
	return do_curl_request(ctx, path, NULL);
}

struct json_object *xplclient_url_set(xplclient_t ctx, const char *path, struct json_object *data)
{
	return do_curl_request(ctx, path, data);
}
