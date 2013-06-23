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
#include "../../conn.h"
#include "../common.h"
#include "dataIP.h"

xbee_err xbee_s6_dataIP_onCreate(struct xbee *xbee, struct xbee_con *con) {
	if (!xbee || !con) return XBEE_EMISSINGPARAM;

	/* ensure we override the default profile and cluster IDs that would be used in xbee_conAddressCmp() */
	if (!con->address.profile_enabled) {
		con->address.profile_enabled = 1;
		con->address.profile_id = 0x2616;
	}
	if (!con->address.cluster_enabled) {
		con->address.cluster_enabled = 1;
		con->address.cluster_id = 0x2616;
	}

	return XBEE_ENONE;
}

xbee_err xbee_s6_dataIP_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_tbuf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;

	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	if (buf->len < 11) return XBEE_ELENGTH;
	
	if ((ret = xbee_pktAlloc(&iPkt, NULL, buf->len - 11)) != XBEE_ENONE) return ret;
	
	address->addr64_enabled = 1;
	address->addr64[0] = 0;
	address->addr64[1] = 0;
	address->addr64[2] = 0;
	address->addr64[3] = 0;
	address->addr64[4] = buf->data[1];
	address->addr64[5] = buf->data[2];
	address->addr64[6] = buf->data[3];
	address->addr64[7] = buf->data[4];
	/* the source / destination crossover is done here so that UART XBee moduless can be used too */
	/* profile is used for source port */
	address->profile_enabled = 1;
	address->profile_id = ((buf->data[5] << 8) & 0xFF00) | (buf->data[6] & 0xFF);
	/* cluster is used for destination port */
	address->cluster_enabled = 1;
	address->cluster_id = ((buf->data[7] << 8) & 0xFF00) | (buf->data[8] & 0xFF);
	
	iPkt->dataLen = buf->len - 11;
	if (iPkt->dataLen > 0) {
		memcpy(iPkt->data, &(buf->data[11]), iPkt->dataLen);
	}
	iPkt->data[iPkt->dataLen] = '\0';
	
	*pkt = iPkt;
	
	return XBEE_ENONE;
}

xbee_err xbee_s6_dataIP_tx_func(struct xbee *xbee, struct xbee_con *con, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, const unsigned char *buf, int len, struct xbee_sbuf **oBuf) {
	struct xbee_sbuf *iBuf;
	size_t bufLen;
	int pos;
	size_t memSize;
	
	if (!xbee || !address || !buf || !oBuf) return XBEE_EMISSINGPARAM;
	if (len > 1400) return XBEE_ELENGTH;
	
	if (!address->addr64_enabled) return XBEE_EINVAL;
	
	/* API Identifier + Frame ID + IP Address + Dest Port + Src Port + Protocol + Options + Payload */
	memSize = 12 + len;
	bufLen = memSize;
	
	memSize += sizeof(*iBuf);
	
	if ((iBuf = malloc(memSize)) == NULL) return XBEE_ENOMEM;
	
	pos = 0;
	iBuf->len = bufLen;
	iBuf->data[pos] = identifier;                         pos++;
	iBuf->data[pos] = frameId;                            pos++;
	if (settings->broadcast) {
		/* broadcast IP address this is the only time that libxbee uses UDP at the moment */
		iBuf->data[pos] = 0xFF;                             pos++;
		iBuf->data[pos] = 0xFF;                             pos++;
		iBuf->data[pos] = 0xFF;                             pos++;
		iBuf->data[pos] = 0xFF;                             pos++;
	} else {
		iBuf->data[pos] = address->addr64[4];               pos++;
		iBuf->data[pos] = address->addr64[5];               pos++;
		iBuf->data[pos] = address->addr64[6];               pos++;
		iBuf->data[pos] = address->addr64[7];               pos++;
	}
	/* the CLUSTER id is used for the DESTINATION TCP/UDP port number */
	if (address->cluster_enabled) {
		iBuf->data[pos] = (address->cluster_id >> 8) & 0xFF;  pos++;
		iBuf->data[pos] =  address->cluster_id       & 0xFF;  pos++;
	} else {
		iBuf->data[pos] = 0x26; /* dest port 0x2616... */   pos++;
		iBuf->data[pos] = 0x16; /* ... (default) */         pos++;
	}
	/* the PROFILE id is used for the SOURCE TCP/UDP port number */
	if (address->profile_enabled) {
		iBuf->data[pos] = (address->profile_id >> 8) & 0xFF;  pos++;
		iBuf->data[pos] =  address->profile_id       & 0xFF;  pos++;
	} else {
		iBuf->data[pos] = 0x26; /* src port 0x2616... */    pos++;
		iBuf->data[pos] = 0x16; /* ... (default) */         pos++;
	}
	iBuf->data[pos] = 0;
	if (!settings->broadcast)       iBuf->data[pos] |= 0x01;
	                                                      pos++;
	iBuf->data[pos] = 0;
	if (settings->noKeepOpen)       iBuf->data[pos] |= 0x01;
	                                                      pos++;

	memcpy(&(iBuf->data[pos]), buf, len);                 pos += len;
	iBuf->data[pos] = '\0';
	
	*oBuf = iBuf;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

struct xbee_modeDataHandlerRx xbee_s6_dataIP_rx  = {
	.identifier = 0xB0,
	.func = xbee_s6_dataIP_rx_func,
};
struct xbee_modeDataHandlerTx xbee_s6_dataIP_tx  = {
	.identifier = 0x20,
	.func = xbee_s6_dataIP_tx_func,
};
struct xbee_modeConType xbee_s6_dataIP = {
	.name = "Data (IP)",
	.allowFrameId = 1,
	.useTimeout = 0,
	.addressRules = ADDR_64_REQUIRED | ADDR_16_NOTALLOW,
	.rxHandler = &xbee_s6_dataIP_rx,
	.txHandler = &xbee_s6_dataIP_tx,

	.onCreate = xbee_s6_dataIP_onCreate,
};
