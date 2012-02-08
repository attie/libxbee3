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

struct xbee_threadInfo;

#define xbee_threadStart(xbee, retThread, restartDelay, func, arg) \
	_xbee_threadStart(xbee, retThread, restartDelay, #func, func, arg)
xbee_err _xbee_threadStart(struct xbee *xbee, xsys_thread *retThread, int restartDelay, const char *funcName, xbee_err (*func)(struct xbee *xbee, int *restart, void *arg), void *arg);
xbee_err xbee_threadKill(struct xbee *xbee, xsys_thread thread);
xbee_err xbee_threadJoin(struct xbee *xbee, xsys_thread thread, xbee_err *retVal);
xbee_err xbee_threadKillJoin(struct xbee *xbee, xsys_thread thread, xbee_err *retVal);
void xbee_threadDestroy(struct xbee_threadInfo *info);

xbee_err xbee_threadDestroyMine(struct xbee *xbee);

#endif /* __XBEE_THREAD_H */
