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
#include "xbee_int.h"
#include "net.h"
#include "net_handlers.h"
#include "pkt.h"
#include "mode.h"
#include "frame.h"

/* ######################################################################### */

xbee_err xbee_netServer_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;
	int pos;
	
	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	if (!arg) {
		if (buf->len < 4) return XBEE_ELENGTH;
		/* only in the client, notify about frameID */
		frameInfo->active = 1;
		frameInfo->id = buf->data[2];
		frameInfo->retVal = buf->data[3];
		pos = 4;
	} else {
		if (buf->len < 3) return XBEE_ELENGTH;
		pos = 3;
	}
	
	address->endpoints_enabled = 1;
	address->endpoint_local = buf->data[1];
	address->endpoint_remote = buf->data[1];
	
	if ((ret = xbee_pktAlloc(&iPkt, NULL, buf->len - pos)) != XBEE_ENONE) return ret;
	
	iPkt->frameId = buf->data[2];
	
	iPkt->dataLen = buf->len - pos;
	if (iPkt->dataLen > 0) {
		memcpy(iPkt->data, &(buf->data[pos]), iPkt->dataLen);
	}
	iPkt->data[iPkt->dataLen] = '\0';
	
	*pkt = iPkt;
	
	return XBEE_ENONE;
}

xbee_err xbee_netServer_tx_func(struct xbee *xbee, struct xbee_con *con, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, unsigned char *buf, int len, struct xbee_buf **oBuf) {
	struct xbee_buf *iBuf;
	size_t bufLen;
	size_t memSize;
	int pos;
	
	if (!xbee || !address || !buf || !oBuf) return XBEE_EMISSINGPARAM;
	
	if (!address->endpoints_enabled) return XBEE_EINVAL;
	
	if (arg) {
		/* the server returns the frameId in the buffer */
		pos = 2;
	} else {
		/* the client sends the frameId from the fBlock */
		pos = 3;
	}
	memSize = pos + len;
	bufLen = memSize;
	
	memSize += sizeof(*iBuf);
	
	if ((iBuf = malloc(memSize)) == NULL) return XBEE_ENOMEM;
	
	iBuf->len = bufLen;
	iBuf->data[0] = identifier;
	iBuf->data[1] = address->endpoint_local;
	if (!arg) {
		iBuf->data[2] = frameId;
	}
	memcpy(&(iBuf->data[pos]), buf, len);

	*oBuf = iBuf;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_netServer_frontchannel_rx = {
	.identifier = 0x01,
	.func = xbee_netServer_rx_func,
};
const struct xbee_modeDataHandlerTx xbee_netServer_frontchannel_tx = {
	.identifier = 0x01,
	.func = xbee_netServer_tx_func,
};
const struct xbee_modeConType xbee_netServer_frontchannel = {
	.name = "Frontchannel",
	.allowFrameId = 0,
	.useTimeout = 0,
	.rxHandler = &xbee_netServer_frontchannel_rx,
	.txHandler = &xbee_netServer_frontchannel_tx,
};

/* ######################################################################### */

/* backchannel (0x00), endpoint 0 (0x00) is ALWAYS the 'start' function */
const struct xbee_modeDataHandlerRx xbee_netServer_backchannel_rx = {
	.identifier = 0x00,
	.func = xbee_netServer_rx_func,
};
const struct xbee_modeDataHandlerTx xbee_netServer_backchannel_tx = {
	.identifier = 0x00,
	.func = xbee_netServer_tx_func,
};
const struct xbee_modeConType xbee_netServer_backchannel = {
	.name = "Backchannel",
	.internal = 1,
	.allowFrameId = 0,
	.useTimeout = 0,
	.rxHandler = &xbee_netServer_backchannel_rx,
	.txHandler = &xbee_netServer_backchannel_tx,
};

/* ######################################################################### */

static const struct xbee_modeConType *conTypes[] = {
	&xbee_netServer_frontchannel,
	&xbee_netServer_backchannel,
	NULL
};

const struct xbee_mode xbee_netServerMode = {
	.name = "libxbee Server",
	
	.conTypes = conTypes,
};
