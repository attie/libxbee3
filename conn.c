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
#include <errno.h>

#include "internal.h"
#include "conn.h"
#include "xbee_int.h"
#include "pkt.h"
#include "mode.h"
#include "log.h"
#include "thread.h"
#include "frame.h"
#include "tx.h"
#include "ll.h"

struct ll_head *conList = NULL;

/* ########################################################################## */
static inline xbee_err _xbee_conFree(struct xbee_con *con);

xbee_err xbee_conAlloc(struct xbee_con **nCon) {
	size_t memSize;
	struct xbee_con *con;
	xbee_err ret;
	
	if (!nCon) return XBEE_EMISSINGPARAM;
	
	memSize = sizeof(*con);
	
	if (!(con = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(con, 0, memSize);
	con->pktList = ll_alloc();
	xsys_sem_init(&con->callbackSem);
	xsys_mutex_init(&con->txMutex);
	
	if ((ret = ll_add_tail(conList, con)) != XBEE_ENONE) {
		_xbee_conFree(con);
		ret = XBEE_ELINKEDLIST;
	} else {
		*nCon = con;
	}
	
	return ret;
}

xbee_err xbee_conFree(struct xbee_con *con) {
	if (!con) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	return _xbee_conFree(con);
}
	
static inline xbee_err _xbee_conFree(struct xbee_con *con) {
	ll_ext_item(conList, con);
	
	xsys_mutex_destroy(&con->txMutex);
	xsys_sem_destroy(&con->callbackSem);
	ll_free(con->pktList, (void(*)(void*))xbee_pktFree);
	
	free(con);
	
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conLink(struct xbee *xbee, struct xbee_modeConType *conType, struct xbee_conAddress *address, struct xbee_con *con) {
	xbee_err ret;
	if (!xbee || !conType || !con) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	ret = XBEE_ENONE;
	ll_lock(conType->conList);
	
	do {
		if ((ret = _ll_get_item(conType->conList, con, 0)) != XBEE_ENOTEXISTS) {
			if (ret == XBEE_ENONE) {
				ret = XBEE_EEXISTS;
			}
			break;
		}
		
		if ((ret = _xbee_conMatchAddress(conType->conList, address, NULL, -1, 0)) != XBEE_ENOTEXISTS) {
			if (ret == XBEE_ENONE) {
				ret = XBEE_EEXISTS;
			}
			break;
		}
	
		if ((ret = _ll_add_tail(conType->conList, con, 0)) != XBEE_ENONE) {
			break;
		}
		
		con->xbee = xbee;
		con->conType = conType;
	} while (0);
	
	ll_unlock(conType->conList);
	
	return ret;
}

xbee_err xbee_conUnlink(struct xbee *xbee, struct xbee_modeConType *conType, struct xbee_con *con) {
	xbee_err ret;
	if (!xbee || !conType || !con) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if ((ret = ll_ext_item(conType->conList, con)) != XBEE_ENONE) return ret;
	
	con->xbee = NULL;
	con->conType = NULL;
	
	return ret;
}

/* ########################################################################## */

xbee_err xbee_conLogAddress(struct xbee *xbee, int minLogLevel, struct xbee_conAddress *address) {
	xbee_log(minLogLevel, "address @ %p...", address);
	if (address->addr16_enabled) {
		xbee_log(minLogLevel, "   16-bit address:  0x%02X%02X", address->addr16[0], address->addr16[1]);
	} else {
		xbee_log(minLogLevel, "   16-bit address:  --");
	}
	if (address->addr64_enabled) {
		xbee_log(minLogLevel, "   64-bit address:  0x%02X%02X%02X%02X 0x%02X%02X%02X%02X",
		                      address->addr64[0], address->addr64[1], address->addr64[2], address->addr64[3],
		                      address->addr64[4], address->addr64[5], address->addr64[6], address->addr64[7]);
	} else {
		xbee_log(minLogLevel, "   64-bit address:  --");
	}
	if (address->endpoints_enabled) {
		xbee_log(minLogLevel, "   endpoints:       local(0x%02X) remote(0x%02X)", address->endpoint_local, address->endpoint_remote);
	} else {
		xbee_log(minLogLevel, "   endpoints:       --");
	}
	return XBEE_ENONE;
}

xbee_err _xbee_conMatchAddress(struct ll_head *conList, struct xbee_conAddress *address, struct xbee_con **retCon, enum xbee_conSleepStates alertLevel, int needsLLLock) {
	struct xbee_con *con;
	struct xbee_con *sCon;
	xbee_err ret;
	xbee_err sRet;
	
	if (!conList || !address) return XBEE_EMISSINGPARAM;
	
	sCon = NULL;
	
	if (needsLLLock) ll_lock(conList);
	for (con = NULL; (ret = _ll_get_next(conList, con, (void**)&con, 0)) == XBEE_ENONE && con; ) {
		if (!memcmp(&con->address, address, sizeof(*address))) {
			if (con->sleepState > alertLevel) continue;
			if (con->sleepState != CON_AWAKE) {
				sCon = con;
				sRet = ret;
				continue;
			}
			break;
		}
	}
	if (needsLLLock) ll_unlock(conList);
	
	if (!con && sCon) {
		con = sCon;
		ret = sRet;
	}
	if (con && retCon) *retCon = con;

	if (!con) return XBEE_ENOTEXISTS;
	
	return ret;
}
xbee_err xbee_conMatchAddress(struct ll_head *conList, struct xbee_conAddress *address, struct xbee_con **retCon, enum xbee_conSleepStates alertLevel) {
	return _xbee_conMatchAddress(conList, address, retCon, alertLevel, 1);
}

/* ########################################################################## */

EXPORT xbee_err xbee_conGetTypes(struct xbee *xbee, char ***retList) {
#warning INFO - needs info from remote, this info should be retrieved at setup
	int i, o;
	size_t memSize;
	char **tList;
	char *tName;
	struct xbee_modeConType *conTypes;
	if (!xbee || !retList) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */

	if (!xbee->conTypes) return XBEE_EINVAL;

	conTypes = xbee->conTypes;

	memSize = 0;
	for (i = 0; conTypes[i].name; i++) {
		memSize += sizeof(char *);
		memSize += sizeof(char) * (strlen(conTypes[i].name) + 1);
	}
	memSize += sizeof(char *);

	if ((tList = malloc(memSize)) == NULL) {
		return XBEE_ENOMEM;
	}

	tName = (char *)&(tList[i+1]);
	o = i;
	for (i = 0; conTypes[i].name && i < o; i++) {
		tList[i] = tName;
		strcpy(tName, conTypes[i].name);
		tName += strlen(tName) + 1;
	}
	tList[i] = NULL;

	*retList = tList;

	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conNew(struct xbee *xbee, struct xbee_con **retCon, char *type, struct xbee_conAddress *address) {
#warning INFO - needs remote
	xbee_err ret;
	struct xbee_con *con;
	struct xbee_modeConType *conType;
	if (!xbee || !retCon || !type) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if ((ret = xbee_modeLocateConType(xbee->conTypes, type, NULL, NULL, &conType)) != XBEE_ENONE) return ret;
	
	if ((ret = xbee_conAlloc(&con)) != XBEE_ENONE) return ret;
	
	memcpy(&con->address, address, sizeof(*address));
	
	if ((ret = xbee_conLink(xbee, conType, &con->address, con)) != XBEE_ENONE) {
		xbee_conFree(con);
		return ret;
	}
	
	*retCon = con;
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_conValidate(struct xbee_con *con) {
#warning INFO - needs remote (if remote fails/contradicts, is XBEE_ESTALE)
	if (ll_get_item(conList, con) != XBEE_ENONE) return XBEE_EINVAL;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conTx(struct xbee_con *con, unsigned char *retVal, char *format, ...) {
	xbee_err ret;
	va_list ap;
	
	if (!con || !format) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	va_start(ap, format);
	ret = xbee_convTx(con, retVal, format, ap);
	va_end(ap);
	
	return ret;
}

EXPORT xbee_err xbee_convTx(struct xbee_con *con, unsigned char *retVal, char *format, va_list args) {
	xbee_err ret;
	int bufLen, outLen;
	char *buf;
	
	if (!con || !format) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
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
	
	ret = xbee_connTx(con, retVal, (unsigned char*)buf, outLen);
	
die:
	if (buf) free(buf);
	return ret;
}

EXPORT xbee_err xbee_connTx(struct xbee_con *con, unsigned char *retVal, unsigned char *buf, int len) {
#warning INFO - needs remote, can return XBEE_ESTALE
	int waitForAck;
	xbee_err ret;
	
	if (!con || !buf) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */

	if (con->settings.noBlock) {
		if (xsys_mutex_trylock(&con->txMutex)) return XBEE_EWOULDBLOCK;
	} else {
		xsys_mutex_lock(&con->txMutex);
	}

	if (!con->conType->allowFrameId) {
		waitForAck = 0;
		con->frameId = 0;
	} else {
		waitForAck = !con->settings.disableAck; /* cache it, incase it changes */
		if (waitForAck) {
			if ((ret = xbee_frameGetFreeID(con->xbee->fBlock, con)) != XBEE_ENONE) {
				ret = XBEE_ENOFREEFRAMEID;
				goto done;
			}
		} else {
			con->frameId = 0; /* status response disabled */
		}
	}
	
	ret = xbee_txHandler(con, buf, len);

	if (waitForAck) {
		struct timespec to;
		clock_gettime(CLOCK_REALTIME, &to);
		if (con->conType->useTimeout) {
			to.tv_sec  += con->conType->timeout.tv_sec;
			to.tv_nsec += con->conType->timeout.tv_nsec;
		} else {
			to.tv_sec += 1; /* default 1 second timeout */
		}
		if (xbee_frameWait(con->xbee->fBlock, con, retVal, &to) != XBEE_ENONE) ret = XBEE_ETX;
	}
	
done:
	xsys_mutex_unlock(&con->txMutex);
	
	return ret;
}

/* ########################################################################## */

xbee_err xbee_conLinkPacket(struct xbee_con *con, struct xbee_pkt *pkt) {
	xbee_err ret;
	if (!con || !pkt) return XBEE_EMISSINGPARAM;
	if ((ret = ll_add_tail(con->pktList, pkt)) != XBEE_ENONE) return ret;
	if (con->callback) return xbee_conCallbackProd(con);
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_conRx(struct xbee_con *con, struct xbee_pkt **retPkt, int *remainingPackets) {
	xbee_err ret;
	unsigned int remain;
	struct xbee_pkt *pkt;
	if (!con || !retPkt) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (con->callback != NULL) return XBEE_EINVAL;
	
	ret = XBEE_ENONE;
	remain = 0;
	
	ll_lock(con->pktList);
	if ((ret = _ll_count_items(con->pktList, &remain, 0)) != XBEE_ENONE) goto die;
	if (remain == 0) {
		*retPkt = NULL;
		ret = XBEE_ENOTEXISTS;
		goto die;
	}
	_ll_ext_head(con->pktList, (void**)&pkt, 0);
	_xbee_pktUnlink(con, pkt, 0);
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
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	con->sleepState = state;
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_conSleepGet(struct xbee_con *con, enum xbee_conSleepStates *state) {
#warning INFO - needs remote (if remote fails/contradicts, update local or is XBEE_ESTALE)
	if (!con || !state) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	*state = con->sleepState;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conDataSet(struct xbee_con *con, void *newData, void **oldData) {
	if (!con) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (oldData) *oldData = con->userData;
	con->userData = newData;
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_conDataGet(struct xbee_con *con, void **curData) {
	if (!con || !curData) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	*curData = con->userData;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conInfoGet(struct xbee_con *con, struct xbee_conInfo *info) {
	if (!con || !info) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	memcpy(info, &con->info, sizeof(con->info));
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conCallbackHandler(struct xbee *xbee, int *restart, void *arg) {
	struct xbee_con *con;
	struct xbee_pkt *pkt, *oPkt;
	xbee_err ret;
	xbee_t_conCallback callback;

	con = arg;

	do {
		callback = con->callback;
		if (!callback) break;
		if ((ret = ll_ext_head(con->pktList, (void**)&pkt)) == XBEE_ERANGE) {
			struct timespec to;
			clock_gettime(CLOCK_REALTIME, &to);
			to.tv_sec += 5; /* 5 second timeout */
			if (xsys_sem_timedwait(&con->callbackSem, &to)) {
				if (errno == ETIMEDOUT) break;
				return XBEE_ESEMAPHORE;
			}
			continue;
		} else if (ret != XBEE_ENONE) {
			return ret;
		}

		xbee_log(8, "connection @ %p got packet @ %p, about to hand to callback function @ %p...", con, pkt, callback);

		oPkt = pkt;
		callback(xbee, con, &pkt);

		if (pkt) {
			if (pkt == oPkt) {
				xbee_pktFree(pkt);
			} else {
				xbee_log(-1, "callback for connection @ %p returned a different packet to what it was provided...");
			}
		}
	} while (1);

	*restart = 0;
	return XBEE_ENONE;
}

xbee_err xbee_conCallbackProd(struct xbee_con *con) {
	struct xbee *xbee;
	xbee_err ret;
	int active;
	unsigned int count;

	if (!con) return XBEE_EMISSINGPARAM;
	if (!con->callback) return XBEE_ENONE;

	if (ll_count_items(con->pktList, &count) != XBEE_ENONE) return XBEE_ELINKEDLIST;
	if (count == 0) return XBEE_ENONE;

	xbee = con->xbee;

	xsys_sem_post(&con->callbackSem);

	if (con->callbackStarted) {
		xbee_err ret2;

		if ((ret = xbee_threadGetState(con->xbee, con->callbackThread, NULL, &active)) != XBEE_ENONE) return ret;
		if (active) return XBEE_ENONE;

		if ((ret = xbee_threadJoin(con->xbee, con->callbackThread, &ret2)) != XBEE_ENONE) return ret;
		if (ret2 != XBEE_ENONE) {
			xbee_log(3, "dead callback for con @ %p returned %d...", con, ret2);
		}
	}

	if (!con->callbackStarted || !active) {
		con->callbackStarted = 1;
		if ((ret = xbee_threadStart(con->xbee, &con->callbackThread, 0, xbee_conCallbackHandler, con)) != XBEE_ENONE) return ret;
	}

	return XBEE_ENONE;
}

EXPORT xbee_err xbee_conCallbackSet(struct xbee_con *con, xbee_t_conCallback newCallback, xbee_t_conCallback *oldCallback) {
	if (!con) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (oldCallback) *oldCallback = con->callback;
	con->callback = newCallback;
	return xbee_conCallbackProd(con);
}

EXPORT xbee_err xbee_conCallbackGet(struct xbee_con *con, xbee_t_conCallback *curCallback) {
	if (!con || !curCallback) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	*curCallback = con->callback;
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conSettings(struct xbee_con *con, struct xbee_conSettings *newSettings, struct xbee_conSettings *oldSettings) {
#warning INFO - needs remote, can return XBEE_ESTALE
	if (!con || (!newSettings && !oldSettings)) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (oldSettings) memcpy(oldSettings, &con->settings, sizeof(con->settings));
	if (newSettings) memcpy(&con->settings, newSettings, sizeof(con->settings));
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conEnd(struct xbee_con *con) {
#warning INFO - needs remote, can return XBEE_ESTALE
	return xbee_conFree(con);
}
