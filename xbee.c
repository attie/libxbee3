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
#include <stdarg.h>
#include <string.h>

#include "internal.h"
#include "xbee_int.h"
#include "conn.h"
#include "frame.h"
#include "thread.h"
#include "log.h"
#include "mode.h"
#include "rx.h"
#include "tx.h"
#include "net.h"
#include "ll.h"

struct ll_head *xbeeList = NULL;
struct ll_head *needsFree = NULL;

EXPORT xbee_err xbee_validate(struct xbee *xbee) {
	if (ll_get_item(xbeeList, xbee) != XBEE_ENONE) return XBEE_EINVAL;
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_alloc(struct xbee **nXbee) {
	size_t memSize;
	struct xbee *xbee;
	char *e;
	int logLevel;
	xbee_err ret;
	
	if (!nXbee) return XBEE_EMISSINGPARAM;

	memSize = sizeof(*xbee);

#ifdef XBEE_LOG_LEVEL
	logLevel = XBEE_LOG_LEVEL;
#else
	logLevel = 0;
#endif

	if ((e = getenv("XBEE_LOG_LEVEL")) != NULL) {
		int l;
		if (sscanf(e, "%d", &l) != 1) {
			fprintf(stderr, "libxbee: Failed to initialize log level from environment (not a number)\n");
		} else {
			logLevel = l;
		}
	}
	
	if (!(xbee = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(xbee, 0, memSize);
	if ((ret = xbee_frameBlockAlloc(&xbee->fBlock)) != XBEE_ENONE)         goto die1;
	if ((ret = xbee_logAlloc(&xbee->log, logLevel, stderr)) != XBEE_ENONE) goto die1;
	if ((ret = xbee_txAlloc(&xbee->iface.tx)) != XBEE_ENONE)                     goto die1;
	if ((ret = xbee_rxAlloc(&xbee->iface.rx)) != XBEE_ENONE)                     goto die1;
	
	if ((ret = ll_add_tail(xbeeList, xbee)) != XBEE_ENONE)                 goto die1;
	
	*nXbee = xbee;
	
	return XBEE_ENONE;
	
die1:
	xbee_free(xbee);
	return ret;
}

xbee_err xbee_free(struct xbee *xbee) {
	ll_ext_item(xbeeList, xbee);
	
	if (xbee->netInfo) xbee_netStop(xbee);
	
	xbee_threadDestroyMine(xbee);
	
	if (xbee->mode && xbee->mode->shutdown) xbee->mode->shutdown(xbee);
	
	xbee_modeCleanup(xbee->iface.conTypes);
	xbee_rxFree(xbee->iface.rx);
	xbee_txFree(xbee->iface.tx);
	xbee_logFree(xbee->log);
	xbee_frameBlockFree(xbee->fBlock);
	
	free(xbee);
	
	return XBEE_ENONE;
}

/* ######################################################################### */

EXPORT xbee_err xbee_setup(struct xbee **retXbee, char *mode, ...) {
	xbee_err ret;
	const struct xbee_mode *xbeeMode;
	struct xbee *xbee;
	va_list ap;
	
	if (!retXbee || !mode) return XBEE_EMISSINGPARAM;
	
	if ((ret = xbee_modeRetrieve(mode, &xbeeMode)) != XBEE_ENONE) return ret;
	
	if ((ret = xbee_alloc(&xbee)) != XBEE_ENONE) return ret;
	
	if ((ret = xbee_modeImport(&xbee->iface.conTypes, xbeeMode)) != XBEE_ENONE) goto die;
	xbee->mode = xbeeMode;
	
	xbee->iface.rx->ioFunc = xbee->mode->rx_io;
	xbee->iface.rx->fBlock = xbee->fBlock;
	xbee->iface.rx->conTypes = &xbee->iface.conTypes;
	
	xbee->iface.tx->ioFunc = xbee->mode->tx_io;
	
	va_start(ap, mode);
	if ((ret = xbee->mode->init(xbee, ap)) != XBEE_ENONE) goto die;
	va_end(ap);
	
	if ((ret = xbee_threadStart(xbee, NULL, 150000, 0, xbee_rx, xbee->iface.rx)) != XBEE_ENONE)                                goto die;
	if ((ret = xbee_threadStart(xbee, NULL, 150000, 0, xbee_rxHandler, xbee->iface.rx)) != XBEE_ENONE)                         goto die;
	if ((ret = xbee_threadStart(xbee, NULL, 150000, 0, xbee_tx, xbee->iface.tx)) != XBEE_ENONE)                                goto die;
	
	if (xbee->mode->prepare) if ((ret = xbee->mode->prepare(xbee)) != XBEE_ENONE)                                           goto die;
	
	if (xbee->mode->thread) if ((ret = xbee_threadStart(xbee, NULL, 150000, 0, xbee->mode->thread, NULL)) != XBEE_ENONE)       goto die;
	
	ll_add_tail(xbeeList, xbee);
	
	*retXbee = xbee;
	
	return XBEE_ENONE;

die:
	xbee_free(xbee);
	return ret;
}

xbee_err xbee_shutdownThread(struct xbee *xbee, int *restart, void *arg) {
	/* release / detach the thread that called shutdown(), dont care on failure */
	xbee_threadRelease(xbee, (xsys_thread)arg);
	xbee_free(xbee);
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_shutdown(struct xbee *xbee) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */

	/* pluck out the instance - from now on it is invalid */
	ll_ext_item(xbeeList, xbee);
	/* start a detached thread (indicated by -2) */
	xbee_threadStart(xbee, NULL, -1, 1, xbee_shutdownThread, (void*)(xsys_thread_self()));
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_attachEOFCallback(struct xbee *xbee, void (*eofCallback)(struct xbee *xbee, void *rxInfo)) {
      if (!xbee || !eofCallback) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
      if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
      if (xbee->iface.rx->eofCallback) return XBEE_EINUSE;
      xbee->iface.rx->eofCallback = (void(*)(struct xbee *xbee, struct xbee_rxInfo *rxInfo))eofCallback;
      return XBEE_ENONE;
}
