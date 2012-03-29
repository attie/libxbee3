#ifndef __XBEE_INTERNAL_H
#define __XBEE_INTERNAL_H

/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with libxbee. If not, see <http://www.gnu.org/licenses/>.
*/

#include "xsys.h"
#include "xbee.h"
#include "mutex.h"

/* ######################################################################### */
/* just to keep the compiler happy when pointers are used, but not dereferenced... */

struct xbee;
struct xbee_con;
struct xbee_frameBlock;
struct xbee_frameInfo;
struct xbee_log;
struct xbee_mode;
struct xbee_txInfo;
struct xbee_rxInfo;
struct xbee_interface;

struct xbee_buf {
	size_t len;
	unsigned char data[1];
};

#define LH fprintf(stderr, "LOG_HERE: %s:%d\n", __FILE__, __LINE__);

/* ######################################################################### */

#endif /* __XBEE_INTERNAL_H */
