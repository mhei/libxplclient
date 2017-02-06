/*
 * Copyright © 2016-2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */
#ifndef XPLCLIENT_H
#define XPLCLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

#include <curl/curl.h>
#include <json.h>

#include "xplclient-version.h"

#define XPLCLIENT_DEFAULT_MC_PORT 4109
#define XPLCLIENT_DEFAULT_MC_GROUP "239.255.255.250"

#define XPLCLIENT_JSON_OBJECT_GET_BY_KEY_MAXDEPTH 8

/**
 * Callback function type used by xplclient_search_devices.
 *
 * @param ctx        Context parameter passed to xplclient_search_devices.
 * @param address    Address of the XPL device which responded.
 * @param addrlen    This argument specifies the size of address.
 * @param deviceinfo Pointer to the root JSON object of the NOTIFY response. Callee is responsible to
 *                   free the object!
 * @return Return value is ignored at the moment, however, return 0 on sucess, -1 on error.
 */
typedef int (*xplclient_search_devices_cb)(void *ctx, const struct sockaddr *address, socklen_t addrlen, struct json_object *deviceinfo);

/**
 * Search for XPL devices in local network(s).
 *
 * If no explicite interface name is given, all available interfaces are used to send out a multicast query.
 * XPL devices usually respond to such queries with a NOTIFY message which contain few device informations.
 * Caller can specify a non-standard multicast address and/or port, if not given, then default values are used.
 *
 * @param cb         Callback function which is called for every found device.
 * @param cb_ctx     Context parameter passed to the callback function as first parameter.
 * @param interface  Name of the interface to use, if NULL is given, then all interfaces are searched in parallel.
 * @param mc_address Multicast address to use when sending the queries, use NULL to use default address.
 * @param port       UDP port to use for the multicast query, use zero to use default value.
 * @param timeout    Timeout in seconds for collecting responses, a value of zero or below zero results in the default of 3s.
 * @return Zero on success, -1 with errno set on error.
 */
int xplclient_search_devices(xplclient_search_devices_cb cb, void *cb_ctx, const char *interface, const char *mc_address, unsigned int port, int timeout);

/**
 * Search for a XPL device with given serial number in local network(s).
 *
 * This is a convinience function which calls xplclient_search_devices with default values.
 * The given serial number is trimmed for the comparison, i.e. leading/trailing whitespace and leading zeros are ignored.
 * Usually, only one device with a given serial number should exists at all, thus only the first device's first address
 * (in case the device has multiple ones) is returned due to the limited interface. However, this should be sufficient
 * for most use-cases.
 *
 * @param serial     The serial number of the desired target device.
 * @param addr       Pointer to a pointer which will receive the address of the target device (if found).
 *                   This will be malloc-ed, callee is responsible for to free it after use.
 * @param addrlen    Pointer to a socklen_t variable which will receive the length of the address (if target is found).
 * @return The count of matching devices (i.e. zero if no one was found at all), -1 with errno set on error.
 */
int xplclient_search_by_serial(const char *serial, struct sockaddr *addr, socklen_t *addrlen);

/**
 * This function assumes that the XPL device with the given serial number is a serial device. It searches
 * for this device by using xplclient_search_by_serial and queries the API whether remote access is possible
 * and which port to use. Finally it tries to open a socket to this port and return this connected socket.
 * Note: Since this function uses xplclient context helpers application is required to call xplclient_init
 *       prior to use this function.
 *
 * @param serial     The serial number of the desired target device.
 * @param comport    The physical port number of the target device to connect to (numbering starts at 1).
 * @return The socket filedescriptor on success, or -1 with errno set on error.
 */
int xplclient_socket_by_serial(const char *serial, unsigned int comport);

/**
 * Must be called from application prior the use of all other functions. Main purpose is
 * to call libcurl's initialize function.
 */
int xplclient_global_init(void);

/* XPL client library context - used in multiple functions to minimize parameters */
struct xplclient {
	/* first part of the URL string up to the port which is passed to cURL */
	char *url_prefix;

	/* curl context */
	CURL *curl;

	/* headers to with each request */
	struct curl_slist *headers;
};

typedef struct xplclient * xplclient_t;

/**
 * Create a new XPL client context by using an IP address.
 *
 * This function assumes that the XPL device is accessible on standard HTTP port 80,
 * i.e. the port field in sockaddr parameter is ignored.
 */
xplclient_t xplclient_new_by_addr(const struct sockaddr *addr, socklen_t addrlen);


/**
 * Create a new XPL client context with a given URL prefix.
 *
 * This function can be used when an XPL device should be accessed e.g. via port forwarding
 * and a dynamic dns address. In this case the URL should begin with "http://" or "https://",
 * include the hostname or IP address and port number (if necessary) and the entry point of
 * the XPL RESTful API (i.e. "/api").
 * Example: xplclient_new_by_url("http://ixasdasdbcvbqweh.myfritz.net:8080/api");
 */
xplclient_t xplclient_new_by_url(const char *url);

/**
 * Free all resources used by the given XPL client context.
 */
void xplclient_free(xplclient_t ctx);


/**
 * FIXME
 */
struct json_object *xplclient_url_get(xplclient_t ctx, const char *path);

/**
 * FIXME
 */
struct json_object *xplclient_url_set(xplclient_t ctx, const char *path, struct json_object *data);

/**
 * Traverse a JSON object hierarchy to access a given key of a JSON object. The path to the
 * desired key is given by a "pathname", that is a list of key names separated by /.
 * (That means that keys cannot include a '/' char by convention - no care is taken about this!)
 *
 * Example:
 *  { "device": {
 *      "product": "My fine product"
 *    }
 *  }
 *
 * Then you get with the pathname "device/product" the pointer for the JSON string object
 * with content "My fine product".
 *
 * @param root       Pointer to a root JSON object where to start.
 * @param key        The path to the desired JSON key object.
 * @return A pointer to the desired JSON object, or NULL on error or if not found.
 */
struct json_object *xplclient_json_object_get_by_key(struct json_object *root, const char *key);

#endif /* XPLCLIENT_H */
