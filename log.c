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
#include "log.h"
#include "xbee_int.h"

#ifdef XBEE_DISABLE_LOGGING

EXPORT xbee_err _xbee_logDev(const char * const file, const int line, const char * const function, struct xbee * const xbee, const int minLevel, const char * const format, ...) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logSetTarget(struct xbee *xbee, FILE *f) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logGetTarget(struct xbee *xbee, FILE **f) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logSetLevel(struct xbee *xbee, int level) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logGetLevel(struct xbee *xbee, int *level) {
	return XBEE_ENOTIMPLEMENTED;
}

/* ######################################################################### */
#else /* XBEE_DISABLE_LOGGING */
/* ######################################################################### */

xbee_err xbee_logAlloc(struct xbee_log **nLog, int defLevel, FILE *defFile) {
	size_t memSize;
	struct xbee_log *log;
	
	if (!nLog) return XBEE_EMISSINGPARAM;
	
	memSize = sizeof(*log);
	
	if (!(log = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(log, 0, memSize);
	
	xsys_mutex_init(&log->mutex);
	log->logLevel = defLevel;
	log->f = defFile;
	
	*nLog = log;
	
	return XBEE_ENONE;
}

xbee_err xbee_logFree(struct xbee_log *log) {
	if (!log) return XBEE_EMISSINGPARAM;
	
	xsys_mutex_destroy(&log->mutex);
	free(log);
	
	return XBEE_ENONE;
}

/* ######################################################################### */

EXPORT xbee_err xbee_logSetTarget(struct xbee *xbee, FILE *f) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	xsys_mutex_lock(&xbee->log->mutex);
	xbee->log->f = f;
	xsys_mutex_unlock(&xbee->log->mutex);
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_logGetTarget(struct xbee *xbee, FILE **f) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	*f = xbee->log->f;
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_logSetLevel(struct xbee *xbee, int level) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	xsys_mutex_lock(&xbee->log->mutex);
	xbee->log->logLevel = level;
	xsys_mutex_unlock(&xbee->log->mutex);
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_logGetLevel(struct xbee *xbee, int *level) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	*level = xbee->log->logLevel;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err _xbee_logWrite(struct xbee_log *log, const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *preStr, char *format, va_list ap) {
	char tBuf[XBEE_LOG_MAXLEN];
	int len;
	const char * const truncStr = XBEE_LOG_TRUNC_STR;
	static int truncLen = 0;
	
	len = vsnprintf(tBuf, XBEE_LOG_MAXLEN, format, ap);
	
	if (len >= XBEE_LOG_MAXLEN) {
		if (truncLen == 0) {
			truncLen = strlen(truncStr);
		}
		strcpy(&(tBuf[XBEE_LOG_MAXLEN - (truncLen + 1)]), truncStr);
	}
	
	xsys_mutex_lock(&log->mutex);
	
	if (!xbee) {
		fprintf(log->f, "%s%3d#[%s:%d] %s(): %s\n",      preStr, minLevel, file, line, function,       tBuf);
	} else if (xbee_validate(xbee) != XBEE_ENONE) {
		fprintf(log->f, "%s%3d#[%s:%d] %s() %p: %s\n",   preStr, minLevel, file, line, function, xbee, tBuf);
	} else {
		fprintf(log->f, "%s%3d#[%s:%d] %s() !%p!: %s\n", preStr, minLevel, file, line, function, xbee, tBuf);
	}
	
	xsys_mutex_unlock(&log->mutex);
	
	return XBEE_ENOTIMPLEMENTED;
}


EXPORT xbee_err _xbee_logDev(const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *format, ...) {
	va_list ap;
	xbee_err ret;

	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	if (xbee_logTest(xbee->log->logLevel, minLevel)) return XBEE_ENONE;
	
	va_start(ap, format);
	ret = _xbee_logWrite(xbee->log, file, line, function, xbee, minLevel, "DEV:", format, ap);
	va_end(ap);
	
	return ret;
}

xbee_err _xbee_log(const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *format, ...) {
	va_list ap;
	xbee_err ret;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	if (xbee_logTest(xbee->log->logLevel, minLevel)) return XBEE_ENONE;
	
	va_start(ap, format);
	ret = _xbee_logWrite(xbee->log, file, line, function, xbee, minLevel, "", format, ap);
	va_end(ap);
	
	return ret;
}

#endif /* XBEE_DISABLE_LOGGING */
