/*
 * Copyright © 2016-2017 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>

#include <json.h>

#include "stringify.h"
#include "xplclient.h"
#include "config.h"

extern char *optarg;
extern int optind;

char *interface = NULL;
char *mc_address = XPLCLIENT_DEFAULT_MC_GROUP;
unsigned int port = XPLCLIENT_DEFAULT_MC_PORT;
int timeout = 3;
int csv_output = 0;

/* command line options */
const struct option long_options[] = {
	{ "interface",          required_argument,      0,      'i' },
	{ "timeout",            required_argument,      0,      't' },
	{ "mc-address",         required_argument,      0,      'a' },
	{ "port",               required_argument,      0,      'p' },
	{ "csv",                no_argument,            0,      'C' },
	{ "version",            no_argument,            0,      'V' },
	{ "help",               no_argument,            0,      'h' },

	{} /* stop condition for iterator */
};

/* descriptions for the command line options */
const char *long_options_descs[] = {
	"interface to use (default: use all available interfaces)",
	"response timeout (default: 3s)",
	"multicast address (default: " XPLCLIENT_DEFAULT_MC_GROUP ")",
	"port to use (default: " __stringify(XPLCLIENT_DEFAULT_MC_PORT) ")",
	"print found devices with CSV delimiters",
	"print version and exit",
	"print this usage and exit",
	NULL /* stop condition for iterator */
};

void usage(char *p, int exitcode)
{
	const char **desc = long_options_descs;
	const struct option *op = long_options;

	fprintf(stderr,
		"%s (%s) -- search and lists XPL devices in the local network\n\n"
		"Usage: %s [options]\n\n"
		"Options:\n",
		p, PACKAGE_STRING, p);

	while (op->name && desc) {
		fprintf(stderr, "\t-%c, --%-12s\t%s\n", op->val, op->name, *desc);
		op++; desc++;
	}

	fprintf(stderr, "\n");

	exit(exitcode);
}

/* parse options from the command line */
int options_parse_cli(int argc, char * argv[])
{
	int rc = EXIT_FAILURE;

	while (1) {
		int c = getopt_long(argc, argv, "i:t:a:p:CVh", long_options, NULL);

		/* detect the end of the options */
		if (c == -1) break;

		switch (c) {
		case 'i':
			interface = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			if (port == 0 || port > 65535) {
				fprintf(stderr, "Error: Port must be in range [0, 65535].");
				exit(EXIT_FAILURE);
			}
			break;
		case 'a':
			mc_address = optarg;
			break;
		case 't':
			timeout = atoi(optarg);
			if (timeout < 0 || timeout > 10) {
				fprintf(stderr, "Error: Timeout must be in range [0, 10] seconds.");
				exit(EXIT_FAILURE);
			}
			break;
		case 'C':
			csv_output = 1;
			break;
		case 'V':
			fprintf(stderr, "%s (%s)\n", argv[0], PACKAGE_STRING);
			exit(EXIT_SUCCESS);
		case '?':
		case 'h':
			rc = EXIT_SUCCESS;
			/* fall-through */
		default:
			usage(argv[0], rc);
		}
	}

	return 0;
}

#define PRETTY_FORMAT "%-16s %-10s %-17s %-10s %s\n"

int print_device(void *ctx, const struct sockaddr *address, socklen_t addrlen, struct json_object *deviceinfo)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	struct json_object *serial = NULL, *mac = NULL, *product = NULL, *sw_version = NULL;

	json_object_object_get_ex(deviceinfo, "serial", &serial);
	json_object_object_get_ex(deviceinfo, "mac_address", &mac);
	json_object_object_get_ex(deviceinfo, "product", &product);
	json_object_object_get_ex(deviceinfo, "software_version", &sw_version);

	printf(csv_output ? "%s;%s;%s;%s;%s\n" : PRETTY_FORMAT, inet_ntoa(addr->sin_addr),
	       serial ? json_object_get_string(serial) : "-",
	       mac ? json_object_get_string(mac) : "-",
	       sw_version ? json_object_get_string(sw_version) : "-",
	       product ? json_object_get_string(product) : "-");

	json_object_put(deviceinfo);

	return 0;
}

int main(int argc, char *argv[])
{
	options_parse_cli(argc, argv);

	if (csv_output) {
		fprintf(stderr, "IP Address;Serial;MAC Address;SW Version;Product\n");
	} else {
		fprintf(stderr, PRETTY_FORMAT, "IP Address", "Serial", "MAC Address", "SW Version", "Product");
		fprintf(stderr, PRETTY_FORMAT, "----------------", "----------", "-----------------", "----------", "-------------");
	}

	return xplclient_search_devices(print_device, NULL, interface, mc_address, port, timeout) ? EXIT_FAILURE : EXIT_SUCCESS;
}
