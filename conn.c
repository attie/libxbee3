/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with libxbee. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "internal.h"
#include "xbee_int.h"
#include "conn.h"
#include "pkt.h"
#include "mode.h"
#include "log.h"
#include "thread.h"
#include "frame.h"
#include "tx.h"
#include "ll.h"

struct xbee_ll_head *conList = NULL;

/* ########################################################################## */
static xbee_err _xbee_conFree(struct xbee_con *con);

xbee_err xbee_conAlloc(struct xbee_con **nCon) {
	size_t memSize;
	struct xbee_con *con;
	xbee_err ret;
	
	if (!nCon) return XBEE_EMISSINGPARAM;
	
	memSize = sizeof(*con);
	
	if (!(con = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(con, 0, memSize);
	con->pktList = xbee_ll_alloc();
	xsys_sem_init(&con->callbackSem);
	xsys_mutex_init(&con->txMutex);
	
	if ((ret = xbee_ll_add_tail(conList, con)) != XBEE_ENONE) {
		_xbee_conFree(con);
		ret = XBEE_ELINKEDLIST;
	} else {
		*nCon = con;
	}
	
	return ret;
}

xbee_err xbee_conFree(struct xbee_con *con) {
	xbee_err ret;
	
	if (!con) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	xbee_conUnlink(con);

	if (con->callbackThread) {
		con->ending = 1;
		xsys_sem_post(&con->callbackSem);
		if ((ret = xbee_threadJoin(con->xbee, con->callbackThread, NULL)) != XBEE_ENONE) {
			int i;
			if (ret != XBEE_EINUSE) return ret;
			
			/* wait patiently upto 50ms for the callback to finish its stuff */
			for (i = 10; i > 0; i--) {
				usleep(5000);
				if ((ret = xbee_threadJoin(con->xbee, con->callbackThread, NULL)) == XBEE_ENONE) break;
				if (ret == XBEE_EINUSE) continue;
				return ret;
			}
			
			/* if it's still not dead, then just kill it! */
			if (i == 0 && (ret = xbee_threadKillJoin(con->xbee, con->callbackThread, NULL)) != XBEE_ENONE) return ret;
		}
		con->callbackThread = NULL;
	}
	
	return _xbee_conFree(con);
}
	
static xbee_err _xbee_conFree(struct xbee_con *con) {
	xbee_ll_ext_item(conList, con);
	
	xbee_mutex_lock(&con->txMutex);
	
	xsys_mutex_destroy(&con->txMutex);
	xsys_sem_destroy(&con->callbackSem);
	xbee_ll_free(con->pktList, (void(*)(void*))xbee_pktFree);
	
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
	xbee_ll_lock(conType->conList);
	
	do {
		if ((ret = _xbee_ll_get_item(conType->conList, con, 0)) != XBEE_ENOTEXISTS) {
			if (ret == XBEE_ENONE) {
				ret = XBEE_EEXISTS;
			}
			break;
		}
		
		if ((ret = _xbee_conLocate(conType->conList, address, NULL, -1, 0)) != XBEE_ENOTEXISTS && 
		     ret != XBEE_ESLEEPING &&
		     ret != XBEE_ECATCHALL) {
			if (ret == XBEE_ENONE) {
				ret = XBEE_EEXISTS;
			}
			break;
		}
	
		if ((ret = _xbee_ll_add_tail(conType->conList, con, 0)) != XBEE_ENONE) {
			break;
		}
		
		con->xbee = xbee;
		con->conType = conType;
	} while (0);
	
	xbee_ll_unlock(conType->conList);
	
	return ret;
}

xbee_err xbee_conUnlink(struct xbee_con *con) {
	struct xbee *xbee;
	struct xbee_modeConType *conType;
	xbee_err ret;
	if (!con) return XBEE_EMISSINGPARAM;
	xbee = con->xbee;
	conType = con->conType;
	if (!xbee || !conType) return XBEE_EINVAL;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if ((ret = xbee_ll_ext_item(conType->conList, con)) != XBEE_ENONE) return ret;
	
	return ret;
}

/* ########################################################################## */

xbee_err xbee_conLogAddress(struct xbee *xbee, int minLogLevel, struct xbee_conAddress *address) {
	if (!address) return XBEE_EINVAL;
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

xbee_err xbee_conAddressCmp(struct xbee_conAddress *addr1, struct xbee_conAddress *addr2) {
	/* first try to match the address */
	if (!addr1->addr16_enabled && !addr2->addr16_enabled &&
	    !addr1->addr64_enabled && !addr2->addr64_enabled) {
		goto got1;
	}
	if (addr1->addr64_enabled && addr2->addr64_enabled) {
		if (!memcmp(addr1->addr64, addr2->addr64, 8)) goto got1;
	}
	if (addr1->addr16_enabled && addr2->addr16_enabled) {
		if (!memcmp(addr1->addr16, addr2->addr16, 2)) goto got1;
	}
	
	return XBEE_EFAILED; /* --- no address match --- */
	
got1:
	/* next try to match the endpoints */
	if (!addr1->endpoints_enabled && !addr2->endpoints_enabled) goto got2;
	if (addr1->endpoints_enabled && addr2->endpoints_enabled) {
		if (addr1->endpoint_local == addr2->endpoint_local) goto got2;
	}
	
	return XBEE_EFAILED; /* --- endpoints didnt match --- */
	
got2:
	return XBEE_ENONE;   /* --- everything matched --- */
}

xbee_err _xbee_conLocate(struct xbee_ll_head *conList, struct xbee_conAddress *address, struct xbee_con **retCon, enum xbee_conSleepStates alertLevel, int needsLLLock) {
	struct xbee_con *con;
	struct xbee_con *sCon;
	struct xbee_con *cCon;
	xbee_err ret;
	
	if (!conList || !address) return XBEE_EMISSINGPARAM;
	
	sCon = NULL;
	cCon = NULL;
	
	if (needsLLLock) xbee_ll_lock(conList);
	for (con = NULL; (ret = _xbee_ll_get_next(conList, con, (void**)&con, 0)) == XBEE_ENONE && con; ) {
		/* skip ending connections */
		if (con->ending) continue;
		
		/* next see if the connection and can be woken */
		if (con->sleepState > alertLevel) continue; /* this connection is outside the 'acceptable wake limit' */
		
		/* keep track of the latest catch-all */
		if (con->settings.catchAll) cCon = con;
		
		/* try to match the address */
		if (xbee_conAddressCmp(&con->address, address) != XBEE_ENONE) continue;
		
		/* is the connection dozing? */
		if (con->sleepState != CON_AWAKE) {
			/* this is designed to get the most recently created sleeping connection, NOT THE FIRST FOUND */
			sCon = con;
			continue;
		}
		break;
	}
	if (needsLLLock) xbee_ll_unlock(conList);
	
	/* did we find a sleepy/catchall connection? */
	if (!con) {
		if (sCon) {
			con = sCon;
			ret = XBEE_ESLEEPING;
		} else if (cCon) {
			con = cCon;
			ret = XBEE_ECATCHALL;
		}
	}

	if (!con) return XBEE_ENOTEXISTS;
	
	if (retCon) *retCon = con;
	
	return ret;
}
xbee_err xbee_conLocate(struct xbee_ll_head *conList, struct xbee_conAddress *address, struct xbee_con **retCon, enum xbee_conSleepStates alertLevel) {
	return _xbee_conLocate(conList, address, retCon, alertLevel, 1);
}

/* ########################################################################## */

EXPORT xbee_err xbee_conGetTypes(struct xbee *xbee, char ***retList) {
	int i, o, p;
	size_t memSize;
	char **tList;
	char *tName;
	struct xbee_modeConType *conTypes;
	if (!xbee || !retList) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */

	if (!xbee->iface.conTypes) return XBEE_EINVAL;

	conTypes = xbee->iface.conTypes;

	memSize = 0;
	p = 0;
	for (i = 0; conTypes[i].name; i++) {
		if (conTypes[i].internal) continue;
		memSize += sizeof(char *);
		memSize += sizeof(char) * (strlen(conTypes[i].name) + 1);
		p++;
	}
	memSize += sizeof(char *);

	if ((tList = malloc(memSize)) == NULL) {
		return XBEE_ENOMEM;
	}

	tName = (char *)&(tList[p+1]);
	o = p;
	p = 0;
	for (i = 0; conTypes[i].name && p < o; i++) {
		if (conTypes[i].internal) continue;
		tList[p] = tName;
		strcpy(tName, conTypes[i].name);
		tName += strlen(tName) + 1;
		p++;
	}
	tList[p] = NULL;

	*retList = tList;

	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err _xbee_conNew(struct xbee *xbee, struct xbee_interface *iface, int allowInternal, struct xbee_con **retCon, const char *type, struct xbee_conAddress *address) {
	xbee_err ret;
	int conIdentifier;
	struct xbee_con *con;
	struct xbee_modeConType *conType;

	if (!xbee || !iface || !iface->conTypes || !retCon || !type) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if ((ret = xbee_modeLocateConType(iface->conTypes, allowInternal, type, NULL, NULL, &conType)) != XBEE_ENONE) return ret;
	if (!conType) return XBEE_EUNKNOWN;
	
	if (conType->addressRules & ADDR_EP_NOTALLOW && ( address &&  address->endpoints_enabled))                          return XBEE_EINVAL;
	if (conType->addressRules & ADDR_EP_REQUIRED && (!address || !address->endpoints_enabled))                          return XBEE_EINVAL;
	if (conType->addressRules & ADDR_64_NOTALLOW && ( address &&  address->addr64_enabled))                             return XBEE_EINVAL;
	if (conType->addressRules & ADDR_16_NOTALLOW && ( address &&  address->addr16_enabled))                             return XBEE_EINVAL;
	if (conType->addressRules & ADDR_64_REQUIRED && (!address || !address->addr64_enabled))                             return XBEE_EINVAL;
	if (conType->addressRules & ADDR_16_REQUIRED && (!address || !address->addr16_enabled))                             return XBEE_EINVAL;
	if (conType->addressRules & ADDR_16OR64      && (!address || !(address->addr16_enabled | address->addr64_enabled))) return XBEE_EINVAL;
	if (conType->addressRules & ADDR_16XOR64     && (!address || !(address->addr16_enabled ^ address->addr64_enabled))) return XBEE_EINVAL;
	
	conIdentifier = 0;
	if (xbee->mode->support.conNew) {
		/* check with support system */
		if ((ret = xbee->mode->support.conNew(xbee, iface, conType, address, &conIdentifier)) != XBEE_ENONE) return ret;
	}
	
	if ((ret = xbee_conAlloc(&con)) != XBEE_ENONE) return ret;
	con->iface = iface;
	con->conIdentifier = conIdentifier;
	
	if (address) {
		memcpy(&con->address, address, sizeof(*address));
	} else {
		memset(&con->address, 0, sizeof(*address));
	}
	
	xbee_log(6, "Created new '%s' type connection", conType->name);
	xbee_conLogAddress(xbee, 8, address);
	
	if ((ret = xbee_conLink(xbee, conType, &con->address, con)) != XBEE_ENONE) {
		xbee_conFree(con);
		return ret;
	}
	
	*retCon = con;
	
	return XBEE_ENONE;
}
EXPORT xbee_err xbee_conNew(struct xbee *xbee, struct xbee_con **retCon, const char *type, struct xbee_conAddress *address) {
	return _xbee_conNew(xbee, &xbee->iface, 0, retCon, type, address);
}

EXPORT xbee_err xbee_conValidate(struct xbee_con *con) {
	xbee_err ret;
	
	if (xbee_ll_get_item(conList, con) != XBEE_ENONE) return XBEE_EINVAL;
	
	if (con->xbee && con->xbee->mode->support.conValidate) {
		/* check with support system */
		if ((ret = con->xbee->mode->support.conValidate(con)) != XBEE_ENONE) return ret;
	}
	return XBEE_ENONE;
}

/* ########################################################################## */

xbee_err xbee_conWake(struct xbee_con *con) {
	xbee_err ret;
	struct xbee_con *iCon;

	if (!con) return XBEE_EMISSINGPARAM;
	if (!con->conType) return XBEE_EINVAL;
	if (con->sleepState == CON_AWAKE) return XBEE_ENONE;
	
	ret = XBEE_ENONE;
	
	for (iCon = NULL; _xbee_ll_get_next(con->conType->conList, iCon, (void**)&iCon, 0) == XBEE_ENONE && iCon != NULL; ) {
		/* discount ourselves */
		if (iCon == con) continue;
		
		/* try to match the addresses */
		if (xbee_conAddressCmp(&con->address, &iCon->address) != XBEE_ENONE) continue;
		
		/* check if it's awake */
		if (iCon->sleepState != CON_AWAKE) continue;
		
		/* if it is, then we can't wake up */
		ret = XBEE_EFAILED;
		break;
	}
	if (ret == XBEE_ENONE) {
		/* we can wakeup! */
		con->sleepState = CON_AWAKE;
	}
	
	xbee_ll_unlock(con->conType->conList);
	return ret;
}

/* ########################################################################## */

xbee_err _xbee_convTx(struct xbee_con *con, unsigned char *retVal, const char *format, va_list args);
xbee_err _xbee_connTx(struct xbee_con *con, unsigned char *retVal, const unsigned char *buf, int len);

EXPORT xbee_err xbee_conTx(struct xbee_con *con, unsigned char *retVal, const char *format, ...) {
	xbee_err ret;
	va_list ap;
	
	if (!con || !format) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	va_start(ap, format);
	ret = _xbee_convTx(con, retVal, format, ap);
	va_end(ap);
	
	return ret;
}

EXPORT xbee_err xbee_convTx(struct xbee_con *con, unsigned char *retVal, const char *format, va_list args) {
	if (!con || !format) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */

	return _xbee_convTx(con, retVal, format, args);
}
xbee_err _xbee_convTx(struct xbee_con *con, unsigned char *retVal, const char *format, va_list args) {
	xbee_err ret;
	int bufLen, outLen;
	char *buf;
	va_list args1;
	
	if (!con || !format) return XBEE_EMISSINGPARAM;
	
	va_copy(args1, args);
	bufLen = vsnprintf(NULL, 0, format, args1);
	va_end(args1);
	
	if (bufLen > 0) {
		if (!(buf = malloc(bufLen + 1))) { /* +1 for the terminating '\0' */
			return XBEE_ENOMEM;
		}
		outLen = vsnprintf(buf, bufLen + 1, format, args);
		if (outLen > bufLen) {
			ret = XBEE_ERANGE;
			goto die;
		}
	} else {
		buf = NULL;
		outLen = 0;
	}
	
	ret = _xbee_connTx(con, retVal, (unsigned char*)buf, outLen);
	
die:
	if (buf) free(buf);
	return ret;
}

EXPORT xbee_err xbee_connTx(struct xbee_con *con, unsigned char *retVal, const unsigned char *buf, int len) {
	if (!con) return XBEE_EMISSINGPARAM;
	if (len < 0) return XBEE_EINVAL;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	return _xbee_connTx(con, retVal, buf, len);
}
xbee_err _xbee_connTx(struct xbee_con *con, unsigned char *retVal, const unsigned char *buf, int len) {
	int waitForAck;
	xbee_err ret;
	unsigned char myret;
	unsigned char *pret;
	unsigned char nada;
	nada = 0x00;
	
	if (!con) return XBEE_EMISSINGPARAM;
	if (len < 0) return XBEE_EINVAL;

	if (con->sleepState != CON_AWAKE) {
		if (con->sleepState != CON_SNOOZE) {
			/* can't transmit when we are deeper than snooze */
			return XBEE_ESLEEPING;
		}
		if (xbee_conWake(con) != XBEE_ENONE) {
			/* can't transmit while another con is awake */
			return XBEE_ESLEEPING;
		}
	}

	if (!buf) {
		len = 0;
		buf = &nada;
	}

	/* we ALWAYS want to be able to check the response value */
	pret = ((!retVal)?&myret:retVal);
	*pret = 0;

	if (con->settings.noBlock) {
		if (xbee_mutex_trylock(&con->txMutex)) return XBEE_EWOULDBLOCK;
	} else {
		xbee_mutex_lock(&con->txMutex);
	}
	
	if (!con->conType->allowFrameId) {
		waitForAck = 0;
		con->frameId = 0;
	} else {
		waitForAck = !(con->settings.disableAck || con->settings.broadcast); /* cache it, incase it changes */
		if (waitForAck) {
			if ((ret = xbee_frameGetFreeID(con->xbee->fBlock, con)) != XBEE_ENONE) {
				ret = XBEE_ENOFREEFRAMEID;
				goto done;
			}
		} else {
			con->frameId = 0; /* status response disabled */
		}
	}
	
	if ((ret = xbee_txHandler(con, buf, len)) != XBEE_ENONE) goto done;

	if (waitForAck) {
		struct timespec to;
		clock_gettime(CLOCK_REALTIME, &to);
		if (con->conType->useTimeout) {
			to.tv_sec  += con->conType->timeout.tv_sec;
			to.tv_nsec += con->conType->timeout.tv_nsec;
			while (to.tv_nsec >= 1000000000) {
				to.tv_sec++;
				to.tv_nsec -= 1000000000;
			}
		} else {
			to.tv_sec += 1; /* default 1 second timeout */
		}
		if (xbee_frameWait(con->xbee->fBlock, con, pret, &to) != XBEE_ENONE || *pret != 0) ret = XBEE_ETX;
	}
	
done:
	xbee_mutex_unlock(&con->txMutex);
	
	return ret;
}

/* ########################################################################## */

xbee_err xbee_conLinkPacket(struct xbee_con *con, struct xbee_pkt *pkt) {
	xbee_err ret;
	if (!con || !pkt) return XBEE_EMISSINGPARAM;
	if ((ret = xbee_ll_add_tail(con->pktList, pkt)) != XBEE_ENONE) return ret;
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
	
	xbee_ll_lock(con->pktList);
	if ((ret = _xbee_ll_count_items(con->pktList, &remain, 0)) != XBEE_ENONE) goto die;
	if (remain == 0) {
		*retPkt = NULL;
		ret = XBEE_ENOTEXISTS;
		goto die;
	}
	_xbee_ll_ext_head(con->pktList, (void**)&pkt, 0);
	_xbee_pktUnlink(con, pkt, 0);
	*retPkt = pkt;
die:
	xbee_ll_unlock(con->pktList);

	if (remainingPackets) *remainingPackets = (remain > 0 ? remain - 1 : 0);
	
	return ret;
}
EXPORT xbee_err xbee_conRxWait(struct xbee_con *con, struct xbee_pkt **retPkt, int *remainingPackets) {
	xbee_err ret;
	int i;

	ret = XBEE_EUNKNOWN;

	/* 50ms * 20 = 1second */
	for (i = 20; i > 0; i--) {
		/* break on success, or any error other than XBEE_ENOTEXISTS (nothing to Rx) */
		if ((ret = xbee_conRx(con, retPkt, remainingPackets)) != XBEE_ENOTEXISTS) break;
		usleep(50000);
	}

	return ret;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conPurge(struct xbee_con *con) {
	struct xbee_pkt *pkt;
	while (xbee_conRx(con, &pkt, NULL) == XBEE_ENONE && pkt) {
		xbee_pktFree(pkt);
	}
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conSleepSet(struct xbee_con *con, enum xbee_conSleepStates state) {
	xbee_err ret;
	if (!con) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if (con->xbee->mode->support.conSleepSet) {
		/* check with support system */
		if ((ret = con->xbee->mode->support.conSleepSet(con, state)) != XBEE_ENONE) return ret;
	}
	
	ret = XBEE_ENONE;
	
	if (state == CON_AWAKE) {
		/* we need to check if we can wakeup */
		ret = xbee_conWake(con); /* <-- this will set us to awake if it is successful */
	} else {
		con->sleepState = state;
	}
	
	return ret;
}

EXPORT xbee_err xbee_conSleepGet(struct xbee_con *con, enum xbee_conSleepStates *state) {
	xbee_err ret;
	if (!con || !state) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if (con->xbee->mode->support.conSleepGet) {
		/* check with support system */
		if ((ret = con->xbee->mode->support.conSleepGet(con)) != XBEE_ENONE) return ret;
	}
	
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

	while (!con->ending) {
		callback = con->callback;
		if (!callback) break;
		if ((ret = xbee_ll_ext_head(con->pktList, (void**)&pkt)) == XBEE_ERANGE) {
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
		callback(xbee, con, &pkt, &con->userData);

		if (pkt) {
			if (pkt == oPkt) {
				xbee_pktFree(pkt);
			} else {
				xbee_log(-1, "callback for connection @ %p returned a different packet to what it was provided...");
			}
		}
	}
	
	*restart = 0;
	return XBEE_ENONE;
}

xbee_err xbee_conCallbackProd(struct xbee_con *con) {
	struct xbee *xbee;
	xbee_err ret;
	unsigned int count;

	if (!con) return XBEE_EMISSINGPARAM;
	if (!con->callback) return XBEE_ENONE;

	if (xbee_ll_count_items(con->pktList, &count) != XBEE_ENONE) return XBEE_ELINKEDLIST;
	if (count == 0) return XBEE_ENONE;

	xbee = con->xbee;

	xsys_sem_post(&con->callbackSem);

	if (con->callbackThread) {
		xbee_err ret2;
		
#ifndef _WIN32
#warning TODO - there is a gap here, needs a mutex
#endif
		if (con->callbackThread->active) return XBEE_ENONE;
		
		if ((ret = xbee_threadJoin(con->xbee, con->callbackThread, &ret2)) != XBEE_ENONE) return ret;
		con->callbackThread = NULL;
		if (ret2 != XBEE_ENONE) {
			xbee_log(3, "dead callback for con @ %p returned %d...", con, ret2);
		}
	}
	
	if ((ret = xbee_threadStart(con->xbee, &con->callbackThread, 0, 0, xbee_conCallbackHandler, con)) != XBEE_ENONE) return ret;
	
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
	xbee_err ret;
	struct xbee_conSettings tempOld;
	if (!con || (!newSettings && !oldSettings)) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	
	if (oldSettings) memcpy(&tempOld, &con->settings, sizeof(con->settings));
	
	if (con->xbee->mode->support.conSettings) {
		/* check with support system - this will update the current settings of the con */
		if ((ret = con->xbee->mode->support.conSettings(con, newSettings)) != XBEE_ENONE) return ret;
	}
	
	if (oldSettings) memcpy(oldSettings, &tempOld, sizeof(con->settings));
	if (newSettings) memcpy(&con->settings, newSettings, sizeof(con->settings));
	
	return XBEE_ENONE;
}

/* ########################################################################## */

EXPORT xbee_err xbee_conEnd(struct xbee_con *con) {
	xbee_err ret;
	xbee_err ret2;

	ret = XBEE_ENONE;
	if (con->xbee->mode->support.conEnd) {
		/* check with support system */
		ret = con->xbee->mode->support.conEnd(con);
		if (ret != XBEE_ENONE && ret != XBEE_ESTALE) return ret;
	}
	
	if ((ret2 = xbee_conFree(con)) != XBEE_ENONE) return ret2;
	
	return ret;
}
