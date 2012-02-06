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
#include "pkt.h"
#include "ll.h"

struct ll_head *conList;

/* ########################################################################## */

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

xbee_err xbee_conFree(struct xbee_con *con) {
	if (!con) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	
	ll_ext_item(conList, con);
	
	ll_free(con->pktList, (void(*)(void*))xbee_pktFree);
	
	free(con);
	
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conLink(struct xbee *xbee, struct xbee_con *con) {
	xbee_err ret;
	if (!xbee || !con) return XBEE_EMISSINGPARAM;
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
	if (ll_get_item(xbee->conList, con) == XBEE_ENONE) return XBEE_EEXISTS;
	if ((ret = ll_add_tail(xbee->conList, con)) == XBEE_ENONE) {
		con->xbee = xbee;
	}
	return ret;
}

xbee_err xbee_conUnlink(struct xbee *xbee, struct xbee_con *con) {
	xbee_err ret;
	if (!xbee || !con) return XBEE_EMISSINGPARAM;
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
	if ((ret = ll_ext_item(xbee->conList, con)) == XBEE_ENONE) {
		con->xbee = NULL;
	}
	return ret;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conGetTypes(struct xbee *xbee, char ***retList) {
#warning INFO - needs info from remote, this info should be retrieved at setup
	return XBEE_ENOTIMPLEMENTED;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conNew(struct xbee *xbee, struct xbee_con *ret_con, char *type, struct xbee_conAddress *address) {
#warning INFO - needs remote
	return XBEE_ENOTIMPLEMENTED;
}

EXPORT xbee_err xbee_conValidate(struct xbee_con *con) {
#warning INFO - needs remote (if remote fails/contradicts, is XBEE_ESTALE)
	if (ll_get_item(conList, con) != XBEE_ENONE) return XBEE_EINVAL;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conTx(struct xbee_con *con, char *format, ...) {
	xbee_err ret;
	va_list ap;
	
	va_start(ap, format);
	ret = xbee_convTx(con, format, ap);
	va_end(ap);
	
	return ret;
}

EXPORT xbee_err xbee_convTx(struct xbee_con *con, char *format, va_list args) {
	xbee_err ret;
	int bufLen, outLen;
	char *buf;
	
	if ((bufLen = vsnprintf(NULL, 0, format, args)) > 0) {
		bufLen += 1; /* make space for the terminating '\0' */
		if (!(buf = malloc(bufLen))) {
			return XBEE_ENOMEM;
		}
		outLen = vsnprintf(buf, bufLen, format, args) + 1;
		if (outLen > bufLen) {
			ret = XBEE_ERANGE;
			goto die;
		}
	} else {
		buf = NULL;
		outLen = 0;
	}
	
	ret = xbee_connTx(con, (unsigned char*)buf, outLen);
	
die:
	if (buf) free(buf);
	return ret;
}

EXPORT xbee_err xbee_connTx(struct xbee_con *con, unsigned char *buf, int len) {
#warning INFO - needs remote, can return XBEE_ESTALE
	return XBEE_ENOTIMPLEMENTED;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conRx(struct xbee_con *con, struct xbee_pkt **retPkt, int *remainingPackets) {
	xbee_err ret;
	unsigned int remain;
	struct xbee_pkt *pkt;
	if (!con || !retPkt) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	if (con->callback != NULL) return XBEE_EINVAL;
	
	ret = XBEE_ENONE;
	remain = 0;
	
	ll_lock(con->pktList);
	if ((ret = _ll_count_items(con->pktList, &remain, 0)) != XBEE_ENONE) goto die;
	if (remain == 0) {
		*retPkt = NULL;
		goto die;
	}
	_ll_ext_head(con->pktList, (void**)&pkt, 0);
	xbee_pktUnlink(con, pkt);
	*retPkt = pkt;
die:
	ll_unlock(con->pktList);

	if (remainingPackets) *remainingPackets = (remain > 0 ? remain - 1 : 0);
	
	return ret;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conSleepSet(struct xbee_con *con, enum xbee_conSleepStates state) {
#warning INFO - needs remote, can return XBEE_ESTALE
	if (!con) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	con->sleepState = state;
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_conSleepGet(struct xbee_con *con, enum xbee_conSleepStates *state) {
#warning INFO - needs remote (if remote fails/contradicts, update local or is XBEE_ESTALE)
	if (!con || !state) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	*state = con->sleepState;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conDataSet(struct xbee_con *con, void *newData, void **oldData) {
	if (!con) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	if (oldData) *oldData = con->userData;
	con->userData = newData;
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_conDataGet(struct xbee_con *con, void **curData) {
	if (!con || !curData) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	*curData = con->userData;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conInfoGet(struct xbee_con *con, struct xbee_conInfo *info) {
	if (!con || !info) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	memcpy(info, &con->info, sizeof(con->info));
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conCallbackSet(struct xbee_con *con, xbee_t_conCallback newCallback, xbee_t_conCallback *oldCallback) {
	return XBEE_ENOTIMPLEMENTED;
}

EXPORT xbee_err xbee_conCallbackGet(struct xbee_con *con, xbee_t_conCallback *curCallback) {
	if (!con || !curCallback) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	*curCallback = con->callback;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conSettings(struct xbee_con *con, struct xbee_conSettings *newSettings, struct xbee_conSettings *oldSettings) {
#warning INFO - needs remote, can return XBEE_ESTALE
	if (!con || (!newSettings && !oldSettings)) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con)) return XBEE_EINVAL;
	if (oldSettings) memcpy(oldSettings, &con->settings, sizeof(con->settings));
	if (newSettings) memcpy(&con->settings, newSettings, sizeof(con->settings));
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conEnd(struct xbee_con *con) {
#warning INFO - needs remote, can return XBEE_ESTALE
	return XBEE_ENOTIMPLEMENTED;
}
