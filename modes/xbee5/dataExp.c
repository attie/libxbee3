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
#include "../common.h"
#include "dataExp.h"

xbee_err xbee_s5_dataExp_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;

	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	if (buf->len < 18) return XBEE_ELENGTH;
	
	/* ClusterID (2 bytes) */
	/* ProfileID (2 bytes) */
	
	if ((ret = xbee_pktAlloc(&iPkt, NULL, buf->len - 18)) != XBEE_ENONE) return ret;
	
	address->addr64_enabled = 1;
	memcpy(address->addr64, &(buf->data[1]), 8);
	address->endpoints_enabled = 1;
	address->endpoint_remote = buf->data[11];
	address->endpoint_local = buf->data[12];
#ifndef _WIN32
#warning TODO - support cluster ID & profile ID
#endif
	
	iPkt->options = buf->data[17];
	
	iPkt->dataLen = buf->len - 18;
	if (iPkt->dataLen > 0) {
		memcpy(iPkt->data, &(buf->data[18]), iPkt->dataLen);
	}
	iPkt->data[iPkt->dataLen] = '\0';
	
	*pkt = iPkt;
	
	return XBEE_ENONE;
}

xbee_err xbee_s5_dataExp_tx_func(struct xbee *xbee, struct xbee_con *con, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, const unsigned char *buf, int len, struct xbee_buf **oBuf) {
	struct xbee_buf *iBuf;
	size_t bufLen;
	int pos;
	size_t memSize;
	
	if (!xbee || !address || !buf || !oBuf) return XBEE_EMISSINGPARAM;
	if (len > 72) return XBEE_ELENGTH;
	
	if (!address->addr64_enabled) return XBEE_EINVAL;
	
	/* API Identifier + Frame ID + Address (64) + Address (16) + Radius + Options + Payload */
	memSize = 20 + len;
	bufLen = memSize;
	
	memSize += sizeof(*iBuf);
	
	if ((iBuf = malloc(memSize)) == NULL) return XBEE_ENOMEM;
	
	pos = 0;
	iBuf->len = bufLen;
	iBuf->data[pos] = identifier;                         pos++;
	iBuf->data[pos] = frameId;                            pos++;
	if (settings->broadcast) {
		/* 64-bit broadcast address */
		iBuf->data[pos] = 0x00;                             pos++;
		iBuf->data[pos] = 0x00;                             pos++;
		iBuf->data[pos] = 0x00;                             pos++;
		iBuf->data[pos] = 0x00;                             pos++;
		iBuf->data[pos] = 0x00;                             pos++;
		iBuf->data[pos] = 0x00;                             pos++;
		iBuf->data[pos] = 0xFF;                             pos++;
		iBuf->data[pos] = 0xFF;                             pos++;
	} else {
		memcpy(&(iBuf->data[pos]), address->addr64, 8);     pos += 8;
	}
	iBuf->data[pos] = 0xFF;                             pos++;
	iBuf->data[pos] = 0xFE;                             pos++;
	if (address->endpoints_enabled) {
		iBuf->data[pos] = address->endpoint_local;          pos++;
		iBuf->data[pos] = address->endpoint_remote;         pos++;
	} else {
		iBuf->data[pos] = 0xE8; /* default to data... */    pos++;
		iBuf->data[pos] = 0xE8; /* ... endpoint */          pos++;
	}
#ifndef _WIN32
#warning TODO - support cluster ID & profile ID
#endif
	iBuf->data[pos] = 0x00; /* custerIDs are not... */    pos++;
	iBuf->data[pos] = 0x11; /* ..supported by libxbee */  pos++;
	iBuf->data[pos] = 0xC1; /* profileIDs are not... */   pos++;
	iBuf->data[pos] = 0x05; /* ..supported by libxbee */  pos++;
	iBuf->data[pos] = settings->broadcastRadius;          pos++;
	iBuf->data[pos] = 0;
	if (settings->disableAck) iBuf->data[pos] |= 0x01;
	if (settings->noRoute)    iBuf->data[pos] |= 0x02;
	                                                      pos++;

	memcpy(&(iBuf->data[pos]), buf, len);                 pos += len;
	iBuf->data[pos] = '\0';
	
	*oBuf = iBuf;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

void xbee_s5_dataExp_init(struct xbee_modeConType *conType) {
	/* we REALLY have to babysit Windows... */
	conType->allowFrameId = 1;
	conType->useTimeout = 0;
	conType->addressRules = ADDR_64_16OPT_EP;
	conType->save_addr16 = 1;
	conType->save_addr64 = 1;
	conType->rxHandler->identifier = 0x91;
	conType->rxHandler->func = xbee_s5_dataExp_rx_func;
	conType->txHandler->identifier = 0x11;
	conType->txHandler->func = xbee_s5_dataExp_tx_func;
}
struct xbee_modeDataHandlerRx xbee_s5_dataExp_rx;
struct xbee_modeDataHandlerTx xbee_s5_dataExp_tx;
struct xbee_modeConType xbee_s5_dataExp = { "Data (explicit)", &xbee_s5_dataExp_rx, &xbee_s5_dataExp_tx, xbee_s5_dataExp_init };
