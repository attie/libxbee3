/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode (AP=2).

	Copyright (C) 2009	Attie Grande (attie@attie.co.uk)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.	If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "internal.h"

EXPORT char *xbee_errorToStr(xbee_err error) {
	char *str;
	
	switch (error) {
		case XBEE_ENONE:
			str = "No error";
			break;
		case XBEE_EUNKNOWN:
		default:
			str = "Unknown error...";
			break;
		case XBEE_ENOMEM:
			str = "Out of memory";
			break;
		case XBEE_ESELECT:
			str = "select() failed";
			break;
		case XBEE_ESELECTINTERRUPTED:
			str = "select() was interrupted";
			break;
		case XBEE_EEOF:
			str = "An EOF character was read";
			break;
		case XBEE_EIORETRIES:
			str = "I/O retries have been depleated";
			break;
		case XBEE_EOPENFAILED:
			str = "open() failed";
			break;
		case XBEE_EIO:
			str = "An I/O request failed";
			break;
		case XBEE_ESEMAPHORE:
			str = "A semaphore error occured";
			break;
		case XBEE_ELINKEDLIST:
			str = "A linkedlist error occured";
			break;
		case XBEE_ETHREAD:
			str = "A pthread error occured";
			break;
		case XBEE_ENOXBEE:
			str = "No libxbee instance was provided (or avaliable)";
			break;
		case XBEE_EMISSINGPARAM:
			str = "A crucial parameter was missing";
			break;
		case XBEE_EINVALBAUDRATE:
			str = "An invalid baud rate was selected";
			break;
		case XBEE_ESETUP:
			str = "Setup failed";
			break;
		case XBEE_ELENGTH:
			str = "A length mis-match occured";
			break;
		case XBEE_EINVAL:
			str = "An invalid argument was provided";
			break;
		case XBEE_EBUSY:
			str = "The system is busy";
			break;
		case XBEE_ENOMODE:
			str = "No mode is currently enabled";
			break;
		case XBEE_EFAILED:
			str = "A function call failed";
			break;
		case XBEE_ECANTTX:
			str = "libxbee is unable to transmit data";
			break;
		case XBEE_ENOTREADY:
			str = "libxbee is not ready yet";
			break;
		case XBEE_ECALLBACK:
			str = "A callback error occured";
			break;
		case XBEE_EUNESCAPED_START:
			str = "An unescaped start sequence was detected";
			break;
		case XBEE_ETIMEOUT:
			str = "A timeout occured";
			break;
		case XBEE_EMUTEX:
			str = "A mutex error occured";
			break;
		case XBEE_EINUSE:
			str = "A function call returned early to prevent blocking";
			break;
		case XBEE_ERANGE:
			str = "Requested data falls outside the boundaries";
			break;
		case XBEE_EEXISTS:
			str = "An item already exists by that name or identifier";
			break;
		case XBEE_ESOCKET:
			str = "A socket error occured";
			break;
		case XBEE_ENOTIMPLEMENTED:
			str = "The called functionality has not yet been implemented";
			break;
		case XBEE_ESTALE:
			str = "Stale information was used during this function call";
			break;
	}
	
	return str;
}
