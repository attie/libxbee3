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
#include "mode.h"

/* ######################################################################### */

xbee_err xbee_netServer_frontchannel_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_netServer_frontchannel_tx_func(struct xbee *xbee, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, unsigned char *buf, int len, struct xbee_buf **oBuf) {
	return XBEE_ENOTIMPLEMENTED;
}

/* ######################################################################### */

xbee_err xbee_netServer_backchannel_rx_func(struct xbee *xbee, void *arg, unsigned char identifier, struct xbee_buf *buf, struct xbee_frameInfo *frameInfo, struct xbee_conAddress *address, struct xbee_pkt **pkt) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_netServer_backchannel_tx_func(struct xbee *xbee, void *arg, unsigned char identifier, unsigned char frameId, struct xbee_conAddress *address, struct xbee_conSettings *settings, unsigned char *buf, int len, struct xbee_buf **oBuf) {
	return XBEE_ENOTIMPLEMENTED;
}

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_netServer_frontchannel_rx = {
	.identifier = 0x01,
	.func = xbee_netServer_frontchannel_rx_func,
};
const struct xbee_modeDataHandlerTx xbee_netServer_frontchannel_tx = {
	.identifier = 0x01,
	.func = xbee_netServer_frontchannel_tx_func,
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
	.func = xbee_netServer_backchannel_rx_func,
};
const struct xbee_modeDataHandlerTx xbee_netServer_backchannel_tx = {
	.identifier = 0x00,
	.func = xbee_netServer_backchannel_tx_func,
};
const struct xbee_modeConType xbee_netServer_backchannel = {
	.name = "Backchannel",
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
