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
#include "mode.h"
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
	struct xbee_buf *buf;
	xbee_err (*rx)(struct xbee *xbee, struct xbee_buf **buf);
	
	rx = arg;
	
	while (!xbee->die) {
		if ((ret = rx(xbee, &buf)) != XBEE_ENONE) {
			xbee_log(1, "rx() returned %d... retrying in 10 ms", ret);
			usleep(10000); /* 10 ms */
			continue;
		}
		
		if (ll_add_tail(xbee->rx->bufList, buf) != XBEE_ENONE) return XBEE_ELINKEDLIST;
		buf = NULL;
		if (xsys_sem_post(&xbee->rx->sem) != 0) return XBEE_ESEMAPHORE;
	}
	
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_rxHandler(struct xbee *xbee, int *restart, void *arg) {
	xbee_err ret;
	struct xbee_buf *buf;
	struct xbee_modeConType *conType;
	struct xbee_conAddress address;
	struct xbee_pkt *pkt;
	
	ret = XBEE_ENONE;
	buf = NULL;
	
	while (!xbee->die) {
		xsys_sem_wait(&xbee->rx->sem);
		
		if (ll_ext_head(xbee->rx->bufList, (void**)&buf) != XBEE_ENONE) return XBEE_ELINKEDLIST;
		if (!buf) continue;
		
		if (buf->len < 1) goto done;
		
		if ((ret - xbee_modeLocateConType(xbee->mode, NULL, &buf->data[0], NULL, &conType)) == XBEE_ENOTEXISTS) goto done;
		if (ret != XBEE_ENONE) break;
		
		if ((ret = conType->rxHandler->func(xbee, buf, &address, &pkt)) != XBEE_ENONE) break;
		
#warning TODO - match connection & add packet to list
		
done:
		free(buf);
		buf = NULL;
	}
	
	if (buf) free(buf);
	
	return ret;
}
