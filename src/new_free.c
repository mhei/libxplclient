/*
 * Copyright © 2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include <curl/curl.h>

#include "xplclient.h"

static int ctx_init_curl(xplclient_t ctx)
{
	/* init cURL context */
	ctx->curl = curl_easy_init();
	if (!ctx->curl)
		return -1;

	ctx->headers = curl_slist_append(ctx->headers, "Accept: application/json");
	if (!ctx->headers)
		goto err_out;

	if (curl_easy_setopt(ctx->curl, CURLOPT_HTTPHEADER, ctx->headers) != CURLE_OK)
		goto free_out;

	if (curl_easy_setopt(ctx->curl, CURLOPT_FOLLOWLOCATION, 1) != CURLE_OK)
		goto free_out;

	return 0;

free_out:
	curl_slist_free_all(ctx->headers);
err_out:
	curl_easy_cleanup(ctx->curl);
	return -1;
}

xplclient_t xplclient_new_by_addr(const struct sockaddr *addr, socklen_t addrlen)
{
	struct sockaddr_storage sa;
	struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)&sa;
	struct sockaddr_in *sa4 = (struct sockaddr_in *)&sa;
	char host[64]; /* > INET6_ADDRSTRLEN */
	char port[8]; /* > 5 digits max */
	char url[64];
	xplclient_t ctx;

	ctx = calloc(1, sizeof(struct xplclient));
	if (!ctx)
		return NULL;

	/* since we want to modify addr use a scratch copy */
	memcpy(&sa, addr, addrlen);

	/* we assume standard http port */
	switch (sa.ss_family) {
	case AF_INET:
		sa4->sin_port = htons(80);
		break;
	case AF_INET6:
		sa6->sin6_port = htons(80);
		break;
	}

	/* this is protocol independed */
	if (getnameinfo((struct sockaddr *)&sa, addrlen, host, sizeof(host), port, sizeof(port),
	                NI_NUMERICHOST | NI_NUMERICSERV) != 0)
		goto free_out;

	/* build our url... */
	if (snprintf(url, sizeof(url), "http://%s:%s/api", host, port) >= sizeof(url))
		goto free_out; /* buffer too small -> URL truncated -> bail out */

	/* ...copy buffer */
	ctx->url_prefix = strdup(url);
	if (!ctx->url_prefix)
		goto free_out;

	if (ctx_init_curl(ctx) == -1)
		goto free_out;

	return ctx;

free_out:
	free(ctx);
	return NULL;
}

xplclient_t xplclient_new_by_url(const char *url)
{
	xplclient_t ctx;

	ctx = calloc(1, sizeof(struct xplclient));
	if (!ctx)
		return NULL;

	/* ...copy url */
	ctx->url_prefix = strdup(url);
	if (!ctx->url_prefix)
		goto free_out;

	if (ctx_init_curl(ctx) == -1)
		goto free_out;

	return ctx;

free_out:
	free(ctx);
	return NULL;
}

void xplclient_free(xplclient_t ctx)
{
	if (!ctx)
		return;

	free(ctx->url_prefix);
	curl_easy_cleanup(ctx->curl);
	curl_slist_free_all(ctx->headers);

	free(ctx);
}
