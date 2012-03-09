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
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "../internal.h"
#include "addrval.h"

xbee_err xbee_addrval_none(struct xbee_conAddress *address) {
	/* we don't need any address information */
	if (!address) return XBEE_ENONE;
	if (address->addr16_enabled) return XBEE_EINVAL;
	if (address->addr64_enabled) return XBEE_EINVAL;
	if (address->endpoints_enabled) return XBEE_EINVAL;
	return XBEE_ENONE;
}

xbee_err xbee_addrval_16or64(struct xbee_conAddress *address) {
	/* we need only ONE of 16-bit, or 64-bit address, NOT both */
	if (!address) return XBEE_EINVAL;
	if (address->endpoints_enabled) return XBEE_EINVAL;
	if (!address->addr16_enabled && !address->addr64_enabled) return XBEE_EINVAL; /* neither */
	if ( address->addr16_enabled &&  address->addr64_enabled) return XBEE_EINVAL; /* both */
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_addrval_16bit(struct xbee_conAddress *address) {
	/* we need only need 16-bit info */
	if (!address) return XBEE_EINVAL;
	if (!address->addr16_enabled) return XBEE_EINVAL;
	if (address->addr64_enabled) return XBEE_EINVAL;
	if (address->endpoints_enabled) return XBEE_EINVAL;
	return XBEE_ENONE;
}

xbee_err xbee_addrval_64bit(struct xbee_conAddress *address) {
	/* we need only need 64-bit info */
	if (!address) return XBEE_EINVAL;
	if (address->addr16_enabled) return XBEE_EINVAL;
	if (!address->addr64_enabled) return XBEE_EINVAL;
	if (address->endpoints_enabled) return XBEE_EINVAL;
	return XBEE_ENONE;
}
