dnl
dnl Copyright © 2016 Michael Heimpold <mhei@heimpold.de>
dnl
dnl SPDX-License-Identifier: LGPL-2.1+
dnl

dnl
dnl Process this file with autoconf to produce a configure script.
dnl

dnl libxplclient package version number (distinct from shared library version)
m4_define([libxplclient_version_major], [0])
m4_define([libxplclient_version_minor], [0])
m4_define([libxplclient_version_micro], [1])

m4_define([libxplclient_version],
    [libxplclient_version_major.libxplclient_version_minor.libxplclient_version_micro])

AC_PREREQ([2.63])
AC_INIT([libxplclient], [libxplclient_version], [https://github.com/mhei/libxplclient/issues])

AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-xz])
AC_GNU_SOURCE

# enable nice build output on automake1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

LIBXPLCLIENT_VERSION_MAJOR=libxplclient_version_major
LIBXPLCLIENT_VERSION_MINOR=libxplclient_version_minor
LIBXPLCLIENT_VERSION_MICRO=libxplclient_version_micro
LIBXPLCLIENT_VERSION=libxplclient_version
AC_SUBST(LIBXPLCLIENT_VERSION_MAJOR)
AC_SUBST(LIBXPLCLIENT_VERSION_MINOR)
AC_SUBST(LIBXPLCLIENT_VERSION_MICRO)
AC_SUBST(LIBXPLCLIENT_VERSION)

# ABI version
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
LIBXPLCLIENT_LD_CURRENT=1
LIBXPLCLIENT_LD_REVISION=0
LIBXPLCLIENT_LD_AGE=0
LIBXPLCLIENT_LT_VERSION_INFO=$LIBXPLCLIENT_LD_CURRENT:$LIBXPLCLIENT_LD_REVISION:$LIBXPLCLIENT_LD_AGE
AC_SUBST(LIBXPLCLIENT_LT_VERSION_INFO)

PACKAGE_DESCRIPTION="Library for finding/controlling I2SE XPL devices"
AC_SUBST(PACKAGE_DESCRIPTION)

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_MAKE_SET
LT_INIT([disable-static])

# Checks for header files
AC_HEADER_STDC

PKG_CHECK_MODULES([JSONC], [json-c],, [AC_MSG_WARN("json-c not found")])
if test "JSONC_LIBS" = ""; then
    PKG_CHECK_MODULES([JSONC], [json],, [AC_MSG_ERROR("no JSON library available")])
fi

PKG_CHECK_MODULES([CURL], [libcurl])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/xplclient-version.h
	tools/Makefile
	libxplclient.pc
])
AC_OUTPUT
