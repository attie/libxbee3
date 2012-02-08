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
#include "ll.h"

struct ll_head *xbeeList = NULL;

EXPORT xbee_err xbee_validate(struct xbee *xbee) {
	if (ll_get_item(xbeeList, xbee) != XBEE_ENONE) return XBEE_EINVAL;
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_alloc(struct xbee **nXbee) {
	size_t memSize;
	struct xbee *xbee;
	xbee_err ret;
	
	if (!nXbee) return XBEE_EMISSINGPARAM;

	memSize = sizeof(*xbee);
	
	if (!(xbee = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(xbee, 0, memSize);
	xbee->conList = ll_alloc();
	if ((ret = xbee_frameBlockAlloc(&xbee->fBlock)) != XBEE_ENONE) goto die1;
	if ((ret = xbee_logAlloc(&xbee->log, 0, stderr)) != XBEE_ENONE) goto die1;
	
	if ((ret = ll_add_tail(xbeeList, xbee)) != XBEE_ENONE) goto die1;
	
	*nXbee = xbee;
	
	return XBEE_ENONE;
	
die1:
	xbee_free(xbee);
	return ret;
}

xbee_err xbee_free(struct xbee *xbee) {
	ll_ext_item(xbeeList, xbee);
	
	xbee_threadDestroyMine(xbee);
	
	xbee_logFree(xbee->log);
	xbee_frameBlockFree(xbee->fBlock);
	ll_free(xbee->conList, (void(*)(void*))xbee_conFree);
	
	if (xbee->mode && xbee->mode->shutdown) xbee->mode->shutdown(xbee);
	
	free(xbee);
	
	return XBEE_ENONE;
}

/* ######################################################################### */

EXPORT xbee_err xbee_setup(struct xbee **ret_xbee, char *mode, ...) {
	xbee_err ret;
	const struct xbee_mode *xbeeMode;
	struct xbee *xbee;
	va_list ap;
	
	if (!ret_xbee || !mode) return XBEE_EMISSINGPARAM;
	
	if ((ret = xbee_modeRetrieve(mode, &xbeeMode)) != XBEE_ENONE) return ret;
	if (!xbeeMode->init) return XBEE_EINVAL;
	if (!xbeeMode->rx) return XBEE_EINVAL;
	if (!xbeeMode->tx) return XBEE_EINVAL;
	
	if ((ret = xbee_alloc(&xbee)) != XBEE_ENONE) return ret;
	
	xbee->mode = xbeeMode;
	
	va_start(ap, mode);
	xbee->mode->init(xbee, ap);
	va_end(ap);
	
	if ((ret = xbee_threadStart(xbee, NULL, 150000, xbee_rx, xbee->mode->rx)) != XBEE_ENONE) goto die;
	if ((ret = xbee_threadStart(xbee, NULL, 150000, xbee_tx, xbee->mode->tx)) != XBEE_ENONE) goto die;
	if (xbee->mode->thread) if ((ret = xbee_threadStart(xbee, NULL, 150000, xbee->mode->thread, NULL)) != XBEE_ENONE) goto die;
	
	ll_add_tail(xbeeList, xbee);
	
	return XBEE_ENONE;

die:
	xbee_free(xbee);
	return ret;
}

EXPORT xbee_err xbee_shutdown(struct xbee *xbee) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */

	xbee_free(xbee);

	return XBEE_ENONE;
}
