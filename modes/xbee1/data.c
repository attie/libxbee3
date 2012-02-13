/*
  libxbee - a C library to aid the use of Digi's XBee wireless modules
            running in API mode (AP=2).

  Copyright (C) 2009  Attie Grande (attie@attie.co.uk)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
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
#include "data.h"

xbee_err xbee_s1_data_rx_func(struct xbee *xbee, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;
	int addrLen;

	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	if (buf->len < 1) return XBEE_ELENGTH;
	
	switch (buf->data[0]) {
		case 0x80: addrLen = 8; break;
		case 0x81: addrLen = 2; break;
		default: return XBEE_EINVAL;
	}
	
	if (buf->len < addrLen + 3) return XBEE_ELENGTH;
	
	if (addrLen == 8) {
		address->addr64_enabled = 1;
		memcpy(address->addr64, &(buf->data[1]), addrLen);
	} else {
		address->addr16_enabled = 1;
		memcpy(address->addr16, &(buf->data[1]), addrLen);
	}
	
	if ((ret = xbee_pktAlloc(&iPkt, NULL, buf->len - (addrLen + 3))) != XBEE_ENONE) return ret;
	
	iPkt->rssi = buf->data[addrLen + 1];
	iPkt->settings = buf->data[addrLen + 2];
	
	iPkt->dataLen = buf->len - (addrLen + 3);
	if (iPkt->dataLen > 0) {
		memcpy(iPkt->data, &(buf->data[addrLen + 3]), iPkt->dataLen);
	}
	iPkt->data[iPkt->dataLen] = '\0';
	
	*pkt = iPkt;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_s1_16bitData_rx  = {
	.identifier = 0x81,
	.func = xbee_s1_data_rx_func,
};
const struct xbee_modeDataHandlerTx xbee_s1_16bitData_tx  = {
	.identifier = 0x01,
	.func = NULL,
};
const struct xbee_modeConType xbee_s1_16bitData = {
	.name = "Local AT",
	.rxHandler = &xbee_s1_16bitData_rx,
	.txHandler = &xbee_s1_16bitData_tx,
};

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_s1_64bitData_rx  = {
	.identifier = 0x80,
	.func = xbee_s1_data_rx_func,
};
const struct xbee_modeDataHandlerTx xbee_s1_64bitData_tx  = {
	.identifier = 0x00,
	.func = NULL,
};
const struct xbee_modeConType xbee_s1_64bitData = {
	.name = "64-bit Data",
	.rxHandler = &xbee_s1_64bitData_rx,
	.txHandler = &xbee_s1_64bitData_tx,
};
