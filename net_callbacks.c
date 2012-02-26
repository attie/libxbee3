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

#ifndef XBEE_NO_NET_SERVER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "xbee_int.h"
#include "net.h"
#include "net_callbacks.h"
#include "ll.h"
#include "mode.h"
#include "conn.h"
#include "log.h"

/* ######################################################################### */

void xbee_net_fromClient(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	xbee_connTx((struct xbee_con *)(*data), NULL, (*pkt)->data, (*pkt)->dataLen);
}

void xbee_net_toClient(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	unsigned char *buf;
	size_t memSize;
	
	/* this will need updating if struct xbee_pkt changes */
	/* 6 = status + settings + rssi + frameId + atCommand[2] */
	/* dataLen can be inferred */
	memSize = 6 + (*pkt)->dataLen + 1;
	
	if ((buf = malloc(memSize)) == NULL) {
		xbee_log(1, "MALLOC FAILED... dataloss has occured");
		return;
	}
	
	buf[0] = (*pkt)->status;
	buf[1] = (*pkt)->settings;
	buf[2] = (*pkt)->rssi;
	buf[3] = (*pkt)->frameId;
	buf[4] = (*pkt)->atCommand[0];
	buf[5] = (*pkt)->atCommand[1];
	if ((*pkt)->dataLen > 0) {
		memcpy(&buf[6], (*pkt)->data, (*pkt)->dataLen);
	}
	
	xbee_connTx((struct xbee_con *)(*data), NULL, buf, memSize);
	
	free(buf);
}

/* ######################################################################### */

void xbee_net_start(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	int i, o;
	int callbackCount;
	struct xbee_buf *iBuf;
	size_t bufLen;
	size_t memSize;
	
	client = *data;

	if (strncasecmp((char *)(*pkt)->data, libxbee_commit, (*pkt)->dataLen)) {
#ifndef XBEE_NO_NET_STRICT_VERSIONS
		unsigned char buf[2];
		buf[0] = (*pkt)->frameId;
		buf[1] = 0x02;
		xbee_connTx(con, NULL, buf, sizeof(buf));
		client->die = 1;
		return;
#else
		xbee_log(-1, "*** client with mismatched version connected... this may cause instability ***");
#endif
	}

	memSize = 0;
	for (i = 1; xbee_netServerCallbacks[i].callback; i++) {
		memSize += strlen(xbee_netServerCallbacks[i].name) + 1;
	}
	callbackCount = i;
	
	memSize += 1; /* for an 8 bit 'count' */
	memSize += 2; /* for the frameId, and return value */
	bufLen = memSize;
	
	memSize += sizeof(*iBuf);
	
	if ((iBuf = malloc(memSize)) == NULL) {
		/* out of memory */
		unsigned char buf[2];
		buf[0] = (*pkt)->frameId;
		buf[1] = 0x01; /* <-- this means intenal error */
		xbee_connTx(con, NULL, buf, sizeof(buf));
		return;
	}
	
	iBuf->len = bufLen;
	iBuf->data[0] = (*pkt)->frameId;
	iBuf->data[1] = 0x00; /* <-- success */
	iBuf->data[2] = callbackCount - 1; /* -1 cos we started at 1, not 0 */
	for (i = 1, o = 3; i < callbackCount; i++) {
		o += snprintf((char *)&(iBuf->data[o]), iBuf->len - o, "%s", xbee_netServerCallbacks[i].name) + 1;
	}
	
	xbee_connTx(con, NULL, iBuf->data, iBuf->len);
	
	free(iBuf);

	client->started = 1;
}

void xbee_net_echo(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	xbee_connTx(con, NULL, (*pkt)->data, (*pkt)->dataLen);
}

/* ######################################################################### */

void xbee_net_conNew(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	xbee_err ret;
	unsigned char retVal;
	struct xbee_netClientInfo *client;
	struct xbee_conAddress address;
	struct xbee_con *nCon, *lCon, *tCon;
	int conIdentifier;
	struct xbee_modeConType *conType;
	char *conTypeName;
	int i, o;
	unsigned char buf[4];
	client = *data;
	if (!client->started) return;
	
	retVal = 0x01; /* <-- internal error */
	
	if ((*pkt)->dataLen != 1 + sizeof(address)) {
		retVal = 0x02; /* <-- request error */
		goto err;
	}
	
	conType = NULL;
	for (i = 0, o = 0; xbee->iface.conTypes[i].name; i++) {
		if (xbee->iface.conTypes[i].internal) continue;
		o++;
		if (o != (*pkt)->data[0]) continue;
		conType = &xbee->iface.conTypes[i];
		conTypeName = (char *)xbee->iface.conTypes[i].name;
		break;
	}
	if (!conType) {
		retVal = 0x02;
		goto err;
	}
	
	/* find a conIdentifier */
	conIdentifier = 0;
	for (tCon = NULL; ll_get_next(conType->conList, tCon, (void **)&tCon) == XBEE_ENONE && tCon; ) {
		if (tCon->conIdentifier == conIdentifier) {
			conIdentifier++;
			tCon = NULL;
			continue;
		}
	}
	if (conIdentifier > 0xFFFF) {
		retVal = 0x03;
		goto err;
	}
	
	
	/* create the local-side connection */
	memcpy(&address, &((*pkt)->data[1]), sizeof(address));
	if ((ret = xbee_conNew(xbee, &lCon, conTypeName, &address)) != XBEE_ENONE) goto err;
	lCon->conIdentifier = conIdentifier;
	lCon->netClient = client;
	ll_add_tail(client->conList, lCon);
	
	/* create the network-side connection */
	memset(&address, 0, sizeof(address));
	address.addr16_enabled = 1;
	address.addr16[0] = (lCon->conIdentifier >> 8) & 0xFF;
	address.addr16[1] = lCon->conIdentifier & 0xFF;
	
	if ((ret = _xbee_conNew(xbee, &client->iface, 0, &nCon, conTypeName, &address)) != XBEE_ENONE) goto err;
	nCon->netClient = client;
	
	xbee_conDataSet(lCon, nCon, NULL);
	xbee_conCallbackSet(lCon, xbee_net_toClient, NULL);
	
	xbee_conDataSet(nCon, lCon, NULL);
	xbee_conCallbackSet(nCon, xbee_net_fromClient, NULL);
	
	buf[0] = (*pkt)->frameId;
	buf[1] = 0x00;
	buf[2] = (lCon->conIdentifier >> 8) & 0xFF;
	buf[3] = lCon->conIdentifier & 0xFF;
	
	xbee_connTx(con, NULL, buf, sizeof(buf));
	
	return;
err:
	if (nCon) {
		//ll_ext_item(client->conList, lCon);
		xbee_conEnd(nCon);
	}
	if (lCon) {
		ll_ext_item(client->conList, lCon);
		xbee_conEnd(lCon);
	}
	{
		unsigned char buf[2];
		buf[0] = (*pkt)->frameId;
		buf[1] = retVal;
		xbee_connTx(con, NULL, buf, sizeof(buf));
	}
}

void xbee_net_conValidate(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	unsigned char retVal;
	int conIdentifier;
	struct xbee_con *iCon;
	client = *data;
	if (!client->started) return;
	
	retVal = 0x02;
	
	if ((*pkt)->dataLen != 2) {
		goto err;
	}
	
	conIdentifier = 0;
	conIdentifier |= (((*pkt)->data[0]) << 8) & 0xFF;
	conIdentifier |= ((*pkt)->data[1]) & 0xFF;
	
	for (iCon = NULL; ll_get_next(client->conList, iCon, (void**)&iCon) == XBEE_ENONE && iCon; ) {
		if (iCon->conIdentifier == conIdentifier) {
			retVal = 0x00;
			break;
		}
	}
	
err:
	{
		unsigned char buf[2];
		buf[0] = (*pkt)->frameId;
		buf[1] = retVal;
		xbee_connTx(con, NULL, buf, sizeof(buf));
	}
}

/* ######################################################################### */

void xbee_net_connTx(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	client = *data;
	if (!client->started) return;
	
}

void xbee_net_conRx(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	client = *data;
	if (!client->started) return;
	
}

/* ######################################################################### */

void xbee_net_conSleep(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	unsigned char retVal;
	int conIdentifier;
	enum xbee_conSleepStates newSleep, oldSleep;
	struct xbee_con *iCon;
	unsigned char buf[3];
	client = *data;
	if (!client->started) return;
	
	retVal = 0x02;
	
	if ((*pkt)->dataLen < 2 || (*pkt)->dataLen > 3) {
		goto err;
	}
	
	conIdentifier = 0;
	conIdentifier |= (((*pkt)->data[0]) << 8) & 0xFF;
	conIdentifier |= ((*pkt)->data[1]) & 0xFF;
	if ((*pkt)->dataLen == 3) newSleep = (*pkt)->data[2];
	
	for (iCon = NULL; ll_get_next(client->conList, iCon, (void**)&iCon) == XBEE_ENONE && iCon; ) {
		if (iCon->conIdentifier == conIdentifier) break;
	}
	if (!iCon) goto err;
	
	if (xbee_conSleepGet(iCon, &oldSleep) != XBEE_ENONE) goto err;
	if ((*pkt)->dataLen == 3) {
		if (xbee_conSleepSet(iCon, newSleep) != XBEE_ENONE) {
			retVal = 0x03; /* <-- failed to apply, old value present in reply */
		} else {
			retVal = 0x00;
		}
	}
	
	buf[0] = (*pkt)->frameId;
	buf[1] = retVal;
	buf[2] = oldSleep & 0xFF;
	xbee_connTx(con, NULL, buf, sizeof(buf));
	
	return;
err:
	{
		unsigned char buf[2];
		buf[0] = (*pkt)->frameId;
		buf[1] = retVal;
		xbee_connTx(con, NULL, buf, sizeof(buf));
	}
}

/* ######################################################################### */

void xbee_net_conSettings(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	client = *data;
	if (!client->started) return;
	
}

/* ######################################################################### */

void xbee_net_conEnd(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	unsigned char retVal;
	int conIdentifier;
	struct xbee_con *iCon;
	client = *data;
	if (!client->started) return;
	
	retVal = 0x02;
	
	if ((*pkt)->dataLen != 2) {
		goto err;
	}
	
	conIdentifier = 0;
	conIdentifier |= (((*pkt)->data[0]) << 8) & 0xFF;
	conIdentifier |= ((*pkt)->data[1]) & 0xFF;
	
	for (iCon = NULL; ll_get_next(client->conList, iCon, (void**)&iCon) == XBEE_ENONE && iCon; ) {
		if (iCon->conIdentifier == conIdentifier) {
			ll_ext_item(client->conList, iCon);
			xbee_conEnd(iCon);
			retVal = 0x00;
			break;
		}
	}
	
err:
	{
		unsigned char buf[2];
		buf[0] = (*pkt)->frameId;
		buf[1] = retVal;
		xbee_connTx(con, NULL, buf, sizeof(buf));
	}
}

/* ######################################################################### */

void xbee_net_conGetTypes(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	struct xbee_netClientInfo *client;
	int typeCount;
	struct xbee_buf *iBuf;
	int i, o, p;
	size_t bufLen;
	size_t memSize;
	struct xbee_modeConType *conType;
	
	client = *data;
	
	if (!client->started) return;
	
	memSize = 0;
	typeCount = 0;
	for (i = 0; xbee->iface.conTypes[i].name; i++) {
		if (xbee->iface.conTypes[i].internal) continue;
		typeCount++;
		memSize += strlen(xbee->iface.conTypes[i].name) + 2; /* 1 for '\0', 1 for flags */
	}
	
	memSize += 1; /* for an 8 bit 'count' */
	memSize += 2; /* for the frameId and return value */
	bufLen = memSize;
	
	memSize += sizeof(*iBuf);
	
	if ((iBuf = malloc(memSize)) == NULL) goto err;
	
	iBuf->len = bufLen;
	iBuf->data[0] = (*pkt)->frameId;
	iBuf->data[1] = 0x00; /* <-- success */
	iBuf->data[2] = typeCount;
	for (i = 0, p = 0, o = 3; xbee->iface.conTypes[i].name && p < typeCount; i++) {
		/* this order of conTypes HAS to match up with the order in net.c xbee_netServerThread() */
		if (xbee->iface.conTypes[i].internal) continue;
		p++;
		conType = &(xbee->iface.conTypes[i]);
		iBuf->data[o] = 0;
		if (conType->allowFrameId) iBuf->data[o] |= 0x01;
		if (conType->rxHandler)    iBuf->data[o] |= 0x02;
		if (conType->txHandler)    iBuf->data[o] |= 0x04;
		o++;
		o += snprintf((char *)&(iBuf->data[o]), iBuf->len - o, "%s", conType->name) + 1;
	}
	
	xbee_connTx(con, NULL, iBuf->data, iBuf->len);
	
	free(iBuf);
	
	return;
err:
	{
		unsigned char buf[2];
		buf[0] = (*pkt)->frameId;
		buf[1] = 0x01; /* <-- this means intenal error */
		xbee_connTx(con, NULL, buf, 2);
	}
}

/* ######################################################################### */

#define ADD_NETSERVERCALLBACK(cb) { .name = #cb, .callback = xbee_net_##cb },
const struct xbee_netCallback xbee_netServerCallbacks[] = {
	/* backchannel (0x00), endpoint 0 (0x00) is ALWAYS the 'start' function */
	ADD_NETSERVERCALLBACK(start) /* this MUST BE FIRST */
	/* the rest may be ordered for efficiency...
	   e.g: tx is probrably going to be the most commonly called */
	ADD_NETSERVERCALLBACK(connTx)
	ADD_NETSERVERCALLBACK(conRx)
	ADD_NETSERVERCALLBACK(conValidate)
	ADD_NETSERVERCALLBACK(conSleep)
	ADD_NETSERVERCALLBACK(conSettings)
	ADD_NETSERVERCALLBACK(conNew)
	ADD_NETSERVERCALLBACK(conEnd)
	ADD_NETSERVERCALLBACK(conGetTypes)
	/* these are 'system' functions */
	ADD_NETSERVERCALLBACK(echo)
	/* terminate */
	{ NULL, NULL },
};

#endif /* XBEE_NO_NET_SERVER */
