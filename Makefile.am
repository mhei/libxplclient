#
# Copyright © 2016 Michael Heimpold <mhei@heimpold.de>
#
# SPDX-License-Identifier: LGPL-2.1+
#

SUBDIRS		= src tools
EXTRA_DIST	= autogen.sh autogen-clean.sh README.md

AM_CFLAGS	= -Wall -pendatic
ACLOCAL_AMFLAGS	= -I m4 ${ACLOCAL_FLAGS}

pkgconfigdir	= $(libdir)/pkgconfig
pkgconfig_DATA	= $(PACKAGE).pc
