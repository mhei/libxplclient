/*
 * Copyright © 2016-2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <json.h>

#include "xplclient.h"

static int open_search_socket(const struct in_addr * const if_addr, unsigned int if_flags)
{
	struct sockaddr_in addr;
	int s, one = 1, zero = 0;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
		return -1;

	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one)) == -1)
		goto err_out;

	if (if_flags & IFF_MULTICAST) {
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &zero, sizeof(zero)) == -1)
			goto err_out;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = if_addr->s_addr;

	if (bind(s, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
		goto err_out;

	return s;

err_out:
	close(s);
	return -1;
}

static int send_query(int s, const struct in_addr * const dst_addr, unsigned int port)
{
	struct sockaddr_in addr;
	char *httpmu_req = NULL;
	int httpmu_len;
	int rv = -1;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = dst_addr->s_addr;
	addr.sin_port = htons(port);

	httpmu_len = asprintf(&httpmu_req,
				"GET /api/device HTTP/1.0\r\n"
				"Host: %s:%u\r\n"
				"NT: i2se:iodevice\r\n"
				"Content-Type: application/json\r\n"
				"Content-Length: 2\r\n"
				"\r\n"
				"{}",
				inet_ntoa(*dst_addr), port);

	if (httpmu_len == -1)
		goto err_out;

	if (sendto(s, httpmu_req, httpmu_len, 0, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
		goto err_out;

	rv = 0;

err_out:
	free(httpmu_req);

	return rv;
}

static char *http_get_header(const char *buffer, const char *key)
{
	char *needle, *start, *end;
	int len, size;

	/* header field must follow a \r\n sequence, i.e. it starts at beginning of a line */
	if ((len = asprintf(&needle, "\r\n%s:", key)) == -1)
		return NULL;

	start = strcasestr(buffer, needle);
	if (!start)
		goto free_out;

	/* start position for next search */
	start += len;

	/* the next \r\n sequence following the header field name will terminate our line */
	end = strstr(start, "\r\n");
	if (!end)
		goto free_out;

	/* determine the size to copy */
	len = end - start;

	free(needle);
	return strndup(start, len);

free_out:
	free(needle);
	return NULL;
}

static char *trim(char *str)
{
	char *p = str;
	int l = strlen(p);

	while (isspace(p[l - 1]))
		p[--l] = 0;

	while (*p && isspace(*p))
		++p, --l;

	memmove(str, p, l + 1);
	return str;
}

static int recv_packet(int s, xplclient_search_devices_cb cb, void *cb_ctx)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	char buffer[1024];
	ssize_t len;
	char *body, *http_ct_len, *endptr;
	struct json_tokener *tok;
	struct json_object *root;
	int ct_len;

	len = recvfrom(s, &buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr *)&addr, &addrlen);
	if (len == -1) {
		/* no packets available */
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 1;

		/* real error */
		return -1;
	}

	body = strstr(buffer, "\r\n\r\n");
	if (!body)
		return -1;

	/* keep first CRLF to terminate last header line */
	body += 2;
	*body++ = '\0';
	*body++ = '\0';

	/* look for Content-Length header */
	http_ct_len = http_get_header(buffer, "Content-Length");
	if (!http_ct_len)
		return -1;

	http_ct_len = trim(http_ct_len);
	ct_len = strtol(http_ct_len, &endptr, 10);
	/* Safety checks:
	 * - len cannot be less or equal zero for valid JSON
	 * - len cannot be larger than packet length itself... yes, sloppy test
	 * - line must contain single numeric value since we trimmed it, anything else is garbage
	 * - client supplied content-length must match packet length
	 *
	 * Note: do free http_ct_len as long as endptr is used!
	 */
	if (ct_len <= 0 || ct_len > len || *endptr != '\0' || (len - (body - buffer) != ct_len)) {
		free(http_ct_len);
		return -1;
	}
	free(http_ct_len);

	tok = json_tokener_new();
	if (!tok)
		return -1;

	root = json_tokener_parse_ex(tok, body, ct_len);
#if JSON_C_MINOR_VERSION > 10
	if (json_tokener_get_error(tok) != json_tokener_success) {
#else
	if (!root) {
#endif
		json_tokener_free(tok);
		return -1;
	}

	json_tokener_free(tok);

	if (cb) {
		cb(cb_ctx, (struct sockaddr *)&addr, addrlen, root);
	} else {
		json_object_put(root);
	}

	return 0;
}

int xplclient_search_devices(xplclient_search_devices_cb cb, void *cb_ctx, const char *interface, const char *mc_address, unsigned int port, int timeout)
{
	struct ifaddrs *addrs, *addr;
	struct in_addr mc_addr;
	struct itimerspec its;
	struct pollfd *fds;
	int i, c = 0, rv = -1;

	/* prepare timer data */
	memset(&its, 0, sizeof(its));
	its.it_value.tv_sec = (timeout > 0) ? timeout : 3;

	/* prepare destination address */
	mc_addr.s_addr = inet_addr(mc_address ? : XPLCLIENT_DEFAULT_MC_GROUP);

	/* get network interface list */
	if (getifaddrs(&addrs) == -1)
		return -1;

	/* first iteration to count interfaces we are considering */
	for (addr = addrs; addr; addr = addr->ifa_next) {
		/* if restricted to a given interfaces skip over if not matching */
		if (interface && strcmp(interface, addr->ifa_name) != 0)
			continue;

		/* skip interfaces which unlikely connect to an XPL */
		if (addr->ifa_flags & (IFF_LOOPBACK | IFF_POINTOPOINT))
			continue;

		/* only consider IPv4 interfaces at the moment */
		if (addr->ifa_addr && addr->ifa_addr->sa_family == AF_INET)
			c++;
	}

	/* bail out if no usable interface is found */
	if (c == 0)
		goto err_out;

	/* get memory for all sockets to use: +1 for timer socket fd added later */
	fds = calloc(c + 1, sizeof(struct pollfd));
	if (!fds)
		goto err_out;

	/* setup a timer for timeout: we know that fds has reserved extra space for this fd */
	fds[c].fd = timerfd_create(CLOCK_MONOTONIC, 0);
	fds[c].events = POLLIN;
	if (fds[c].fd == -1)
		goto err_out;

	/* set to zero, otherwise we cannot detect errors */
	rv = 0;

	/* second iteration */
	for (addr = addrs, i = 0; addr; addr = addr->ifa_next) {
		/* if restricted to a given interfaces skip over if not matching */
		if (interface && strcmp(interface, addr->ifa_name) != 0)
			continue;

		/* skip interfaces which unlike connect to an XPL */
		if (addr->ifa_flags & (IFF_LOOPBACK | IFF_POINTOPOINT))
			continue;

		/* only consider IPv4 interfaces at the moment */
		if (addr->ifa_addr && addr->ifa_addr->sa_family == AF_INET) {

			fds[i].fd = open_search_socket(&((struct sockaddr_in *)(addr->ifa_addr))->sin_addr, addr->ifa_flags);
			fds[i].events = POLLIN;

			/* if socket is setup send query packet */
			if (fds[i].fd != -1) {
				rv |= send_query(fds[i].fd, (addr->ifa_flags & IFF_MULTICAST) ? &mc_addr :
						&((struct sockaddr_in *)(addr->ifa_broadaddr))->sin_addr, port ? : XPLCLIENT_DEFAULT_MC_PORT);
			}

			i++;
		}
	}

	/* any error occurred? */
	if (rv)
		goto close_out;

	/* this arms the timer now */
	rv = timerfd_settime(fds[c].fd, 0, &its, NULL);
	if (rv == -1)
		goto close_out;

	while (1) {
		rv = poll(fds, c + 1, -1);
		if (rv == -1)
			goto close_out;

		for (i = 0; i <= c; i++) {
			/* anything on this fd? */
			if (fds[i].revents == 0)
				continue;

			/* unexpected result? */
			if (fds[i].revents != POLLIN) {
				rv = -1;
				goto close_out;
			}

			/* timeout fd triggered */
			if (i == c)
				goto ok_out;

			/* received a packet on this socket so process it */
			while (recv_packet(fds[i].fd, cb, cb_ctx) == 0)
				/* process each packet available */
				;
		}
	}

ok_out:
	/* indicate success */
	rv = 0;

close_out:
	/* close all fds (including a timer fd at last position) */
	for (i = 0; i <= c; i++)
		if (fds[i].fd != -1)
			close(fds[i].fd);

	free(fds);

err_out:
	freeifaddrs(addrs);
	return rv;
}
