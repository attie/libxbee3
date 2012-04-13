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

#include "internal.h"
#include "prepare.h"
#include "ll.h"
#include "xbee_int.h"
#include "conn.h"
#include "pkt.h"
#include "thread.h"
#include "net.h"

EXPORT INIT void xbee_init(void) {
	xsys_thread_key_init(&threadInfoKey, NULL);
	if (!xbeeList && (xbeeList = ll_alloc()) == NULL) {
		fprintf(stderr, "libxbee: failed to initialize xbeeList...\n");
	}
	if (!conList && (conList = ll_alloc()) == NULL) {
		fprintf(stderr, "libxbee: failed to initialize conList...\n");
	}
	if (!pktList && (pktList = ll_alloc()) == NULL) {
		fprintf(stderr, "libxbee: failed to initialize pktList...\n");
	}
	if (!netDeadClientList && (netDeadClientList = ll_alloc()) == NULL) {
		fprintf(stderr, "libxbee: failed to initialize netDeadClientList...\n");
	}
	if (!threadList && (threadList = ll_alloc()) == NULL) {
		fprintf(stderr, "libxbee: failed to initialize threadList...\n");
	}
	if (!needsFree && (needsFree = ll_alloc()) == NULL) {
		fprintf(stderr, "libxbee: failed to initialize needsFree...\n");
	}
}

EXPORT FINI void xbee_fini(void) {
#ifndef XBEE_NO_FINI
	/* clean up threads, so that they can't depend on anything we are about to free! */
	if (threadList) {
		ll_free(threadList, (void(*)(void*))xbee_threadKillThis);
		threadList = NULL;
	}
	if (xbeeList) {
		ll_free(xbeeList, (void(*)(void*))xbee_shutdown);
		xbeeList = NULL;
	}
	if (conList) {
		ll_free(conList, (void(*)(void*))xbee_conEnd);
		conList = NULL;
	}
	if (pktList) {
		ll_free(pktList, (void(*)(void*))xbee_pktFree);
		pktList = NULL;
	}
	if (netDeadClientList) {
		ll_free(netDeadClientList, (void(*)(void*))xbee_netClientShutdown);
		netDeadClientList = NULL;
	}
	if (needsFree) {
		ll_free(needsFree, (void(*)(void*))free);
		needsFree = NULL;
	}
#endif /* XBEE_NO_FINI */
}
