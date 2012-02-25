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

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../mode.h"
#include "../../net_handlers.h"
#include "handlers.h"

xbee_err xbee_net_frontchannel_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_net_frontchannel_tx_func(struct xbee *xbee, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, unsigned char *buf, int len, struct xbee_buf **oBuf) {
	return XBEE_ENOTIMPLEMENTED;
}

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_net_frontchannel_rx = {
	.identifier = 0x00,
	.func = xbee_net_frontchannel_rx_func,
};

const struct xbee_modeDataHandlerTx xbee_net_frontchannel_tx = {
	.identifier = 0x00,
	.func = xbee_net_frontchannel_tx_func,
};

/* this one allows frame IDs */
const struct xbee_modeConType xbee_net_frontchannel_template = {
	.name = NULL,
	.internal = 0,
	.allowFrameId = 1,
	.useTimeout = 1,
	.timeout = {
		.tv_sec = 5,
		.tv_nsec = 0,
	},
	.rxHandler = NULL,
	.txHandler = NULL,
};

/* this one does not allow frame IDs */
const struct xbee_modeConType xbee_net_frontchannel_template_fid = {
	.name = NULL,
	.internal = 0,
	.allowFrameId = 0,
	.useTimeout = 1,
	.timeout = {
		.tv_sec = 5,
		.tv_nsec = 0,
	},
	.rxHandler = NULL,
	.txHandler = NULL,
};

/* ######################################################################### */

const struct xbee_modeConType xbee_net_backchannel = {
	.name = "Backchannel",
	.internal = 1,
	.allowFrameId = 1, /* this needs redeclaring, because this is enabled for the client */
	.useTimeout = 1,
	.timeout = {
		.tv_sec = 5,
		.tv_nsec = 0,
	},
	.rxHandler = &xbee_netServer_backchannel_rx,
	.txHandler = &xbee_netServer_backchannel_tx,
};