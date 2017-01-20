/*
 * Copyright © 2016-2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */
#ifndef XPLCLIENT_H
#define XPLCLIENT_H

#include <sys/socket.h>
#include <json.h>

#include "xplclient-version.h"

#define XPLCLIENT_DEFAULT_MC_PORT 4109
#define XPLCLIENT_DEFAULT_MC_GROUP "239.255.255.250"

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
int xplclient_search_by_serial(const char *serial, struct sockaddr **addr, socklen_t *addrlen);

/**
 * Assumes that the XPL device with the given serial number is a serial device.
 * open socket and connects socket
 * FIXME
 */
int xplclient_socket_by_serial(const char *serial, int comport);

#endif /* XPLCLIENT_H */
