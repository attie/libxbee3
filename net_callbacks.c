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
#include "net_callbacks.h"

/* ######################################################################### */

void xbee_net_start(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

void xbee_net_echo(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

void xbee_net_validate(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

/* ######################################################################### */

void xbee_net_conNew(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

void xbee_net_conValidate(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

/* ######################################################################### */

void xbee_net_connTx(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

void xbee_net_conRx(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

/* ######################################################################### */

void xbee_net_conSleep(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

/* ######################################################################### */

void xbee_net_conInfoGet(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

/* ######################################################################### */

void xbee_net_conSettings(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

/* ######################################################################### */

void xbee_net_conEnd(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	
}

/* ######################################################################### */

#define ADD_NETSERVERCALLBACK(cb) { .name = #cb, .callback = xbee_net_##cb },
const struct xbee_netCallback xbee_netServerCallbacks[] = {
	/* backchannel (0x00), endpoint 0 (0x00) is ALWAYS the 'start' function */
	ADD_NETSERVERCALLBACK(start) /* this MUST BE FIRST */
	/* - */
	ADD_NETSERVERCALLBACK(connTx)
	ADD_NETSERVERCALLBACK(conRx)
	ADD_NETSERVERCALLBACK(conValidate)
	ADD_NETSERVERCALLBACK(conSleep)
	ADD_NETSERVERCALLBACK(conInfoGet)
	ADD_NETSERVERCALLBACK(conSettings)
	ADD_NETSERVERCALLBACK(conNew)
	ADD_NETSERVERCALLBACK(conEnd)
	/* - */
	ADD_NETSERVERCALLBACK(echo)
	ADD_NETSERVERCALLBACK(validate)
	/* - */
	{ NULL, NULL },
};
