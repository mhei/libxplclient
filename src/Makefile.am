#
# Copyright © 2016 Michael Heimpold <mhei@heimpold.de>
#
# SPDX-License-Identifier: LGPL-2.1+
#

pkginclude_HEADERS = xplclient.h
lib_LTLIBRARIES = libxplclient.la

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBEXECDIR=\""$(libexecdir)"\" \
	-I${top_srcdir}/src

libxplclient_la_SOURCES = \
	search_devices.c \
	search_by_serial.c \
	stringify.h \
	xplclient.h \
	xplclient-version.h

libxplclient_la_CFLAGS = $(JSONC_CFLAGS)

libxplclient_la_LDFLAGS = $(JSONC_LIBS) -no-undefined \
	-version-info $(LIBXPLCLIENT_LT_VERSION_INFO)

# Header files to install
libxplclientincludedir = $(includedir)/libxplclient
libxplclientinclude_HEADERS = xplclient-version.h

CLEANFILES = *~
DISTCLEANFILES = xplclient-version.h
EXTRA_DIST = xplclient-version.h.in
