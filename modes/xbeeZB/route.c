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
#include <string.h>

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../mode.h"
#include "../../pkt.h"
#include "../../frame.h"
#include "../common.h"
#include "route.h"

xbee_err xbee_sZB_createSourceRoute_tx_func(struct xbee *xbee, struct xbee_con *con, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, const unsigned char *buf, int len, struct xbee_sbuf **oBuf) {
	struct xbee_sbuf *iBuf;

	if (!xbee || !address || !buf || !oBuf) return XBEE_EMISSINGPARAM;

	if (len < 12) return XBEE_ELENGTH;

	if ((iBuf = malloc(sizeof(*iBuf) + 1 + len)) == NULL) return XBEE_ENOMEM;

	iBuf->data[0] = identifier;
	memcpy(&(iBuf->data[1]), buf, len);
	iBuf->data[11] = 0; /* the datasheet specifies that this is zero */
	iBuf->data[1 + len] = '\0';

	*oBuf = iBuf;

	return XBEE_ENONE;
}

struct xbee_modeDataHandlerTx xbee_sZB_createSourceRoute_tx  = {
	.identifier = 0x21,
	.func = xbee_sZB_createSourceRoute_tx_func,
};
struct xbee_modeConType xbee_sZB_createSourceRoute = {
	.name = "Create Source Route",
	.allowFrameId = 0,
	.useTimeout = 0,
	/* no addressing is allowed... you must always handcraft the whole payload
	   refer to the datasheet, and start with the 64-bit destination address */
	.addressRules = ADDR_NONE,
	.rxHandler = NULL,
	.txHandler = &xbee_sZB_createSourceRoute_tx,
};

/* ######################################################################### */

xbee_err xbee_sZB_routeRecordIndicator_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_tbuf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;

	if (!xbee || !frameInfo || !buf || !address | !pkt) return XBEE_EMISSINGPARAM;

	if (buf->len < 13) return XBEE_ELENGTH;

	if ((ret = xbee_pktAlloc(&iPkt, NULL, buf->len - 12)) != XBEE_ENONE) return ret;

	address->addr64_enabled = 1;
	memcpy(address->addr64, &(buf->data[1]), 8);
	address->addr16_enabled = 1;
	memcpy(address->addr16, &(buf->data[9]), 2);

	iPkt->options = buf->data[11];

	iPkt->dataLen = buf->len - 12;

	if (iPkt->dataLen > 0) {
		memcpy(iPkt->data, &(buf->data[12]), iPkt->dataLen);
	}
	iPkt->data[iPkt->dataLen] = '\0';

	*pkt = iPkt;

	return XBEE_ENONE;
}


/* ######################################################################### */

struct xbee_modeDataHandlerRx xbee_sZB_routeRecordIndicator_rx = {
	.identifier = 0xA1,
	.func = xbee_sZB_routeRecordIndicator_rx_func,
};
struct xbee_modeConType xbee_sZB_routeRecordIndicator = {
	.name = "Route Record Indicator",
	.allowFrameId = 0,
	.useTimeout = 0,
	/* for the route record indicator, there is no requirement for any addressing...
	   but be careful, either use addressing (possibly with a catchall), or don't
	   a connection without an address will match all incoming packets */
	.addressRules = ADDR_EP_NOTALLOW,
	.save_addr16 = 1,
	.save_addr64 = 1,
	.rxHandler = &xbee_sZB_routeRecordIndicator_rx,
	.txHandler = NULL,
};
