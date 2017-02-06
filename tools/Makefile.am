#
# Copyright © 2016-2017 Michael Heimpold <mhei@heimpold.de>
#
# SPDX-License-Identifier: LGPL-2.1+
#

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src
AM_CFLAGS = -Wall -g

common_ldflags = $(top_builddir)/src/libxplclient.la

bin_PROGRAMS = xpl-list xpl-conf-get xpl-conf-set

xpl_list_SOURCES = xpl-list.c
xpl_list_CFLAGS = $(JSONC_CFLAGS)
xpl_list_LDADD = $(common_ldflags) $(JSONC_LIBS)

xpl_conf_get_SOURCES = xpl-conf-get.c
xpl_conf_get_CFLAGS = $(JSONC_CFLAGS)
xpl_conf_get_LDADD = $(common_ldflags) $(JSONC_LIBS) $(CURL_LIBS)

xpl_conf_set_SOURCES = xpl-conf-set.c
xpl_conf_set_CFLAGS = $(JSONC_CFLAGS)
xpl_conf_set_LDADD = $(common_ldflags) $(JSONC_LIBS) $(CURL_LIBS)

CLEANFILES = *~
