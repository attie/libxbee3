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
#include "rx.h"
#include "xbee_int.h"
#include "log.h"
#include "ll.h"

xbee_err xbee_rxAlloc(struct xbee_rxInfo **nInfo) {
	size_t memSize;
	struct xbee_rxInfo *info;
	
	if (!nInfo) return XBEE_EMISSINGPARAM;
	
	memSize = sizeof(*info);
	
	if (!(info = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(info, 0, memSize);
	info->bufList = ll_alloc();
	xsys_sem_init(&info->sem);
	
	*nInfo = info;
	
	return XBEE_ENONE;
}

xbee_err xbee_rxFree(struct xbee_rxInfo *info) {
	if (!info) return XBEE_EMISSINGPARAM;
	
	ll_free(info->bufList, (void(*)(void*))xbee_pktFree);
	xsys_sem_destroy(&info->sem);
	free(info);
	
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_rx(struct xbee *xbee, int *restart, void *arg) {
	xbee_err ret;
	struct xbee_rxInfo *info;
	struct xbee_buf *buf;
	xbee_err (*rx)(struct xbee *xbee, struct xbee_buf **buf);
	
	info = xbee->rx;
	rx = arg;
	
	while (!xbee->die) {
		if ((ret = rx(xbee, &buf)) != XBEE_ENONE) {
			xbee_log(1, "rx() returned %d... retrying in 10 ms", ret);
			usleep(10000); /* 10 ms */
			continue;
		}
		
		ll_add_tail(info->bufList, buf);
		buf = NULL;
		xsys_sem_post(&info->sem);
	}
	
	return XBEE_ENONE;
}
