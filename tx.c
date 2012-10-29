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
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "tx.h"
#include "xbee_int.h"
#include "log.h"
#include "conn.h"
#include "mode.h"
#include "ll.h"

xbee_err xbee_txAlloc(struct xbee_txInfo **nInfo) {
	static char logColor = 1;
	size_t memSize;
	struct xbee_txInfo *info;
	
	if (!nInfo) return XBEE_EMISSINGPARAM;
	
	memSize = sizeof(*info);
	
	if (!(info = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(info, 0, memSize);
	info->bufList = xbee_ll_alloc();
	xsys_sem_init(&info->sem);
	
	/* give it a log color */
	info->logColor = logColor;
	if (logColor++ > 7) logColor = 7;
	
	*nInfo = info;
	
	return XBEE_ENONE;
}

xbee_err xbee_txFree(struct xbee_txInfo *info) {
	if (!info) return XBEE_EMISSINGPARAM;
	
	xbee_ll_free(info->bufList, (void(*)(void*))free);
	xsys_sem_destroy(&info->sem);
	free(info);
	
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_tx(struct xbee *xbee, int *restart, void *arg) {
	xbee_err ret;
	struct xbee_txInfo *info;
	struct xbee_buf *buf;
	
	info = arg;
	if (!info->ioFunc) {
		*restart = 0;
		return XBEE_EINVAL;
	}
	
	while (!xbee->die) {
		if (xsys_sem_wait(&info->sem) != 0) return XBEE_ESEMAPHORE;
		if ((ret = xbee_ll_ext_head(info->bufList, (void**)&buf)) != XBEE_ENONE && ret != XBEE_ERANGE) return XBEE_ELINKEDLIST;
		if (!buf) continue;
		
#ifdef XBEE_LOG_TX
		{
			/* format: tx[0x0000000000000000] */
			char label[42]; /* enough space for a 64-bit pointer and ANSI color codes */
			
#ifdef XBEE_LOG_NO_COLOR
			snprintf(label, sizeof(label), "tx[%p]", info);
#else
			snprintf(label, sizeof(label), "tx[%c[%dm%p%c[0m]", 27, 30 + info->logColor, info,  27);
#endif
			xbee_logData(25, label, buf->data, buf->len);
		}
#endif /* XBEE_LOG_TX */

		if ((ret = info->ioFunc(xbee, info->ioArg, buf)) != XBEE_ENONE) {
			xbee_log(1, "tx() returned %d... buffer was lost", ret);
			continue;
		}
		
		free(buf);
	}
	
	return XBEE_ESHUTDOWN;
}

/* ######################################################################### */

xbee_err xbee_txQueueBuffer(struct xbee_txInfo *info, struct xbee_buf *buf) {
	if (xbee_ll_add_tail(info->bufList, buf) != XBEE_ENONE) return XBEE_ELINKEDLIST;
	if (xsys_sem_post(&info->sem) != 0) return XBEE_ESEMAPHORE;
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_txHandler(struct xbee_con *con, const unsigned char *buf, int len) {
	xbee_err ret;
	struct xbee_buf *oBuf;
	
	if (!con) return XBEE_EMISSINGPARAM;
	if (!con->conType) return XBEE_EINVAL;
	if (!con->conType->txHandler || !con->conType->txHandler->func) return XBEE_ENOTIMPLEMENTED;
	
	oBuf = NULL;
	if ((ret = con->conType->txHandler->func(con->xbee, con, con->iface->tx->ioArg, con->conType->txHandler->identifier, con->frameId, &con->address, &con->settings, buf, len, &oBuf)) != XBEE_ENONE) return ret;
	
	if (!oBuf) return XBEE_EUNKNOWN;
	
	con->info.countTx++;
	
	return xbee_txQueueBuffer(con->iface->tx, oBuf);
}
