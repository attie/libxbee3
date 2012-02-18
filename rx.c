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
#include "frame.h"
#include "conn.h"
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
			xbee_log(1, "rx() returned %d (%s)... retrying in 10 ms", ret, xbee_errorToStr(ret));
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
	
	struct xbee_frameInfo frameInfo;
	struct xbee_conAddress address;
	struct xbee_pkt *pkt;
	
	struct xbee_con *con;
	
	struct xbee_frameBlock *fBlock;
	
	ret = XBEE_ENONE;
	fBlock = arg;
	buf = NULL;
	
	while (!xbee->die) {
		xsys_sem_wait(&xbee->rx->sem);
		
		/* get the next buffer */
		if (ll_ext_head(xbee->rx->bufList, (void**)&buf) != XBEE_ENONE) return XBEE_ELINKEDLIST;
		if (!buf) continue;
		
		/* check we actually have some data to work with... */
		if (buf->len < 1) goto done;
		
		/* locate the connection type of this buffer */
		if ((ret = xbee_modeLocateConType(xbee->conTypes, NULL, &buf->data[0], NULL, &conType)) == XBEE_ENOTEXISTS || !conType) {
			xbee_log(4, "Unknown message type recieved... (0x%02X)", buf->data[0]);
			goto done;
		} else if (ret != XBEE_ENONE) {
			/* some other error occured */
			break;
		}
		
		/* prepare the buckets */
		memset(&frameInfo, 0, sizeof(frameInfo));
		memset(&address, 0, sizeof(address));
		pkt = NULL;
		
		/* process the buffer into the buckets */
		if ((ret = conType->rxHandler->func(xbee, conType->rxHandler->identifier, buf, &frameInfo, &address, &pkt)) != XBEE_ENONE) break;
		
		/* handle any frame info (prod someone who may be waiting for ACK/NAK/etc...) */
		if (frameInfo.active != 0) {
			xbee_log(20, "received Tx status (block: %p, frame: 0x%02X, status: 0x%02X)", fBlock, frameInfo.id, frameInfo.retVal);
			if ((ret = xbee_framePost(fBlock, frameInfo.id, frameInfo.retVal)) != XBEE_ENONE) {
				xbee_log(2, "failed to respond to frame (block: %p, frame: 0x%02X)... xbee_framePost() returned %d", fBlock, frameInfo.id, ret);
				ret = XBEE_ENONE;
			}
		}
		
		/* its possible that the buffer ONLY contained frame information... if so, were done! */
		if (!pkt) goto done;
		
		/* match the address to a connection */
		if ((ret = xbee_conMatchAddress(conType->conList, &address, &con, CON_SNOOZE)) != XBEE_ENONE || !con) {
			if (ret == XBEE_ENOTEXISTS) {
				xbee_log(5, "connectionless packet (%d bytes)...", buf->len);
				xbee_conLogAddress(xbee, 10, &address);
				goto done;
			}
			xbee_log(1, "xbee_conMatchAddress() returned %d...", ret);
			break;
		}
		
		/* wake the connection if necessary */
		if (con->sleepState != CON_AWAKE) {
			con->sleepState = CON_AWAKE;
			xbee_log(1, "woke connection @ %p", con);
		}
		
		/* add the packet to the connection's tail! */
		if ((ret = xbee_conLinkPacket(con, pkt)) != XBEE_ENONE) break;
		
done:
		ll_ext_item(needsFree, buf);
		free(buf);
		buf = NULL;
	}
	
	if (buf) {
		ll_ext_item(needsFree, buf);
		free(buf);
	}
	
	return ret;
}
