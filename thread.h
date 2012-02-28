#ifndef __XBEE_THREAD_H
#define __XBEE_THREAD_H

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

extern struct ll_head *threadList;
extern xsys_thread_key threadInfoKey;

struct xbee_threadInfo {
	int run;     /* FALSE will cause the thread to die once func() returns */
	int detached;/* TRUE will cause the thread to free the info block before it returns */
	int running; /* TRUE means that the function is actually running */
	int active;  /* TRUE means that the thread is alive */

	time_t restartDelay;
	xsys_thread thread;

	xsys_sem mutexSem; /* keeps count of mutexes held, if > 0, then the thread should be locked */

	struct xbee *xbee;
	const char *funcName;
	xbee_err (*func)(struct xbee *xbee, int *restart, void *arg);
	void *arg;
};

#define xbee_threadStart(xbee, retThread, restartDelay, detach, func, arg) \
	_xbee_threadStart(xbee, retThread, restartDelay, detach, #func, func, arg)
xbee_err _xbee_threadStart(struct xbee *xbee, xsys_thread *retThread, int restartDelay, int detach, const char *funcName, xbee_err (*func)(struct xbee *xbee, int *restart, void *arg), void *arg);

xbee_err xbee_threadGetState(struct xbee *xbee, xsys_thread thread, int *running, int *active);

xbee_err xbee_threadKill(struct xbee *xbee, xsys_thread thread);
xbee_err xbee_threadJoin(struct xbee *xbee, xsys_thread thread, xbee_err *retVal);
xbee_err xbee_threadKillJoin(struct xbee *xbee, xsys_thread thread, xbee_err *retVal);
xbee_err xbee_threadKillRelease(struct xbee *xbee, xsys_thread thread);

xbee_err xbee_threadDestroy(struct xbee_threadInfo *info);
xbee_err xbee_threadDestroyMine(struct xbee *xbee);

xbee_err xbee_threadRelease(struct xbee *xbee, xsys_thread thread);
xbee_err xbee_threadStopRelease(struct xbee *xbee, xsys_thread thread);

#endif /* __XBEE_THREAD_H */
