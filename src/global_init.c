/*
 * Copyright © 2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <curl/curl.h>

#include "xplclient.h"

int xplclient_global_init(void)
{
	CURLcode rv;

	rv = curl_global_init(CURL_GLOBAL_ALL);
	return (rv == 0) ? 0 : -1;
}
