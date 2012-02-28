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
xsys_thread_key threadInfoKey;

xbee_err _xbee_threadGetInfo(struct xbee *xbee, xsys_thread thread, struct xbee_threadInfo **retInfo, int needsLLLock);
xbee_err _xbee_threadGetState(struct xbee *xbee, xsys_thread thread, int *running, int *active, int needsLLLock);
xbee_err _xbee_threadDestroy(struct xbee_threadInfo *info, int needsLLLock);

/* ########################################################################## */

void *threadFunc(struct xbee_threadInfo *info) {
	int restart; /* FALSE allows the thread to request that it is not restarted */
	struct xbee *xbee;
	xbee_err ret;
	
	xbee = info->xbee;
	info->active = 1;
	
	/* setup the thread info */
	xsys_thread_key_set(threadInfoKey, info);
	
	if (info->detached) {
		xsys_thread_detach_self();
	}
	
	if (info->restartDelay < 0) {
		/* a restartDelay of < 0 indicates that the thread should not restart (by default, the thread can request that it is restarted, in which case -(info->restartDelay) is used as the delay) */
		restart = 0;
		info->restartDelay = -info->restartDelay;
	} else {
		restart = 1;
	}
	
	do {
		xbee_log(15, "starting thread %p, function %s()...", info->thread, info->funcName);
	
		xsys_thread_unlock();
		info->running = 1;
		ret = info->func(info->xbee, &restart, info->arg);
		info->running = 0;
		xsys_thread_lock();

		if (ret != XBEE_ENONE) {
			xbee_log(1, "thread %p, function %s() returned %d...", info->thread, info->funcName, ret);
		} else {
			xbee_log(10, "thread %p, function %s() returned without error...", info->thread, info->funcName, ret);
		}
		if (!restart || !info->run) break;
		if (info->restartDelay != 0) {
			xbee_log(20, "restarting thread %p, function %s() in %d us...", info->thread, info->funcName, info->restartDelay);
			usleep(info->restartDelay);
		} else {
			xbee_log(20, "restarting thread %p, function %s() with zero delay...", info->thread, info->funcName);
		}
	} while (info->run);
	
	info->active = 0;
	
	xbee_log(15, "thread %p, function %s() has now ended...", info->thread, info->funcName);
	
	if (info->detached) free(info);
	
	return (void*)ret;
}

/* ########################################################################## */

xbee_err _xbee_threadStart(struct xbee *xbee, xsys_thread *retThread, int restartDelay, int detach, const char *funcName, xbee_err (*func)(struct xbee *xbee, int *restart, void *arg), void *arg) {
	struct xbee_threadInfo *info;

	if (!xbee || !func) return XBEE_EMISSINGPARAM;

	if ((info = malloc(sizeof(*info))) == NULL) return XBEE_ENOMEM;
	memset(info, 0, sizeof(*info));

	info->xbee = xbee;
	info->funcName = funcName;
	info->func = func;
	info->arg = arg;
	info->run = 1;
	info->detached = detach;
	info->restartDelay = restartDelay;
	xsys_sem_init(&info->mutexSem);

	if ((xsys_thread_create(&info->thread, (void*(*)(void *))threadFunc, info)) != 0) {
		free(info);
		return XBEE_ETHREAD;
	}

	if (!detach) {
		ll_add_tail(threadList, info);
	}
	if (retThread) *retThread = info->thread;

	return XBEE_ENONE;
}

xbee_err _xbee_threadGetInfo(struct xbee *xbee, xsys_thread thread, struct xbee_threadInfo **retInfo, int needsLLLock) {
	struct xbee_threadInfo *info;

	if (!retInfo) return XBEE_EMISSINGPARAM;

	if (needsLLLock) ll_lock(threadList);
	for (info = NULL; _ll_get_next(threadList, info, (void**)&info, 0) == XBEE_ENONE; ) {
		if (xbee && info->xbee != xbee) continue;
		if (info->thread != thread) continue;
		break;
	}
	if (needsLLLock) ll_unlock(threadList);

	if (!info) return XBEE_ENOTEXISTS;

	*retInfo = info;
	return XBEE_ENONE;
}
xbee_err xbee_threadGetInfo(struct xbee *xbee, xsys_thread thread, struct xbee_threadInfo **retInfo) {
	return _xbee_threadGetInfo(xbee, thread, retInfo, 1);
}

xbee_err _xbee_threadGetState(struct xbee *xbee, xsys_thread thread, int *running, int *active, int needsLLLock) {
	xbee_err ret;
	struct xbee_threadInfo *info;

	if (!running && !active) return XBEE_EMISSINGPARAM;

	if ((ret = _xbee_threadGetInfo(xbee, thread, &info, needsLLLock)) != XBEE_ENONE) return ret;

	if (running) *running = info->running;
	if (active)  *active  = info->active;

	return XBEE_ENONE;
}
xbee_err xbee_threadGetState(struct xbee *xbee, xsys_thread thread, int *running, int *active) {
	return _xbee_threadGetState(xbee, thread, running, active, 1);
}

xbee_err xbee_threadKill(struct xbee *xbee, xsys_thread thread) {
	xbee_err ret;
	struct xbee_threadInfo *info;
	int active;

	if (!xbee) return XBEE_EMISSINGPARAM;

	if ((ret = xbee_threadGetInfo(xbee, thread, &info)) != XBEE_ENONE) return XBEE_EINVAL;

	if ((ret = xbee_threadGetState(xbee, thread, NULL, &active)) != XBEE_ENONE) return ret;
	if (active) {
		info->run = 0;
		usleep(1000); /* 1ms */
		if (xsys_thread_cancel(thread)) return XBEE_ETHREAD;
	}

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

	return xbee_threadDestroy(info);
}

xbee_err _xbee_threadDestroy(struct xbee_threadInfo *info, int needsLLLock) {
	int active;
	xbee_err ret;

	if (!info) return XBEE_EMISSINGPARAM;

	if ((ret = _xbee_threadGetState(NULL, info->thread, NULL, &active, needsLLLock)) != XBEE_ENONE) return ret;
	if (active) {
		info->run = 0;
		usleep(1000); /* 1ms */
		if (xsys_thread_cancel(info->thread)) return XBEE_ETHREAD;
	}

	if (xsys_thread_join(info->thread, NULL)) return XBEE_ETHREAD;

	_ll_ext_item(threadList, info, needsLLLock);
	free(info);

	return XBEE_ENONE;
}
xbee_err xbee_threadDestroy(struct xbee_threadInfo *info) {
	return _xbee_threadDestroy(info, 1);
}

xbee_err xbee_threadDestroyMine(struct xbee *xbee) {
	xbee_err ret;
	struct xbee_threadInfo *info;
	struct xbee_threadInfo *pInfo;

	if (!xbee) return XBEE_EMISSINGPARAM;

	pInfo = NULL;
	ret = XBEE_ENONE;
	ll_lock(threadList);
	for (info = NULL; _ll_get_next(threadList, info, (void**)&info, 0) == XBEE_ENONE && info; ) {
		if (info->xbee != xbee) {
			pInfo = info;
			continue;
		}

		if ((ret = _xbee_threadDestroy(info, 0)) != XBEE_ENONE) {
			xbee_log(1, "failed to destroy thread %p...", info->thread);
			continue;
		}

		info = pInfo;
	}
	ll_unlock(threadList);
	
	return ret;
}

xbee_err xbee_threadRelease(struct xbee *xbee, xsys_thread thread) {
	xbee_err ret;
	struct xbee_threadInfo *info;

	if (!xbee) return XBEE_EMISSINGPARAM;

	if ((ret = xbee_threadGetInfo(xbee, thread, &info)) != XBEE_ENONE) return XBEE_EINVAL;
	
	xsys_thread_detach(info->thread);
	info->detached = 1;
	ll_ext_item(threadList, info);
	
	return XBEE_ENONE;
}

xbee_err xbee_threadStopRelease(struct xbee *xbee, xsys_thread thread) {
	xbee_err ret;
	struct xbee_threadInfo *info;

	if (!xbee) return XBEE_EMISSINGPARAM;

	if ((ret = xbee_threadGetInfo(xbee, thread, &info)) != XBEE_ENONE) return XBEE_EINVAL;
	
	xsys_thread_detach(info->thread);
	info->detached = 1;
	info->run = 0;
	ll_ext_item(threadList, info);
	
	return XBEE_ENONE;
}

xbee_err xbee_threadKillRelease(struct xbee *xbee, xsys_thread thread) {
	xbee_err ret;
	struct xbee_threadInfo *info;

	if (!xbee) return XBEE_EMISSINGPARAM;

	if ((ret = xbee_threadGetInfo(xbee, thread, &info)) != XBEE_ENONE) return XBEE_EINVAL;
	
	xsys_thread_detach(info->thread);
	xsys_thread_cancel(info->thread);
	
	ll_ext_item(threadList, info);
	
	free(info);
	
	return XBEE_ENONE;
}
