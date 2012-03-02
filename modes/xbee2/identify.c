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
#include "../addrval.h"
#include "identify.h"

xbee_err xbee_s2_identify_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	struct xbee_pkt *iPkt;
	xbee_err ret;
	struct xbee_conAddress *addr;

	if (!xbee || !frameInfo || !buf || !address || !pkt) return XBEE_EMISSINGPARAM;
	
	if (buf->len < 30) return XBEE_ELENGTH;
	
	if ((ret = xbee_pktAlloc(&iPkt, NULL, buf->len - 12)) != XBEE_ENONE) return ret;
	
	iPkt->options = buf->data[11];
	
	iPkt->dataLen = buf->len - 12;
	if (iPkt->dataLen > 0) {
		memcpy(iPkt->data, &(buf->data[12]), iPkt->dataLen);
		
		if (iPkt->dataLen > 2) {
			xbee_pktDataAdd(iPkt, "Address (16-bit)", 0, &(iPkt->data[0]), NULL);
		}
		
		if (iPkt->dataLen > 10) {
			xbee_pktDataAdd(iPkt, "Address (64-bit)", 0, &(iPkt->data[2]), NULL);
			
			if ((addr = malloc(sizeof(*addr))) != NULL) {
				memset(addr, 0, sizeof(*addr));
				addr->addr16_enabled = 1;
				memcpy(addr->addr16, &(iPkt->data[0]), 2);
				addr->addr64_enabled = 1;
				memcpy(addr->addr64, &(iPkt->data[2]), 8);
				
				if (xbee_pktDataAdd(iPkt, "Address", 0, addr, free) != XBEE_ENONE) {
					free(addr);
				}
			}
		}
		
		if (iPkt->dataLen > 11) {
			/* just point into the packet data */
			xbee_pktDataAdd(iPkt, "NI", 0, &(iPkt->data[10]), NULL);
		}
	}
	iPkt->data[iPkt->dataLen] = '\0';
	
	*pkt = iPkt;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_s2_identify_rx  = {
	.identifier = 0x95,
	.func = xbee_s2_identify_rx_func,
};
const struct xbee_modeConType xbee_s2_identify = {
	.name = "Identify",
	.allowFrameId = 0,
	.useTimeout = 0,
	.address_validator = xbee_addrval_none,
	.rxHandler = &xbee_s2_identify_rx,
	.txHandler = NULL,
};
