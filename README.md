[![Build Status](https://travis-ci.org/mhei/libxplclient.svg?branch=master)](https://travis-ci.org/mhei/libxplclient)

Overview
--------

libxplclient is a free software library to find I2SE's XPL devices within your
LAN, to switch XPL device's digital outputs and read/write analog input/outputs.

The description of the XPL device series RESTful API is documented online[1].

The license of libxplclient is LGPL v2.1 and the license of programs in
tools directory is GPL v3.

The documentation is available under the Creative Commons Attribution-ShareAlike
License 3.0 (Unported) (http://creativecommons.org/licenses/by-sa/3.0/).

The library is written in C and designed to run on Linux.


Installation
------------

The shell commands are ``./autogen.sh; ./configure; make; make install``.


Report a Bug
------------

To report a bug, you can:
 * fill a bug report on the issue tracker
   http://github.com/mhei/libxplclient/issues
 * or send an email to mhei@heimpold.de


Legal Notes
-----------

Trade names, trademarks and registered trademarks are used without special
marking throughout the source code and/or documentation. All are the properties
of their respective owners.


References
----------

[1] Manual of I2SE's XPL RESTful API
http://xpl-rest-api.readthedocs.io/en/latest/
