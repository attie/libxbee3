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
#include "xbee_int.h"
#include "thread.h"
#include "log.h"
#include "ll.h"

struct ll_head *threadList = NULL;

struct xbee_threadInfo {
	int run;     /* FALSE will cause the thread to die once func() returns */
	int running; /* TRUE means that the function is actually running */
	int active;  /* TRUE means that the thread is alive */

	time_t restartDelay;
	xsys_thread thread;

	struct xbee *xbee;
	const char *funcName;
	xbee_err (*func)(struct xbee *xbee, int *restart, void *arg);
	void *arg;
};

/* ########################################################################## */

void *threadFunc(struct xbee_threadInfo *info) {
	int restart; /* FALSE allows the thread to request that it is not restarted */
	struct xbee *xbee;
	xbee_err ret;
	
	xbee = info->xbee;
	info->active = 1;
	restart = 1;
	
	do {
		xbee_log(15, "starting thread %p, function %s()...", info->thread, info->funcName);
	
		info->running = 1;
		ret = info->func(info->xbee, &restart, info->arg);
		info->running = 0;

		if (ret != XBEE_ENONE) {
			xbee_log(1, "thread %p, function %s() returned %d...", info->thread, info->funcName, ret);
		} else {
			xbee_log(10, "thread %p, function %s() returned without error...", info->thread, info->funcName, ret);
		}
		if (!restart || !info->run) break;
		if (info->restartDelay) {
			xbee_log(20, "restarting thread %p, function %s() in %d us...", info->thread, info->funcName, info->restartDelay);
			usleep(info->restartDelay);
		}
	} while (info->run);
	
	info->active = 0;
	
	xbee_log(15, "thread %p, function %s() has now ended...", info->thread, info->funcName);
	
	return (void*)ret;
}

/* ########################################################################## */

xbee_err _xbee_threadStart(struct xbee *xbee, xsys_thread *retThread, int restartDelay, const char *funcName, xbee_err (*func)(struct xbee *xbee, int *restart, void *arg), void *arg) {
	struct xbee_threadInfo *info;

	if (!xbee || !func) return XBEE_EMISSINGPARAM;

	if ((info = malloc(sizeof(*info))) == NULL) return XBEE_ENOMEM;
	memset(info, 0, sizeof(*info));

	info->xbee = xbee;
	info->funcName = funcName;
	info->func = func;
	info->arg = arg;
	info->run = 1;
	info->restartDelay = restartDelay;

	if ((xsys_thread_create(&info->thread, (void*(*)(void *))threadFunc, info)) != 0) {
		free(info);
		return XBEE_ETHREAD;
	}

	ll_add_tail(threadList, info);
	if (retThread) *retThread = info->thread;

	return XBEE_ENONE;
}

xbee_err xbee_threadGetInfo(struct xbee *xbee, xsys_thread thread, struct xbee_threadInfo **retInfo) {
	struct xbee_threadInfo *info;

	if (!xbee || !retInfo) return XBEE_EMISSINGPARAM;

	ll_lock(threadList);
	for (info = NULL; _ll_get_next(threadList, info, (void**)&info, 0) == XBEE_ENONE; ) {
		if (info->xbee != xbee) continue;
		if (info->thread != thread) continue;
		break;
	}
	ll_unlock(threadList);

	if (!info) return XBEE_EINVAL;

	*retInfo = info;
	return XBEE_ENONE;
}

xbee_err xbee_threadKill(struct xbee *xbee, xsys_thread thread) {
	xbee_err ret;
	struct xbee_threadInfo *info;

	if (!xbee) return XBEE_EMISSINGPARAM;

	if ((ret = xbee_threadGetInfo(xbee, thread, &info)) != XBEE_ENONE) return XBEE_EINVAL;

	if (xsys_thread_cancel(thread)) return XBEE_ETHREAD;

	return XBEE_ENONE;
}

xbee_err xbee_threadJoin(struct xbee *xbee, xsys_thread thread, xbee_err *retVal) {
	xbee_err ret;
	struct xbee_threadInfo *info;

	if (!xbee) return XBEE_EMISSINGPARAM;

	if ((ret = xbee_threadGetInfo(xbee, thread, &info)) != XBEE_ENONE) return XBEE_EINVAL;

	if (info->active != 0) return XBEE_EINUSE;

	if (xsys_thread_join(thread, (void**)retVal)) return XBEE_ETHREAD;

	ll_ext_item(threadList, info);
	free(info);

	return XBEE_ENONE;
}

xbee_err xbee_threadKillJoin(struct xbee *xbee, xsys_thread thread, xbee_err *retVal) {
	xbee_err ret;
	struct xbee_threadInfo *info;

	if (!xbee) return XBEE_EMISSINGPARAM;

	if ((ret = xbee_threadGetInfo(xbee, thread, &info)) != XBEE_ENONE) return XBEE_EINVAL;

	if (xsys_thread_cancel(thread)) return XBEE_ETHREAD;
	if (xsys_thread_join(thread, (void**)retVal)) return XBEE_ETHREAD;

	ll_ext_item(threadList, info);
	free(info);

	return XBEE_ENONE;
}

void xbee_threadDestroy(struct xbee_threadInfo *info) {
	if (!info) return;
	
	if (xsys_thread_cancel(info->thread)) return;
	if (xsys_thread_join(info->thread, NULL)) return;

	ll_ext_item(threadList, info);
	free(info);
}

xbee_err xbee_threadDestroyMine(struct xbee *xbee) {
	struct xbee_threadInfo *info;

	if (!xbee) return XBEE_EMISSINGPARAM;

	ll_lock(threadList);
	for (info = NULL; _ll_get_next(threadList, info, (void**)&info, 0) == XBEE_ENONE; ) {
		if (!info) break;
		if (info->xbee != xbee) continue;
	
		if (xsys_thread_cancel(info->thread)) return XBEE_ETHREAD;
		if (xsys_thread_join(info->thread, NULL)) return XBEE_ETHREAD;
		
		_ll_ext_item(threadList, info, 0);
		
		free(info);
	}
	ll_unlock(threadList);
	
	return XBEE_ENONE;
}
