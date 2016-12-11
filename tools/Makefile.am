#
# Copyright © 2016 Michael Heimpold <mhei@heimpold.de>
#
# SPDX-License-Identifier: LGPL-2.1+
#

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src
AM_CFLAGS = -Wall -g

common_ldflags = $(top_builddir)/src/libxplclient.la

bin_PROGRAMS = xpl-list

xpl_list_SOURCES = xpl-list.c
xpl_list_CFLAGS = $(JSONC_CFLAGS)
xpl_list_LDADD = $(common_ldflags) $(JSONC_LIBS)

CLEANFILES = *~
