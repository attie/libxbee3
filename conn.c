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
#include "conn.h"
#include "ll.h"

xbee_err xbee_conAlloc(struct xbee *xbee, struct xbee_con **nCon) {
	size_t memSize;
	struct xbee_con *con;
	xbee_err ret;
	
	if (!xbee || !nCon) return XBEE_EMISSINGPARAM;
	if (xbee_validate(xbee)) return XBEE_EINVAL;
	
	memSize = sizeof(*con);
	
	if (!(con = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(con, 0, memSize);
	con->pktList = ll_alloc();
	con->xbee = xbee;
	
	if ((ret = ll_add_tail(conList, con)) != XBEE_ENONE) {
		free(con);
		ret = XBEE_ELINKEDLIST;
	}
	
	*nCon = con;
	return ret;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conValidate(struct xbee_con *con) {
	if (ll_get_item(conList, con) != XBEE_ENONE) return XBEE_EINVAL;
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conSleepSet(struct xbee_con *con, enum xbee_conSleepStates state) {
	if (!con) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	con->sleepState = state;
	return XBEE_ENONE;
}

xbee_err xbee_conSleepGet(struct xbee_con *con, enum xbee_conSleepStates *state) {
	if (!con || !state) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	*state = con->sleepState;
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conDataSet(struct xbee_con *con, void *newData, void **oldData) {
	if (!con) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	if (oldData) *oldData = con->userData;
	con->userData = newData;
	return XBEE_ENONE;
}

xbee_err xbee_conDataGet(struct xbee_con *con, void **curData) {
	if (!con || !curData) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	*curData = con->userData;
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conInfoGet(struct xbee_con *con, struct xbee_conInfo *info) {
	if (!con || !info) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	memcpy(info, &con->info, sizeof(con->info));
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conSettings(struct xbee_con *con, struct xbee_conSettings *newSettings, struct xbee_conSettings *oldSettings) {
	if (!con || (!newSettings && !oldSettings)) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	if (oldSettings) memcpy(oldSettings, &con->settings, sizeof(con->settings));
	if (newSettings) memcpy(&con->settings, newSettings, sizeof(con->settings));
	return XBEE_ENONE;
}
