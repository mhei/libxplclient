/*
 * Copyright © 2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <json.h>

#include "xplclient.h"

int xplclient_socket_by_serial(const char *serial, unsigned int comport)
{
	struct sockaddr_storage sa;
	struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)&sa;
	struct sockaddr_in *sa4 = (struct sockaddr_in *)&sa;
	socklen_t addrlen;
	xplclient_t xpl;
	char path[64];
	struct json_object *root, *val;
	int port, mode;
	int rv, s = -1;

	/* search for device */
	rv = xplclient_search_by_serial(serial, (struct sockaddr *)&sa, &addrlen);
	if (rv < 0)
		return -1;
	/* if no device is found with this serial we adjust errno */
	if (rv == 0) {
		errno = ENODEV;
		return -1;
	}

	/* create new context */
	xpl = xplclient_new_by_addr((struct sockaddr *)&sa, addrlen);
	if (!xpl)
		return -1;

	if (snprintf(path, sizeof(path), "/channel/physical/serial/%u", comport) >= sizeof(path))
		goto free1_out;

	/* get JSON COM port object */
	root = xplclient_url_get(xpl, path);
	if (!root) {
		errno = ENXIO;
		goto free1_out;
	}

	/* get currently configured port for remote access */
	val = xplclient_json_object_get_by_key(root, "port");
	if (!val)
		goto free2_out;
	port = json_object_get_int(val);

	/* simple sanity checks for port */
	if (port <= 0 || port > 65535)
		goto free2_out;

	/* check whether remote access is enable at least */
	val = xplclient_json_object_get_by_key(root, "mode");
	if (!val)
		goto free2_out;
	mode = json_object_get_int(val);

	/* mode has bit flags:
	 *   0x1 = virtual channel
	 *   0x2 = raw socket
	 *   0x4 = telnet
	 */
	if (mode <= 1) {
		errno = EINVAL;
		goto free2_out;
	}

	/* now create a socket... */
	s = socket(sa.ss_family, SOCK_STREAM, 0);
	if (s == -1)
		goto free2_out;

	switch (sa.ss_family) {
	case AF_INET:
		sa4->sin_port = htons(port);
		break;
	case AF_INET6:
		sa6->sin6_port = htons(port);
		break;
	default:
		goto close_out;
	}

	/* ... and finally let's connect to this port */
	if (connect(s, (struct sockaddr *)&sa, addrlen) == -1)
		goto close_out;

	/* everything ok, jump over the following close part */
	goto free2_out;

close_out:
	close(s);
	s = -1;

free2_out:
	json_object_put(root);

free1_out:
	xplclient_free(xpl);
	return s;
}
