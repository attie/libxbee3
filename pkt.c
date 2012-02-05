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
#include <string.h>

#include "internal.h"
#include "pkt.h"
#include "ll.h"

struct ll_head *pktList;

xbee_err xbee_pktAlloc(struct xbee *xbee, struct xbee_pkt *oPkt, int dataLen) {
	size_t memSize;
	struct xbee_pkt *pkt;
	xbee_err ret;
	
	if (!xbee || xbee_validate(xbee)) return XBEE_EINVAL;
	
	if (oPkt) {
		if ((ret = ll_ext_item(xbee->pktList, oPkt)) != XBEE_ENONE) {
			return ret;
		}
	}
	
	memSize = sizeof(*pkt);
	memSize += sizeof(char) * dataLen;
	
	if (!(pkt = realloc(oPkt, memSize))) return XBEE_ENOMEM;
	
	if (!oPkt) {
		memset(pkt, 0, memSize);
		pkt->dataItems = ll_alloc();
	}
	pkt->xbee = xbee;
	
	if ((ret = ll_add_tail(xbee->pktList, pkt)) != XBEE_ENONE) {
		free(pkt);
		ret = XBEE_ELINKEDLIST;
	}
	
	return ret;
}

/* ########################################################################## */

EXPORT xbee_err xbee_pktValidate(struct xbee_pkt *pkt) {
	if (ll_get_item(pktList, pkt) == NULL) return XBEE_EINVAL;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_pktGetDigital(struct xbee_pkt *pkt, int channel, int index, int *retVal) {
	return XBEE_ENOTIMPLEMENTED;
}

EXPORT xbee_err xbee_pktGetAnalog(struct xbee_pkt *pkt, int channel, int index, int *retVal) {
	return XBEE_ENOTIMPLEMENTED;
}

/* ########################################################################## */

EXPORT xbee_err xbee_pktFree(struct xbee_pkt *pkt) {
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
	
	if (pkt->xbee) ll_ext_item(pkt->xbee->pktList, pkt);
	if (pkt->con)  ll_ext_item(pkt->con->pktList, pkt);
	               ll_ext_item(pktList, pkt);
	
	return XBEE_ENONE;
}