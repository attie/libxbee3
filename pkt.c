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

struct ll_head *pktList = NULL;

/* ########################################################################## */

#define PKT_DATAKEY_MAXLEN 32
struct pkt_dataKey {
	char name[PKT_DATAKEY_MAXLEN]; /* eg: 'analog' */
	int id; /* eg: (channel) 3 */
	struct ll_head *items; /* this contains a list of data, which CAN be raw data cast to a void*, eg: 524 */
	void (*freeCallback)(void*); /* can only be assigned once for each key */
};

/* ########################################################################## */

xbee_err xbee_pktAlloc(struct xbee *xbee, struct xbee_pkt **nPkt, struct xbee_pkt *oPkt, int dataLen) {
	size_t memSize;
	struct xbee_pkt *pkt;
	xbee_err ret;
	
	if (!xbee || !nPkt) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if (oPkt) {
		if ((ret = ll_ext_item(pktList, oPkt)) != XBEE_ENONE) {
			return ret;
		}
	}
	
	memSize = sizeof(*pkt);
	memSize += sizeof(char) * dataLen;
	
	if (!(pkt = realloc(oPkt, memSize))) return XBEE_ENOMEM;
	
	if (!oPkt) {
		memset(pkt, 0, memSize);
		pkt->dataItems = ll_alloc();
	}
	pkt->xbee = xbee;
	
	if ((ret = ll_add_tail(pktList, pkt)) != XBEE_ENONE) {
		free(pkt);
		ret = XBEE_ELINKEDLIST;
	}
	
	*nPkt = pkt;
	return ret;
}

EXPORT xbee_err xbee_pktFree(struct xbee_pkt *pkt) {
	if (!pkt) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	ll_ext_item(pktList, pkt);
	free(pkt);
	
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_pktValidate(struct xbee_pkt *pkt) {
	if (ll_get_item(pktList, pkt) != XBEE_ENONE) return XBEE_EINVAL;
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_pktLink(struct xbee_con *con, struct xbee_pkt *pkt) {
	xbee_err ret;
	if (!con || !pkt) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_pktValidate(pkt) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (ll_get_item(con->pktList, pkt) == XBEE_ENONE) return XBEE_EEXISTS;
	if ((ret = ll_add_tail(con->pktList, pkt)) == XBEE_ENONE) {
		pkt->xbee = con->xbee;
		pkt->con = con;
	}
	return ret;
}

xbee_err xbee_pktUnlink(struct xbee_con *con, struct xbee_pkt *pkt) {
	xbee_err ret;
	if (!con || !pkt) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_pktValidate(pkt) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if ((ret = ll_ext_item(con->pktList, pkt)) == XBEE_ENONE) {
		pkt->xbee = NULL;
		pkt->con = NULL;
	}
	return ret;
}

/* ########################################################################## */

xbee_err xbee_pktDataKeyAdd(struct xbee_pkt *pkt, char *key, int id, struct pkt_dataKey **retKey, void (*freeCallback)(void*)) {
	struct pkt_dataKey *k;
	xbee_err ret;
	
	if (!pkt || !key) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if (xbee_pktDataKeyGet(pkt, key, id, &k) == XBEE_ENONE) {
		if (retKey) *retKey = k;
		return XBEE_EEXISTS;
	}
	
	if ((k = calloc(1, sizeof(*k))) == NULL) {
		return XBEE_ENOMEM;
	}
	
	ret = XBEE_ENONE;
	snprintf(k->name, PKT_DATAKEY_MAXLEN, "%s", key);
	k->id = id;
	k->freeCallback = freeCallback;
	if ((k->items = ll_alloc()) == NULL) {
		ret = XBEE_ENOMEM;
		goto die1;
	}
	
	if (ll_add_tail(pkt->dataItems, k) != XBEE_ENONE) {
		ret = XBEE_ELINKEDLIST;
		goto die2;
	}
	
	goto done;
die2:
	ll_free(k->items, NULL);
die1:
	free(k);
done:
	return ret;
}

xbee_err xbee_pktDataKeyGet(struct xbee_pkt *pkt, char *key, int id, struct pkt_dataKey **retKey) {
	struct pkt_dataKey *k;
	
	if (!pkt || !key) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	ll_lock(pkt->dataItems);
	for (k = NULL; (_ll_get_next(pkt->dataItems, k, (void**)&k, 0) == XBEE_ENONE) && k; ) {
		if (!strncasecmp(key, k->name, PKT_DATAKEY_MAXLEN)) {
			if (id == -1 || id == k->id) {
				if (retKey) *retKey = k;
				return XBEE_ENONE;
			}
		}
	}
	
	return XBEE_EFAILED;
}

/* ########################################################################## */

xbee_err xbee_pktDataAdd(struct xbee_pkt *pkt, char *key, int id, void *data, void (*freeCallback)(void*)) {
	struct pkt_dataKey *k;
	xbee_err ret;
	
	if (!pkt || !key || !data) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if ((ret = xbee_pktDataKeyAdd(pkt, key, id, &k, freeCallback)) != XBEE_ENONE && ret != XBEE_EEXISTS) {
		return XBEE_EFAILED;
	}
	
	if (ll_add_tail(k->items, data)) {
		return XBEE_ELINKEDLIST;
	}
	
	return XBEE_ENONE;
}

xbee_err xbee_pktDataGet(struct xbee_pkt *pkt, char *key, int id, int index, void **retData) {
	struct pkt_dataKey *k;
	unsigned int count;
	xbee_err ret;
	
	if (!pkt || !key || !retData) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if ((ret = xbee_pktDataKeyGet(pkt, key, id, &k)) != XBEE_ENONE) return ret;
	
	if (ll_count_items(k->items, &count) != XBEE_ENONE) return XBEE_ELINKEDLIST;
	if (index >= count) return XBEE_ERANGE;
	
	if (ll_get_index(k->items, index, retData) != XBEE_ENONE) return XBEE_EINVAL;
	
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_pktAnalogAdd(struct xbee_pkt *pkt, int channel, int value) {
	if (!pkt) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	return xbee_pktDataAdd(pkt, "analog", channel, (void*)&value, NULL);
}

EXPORT xbee_err xbee_pktAnalogGet(struct xbee_pkt *pkt, int channel, int index, int *retVal) {
	xbee_err ret;
	
	if (!pkt || !retVal) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if ((ret = xbee_pktDataGet(pkt, "analog", channel, index, (void*)retVal)) != XBEE_ENONE) return ret;
	
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_pktDigitalAdd(struct xbee_pkt *pkt, int channel, int value) {
	if (!pkt) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	value = !!value;
	value += 1;
	
	return xbee_pktDataAdd(pkt, "digital", channel, (void*)&value, NULL);
}

EXPORT xbee_err xbee_pktDigitalGet(struct xbee_pkt *pkt, int channel, int index, int *retVal) {
	int value;
	xbee_err ret;
	
	if (!pkt || !retVal) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_pktValidate(pkt)) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if ((ret = xbee_pktDataGet(pkt, "digital", channel, index, (void*)&value)) != XBEE_ENONE) return ret;
	value -= 1;
	value = !!value;
	*retVal = value;
	
	return XBEE_ENONE;
}
