/*
	libxbee - a C/C++ library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with libxbee. If not, see <http://www.gnu.org/licenses/>.
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
EXPORT xbee_err xbee_logTargetSet(struct xbee *xbee, FILE *f) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logTargetGet(struct xbee *xbee, FILE **f) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logLevelSet(struct xbee *xbee, int level) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logLevelGet(struct xbee *xbee, int *level) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logRxSet(struct xbee *xbee, int enable) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logRxGet(struct xbee *xbee, int *enabled) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logTxSet(struct xbee *xbee, int enable) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logTxGet(struct xbee *xbee, int *enabled) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logColorSet(struct xbee *xbee, int enable) {
	return XBEE_ENOTIMPLEMENTED;
}
EXPORT xbee_err xbee_logColorGet(struct xbee *xbee, int *enabled) {
	return XBEE_ENOTIMPLEMENTED;
}

/* ######################################################################### */
#else /* XBEE_DISABLE_LOGGING */
/* ######################################################################### */

xbee_err xbee_logAlloc(struct xbee_log **nLog) {
	size_t memSize;
	struct xbee_log *log;
	const char *e;
	
	if (!nLog) return XBEE_EMISSINGPARAM;
	
	memSize = sizeof(*log);
	
	if (!(log = malloc(memSize))) return XBEE_ENOMEM;
	
	memset(log, 0, memSize);

	xsys_mutex_init(&log->mutex);
	log->f = stderr;

	if ((e = getenv("XBEE_LOG_LEVEL")) != NULL) {
		int l;
		if (sscanf(e, "%d", &l) != 1) {
			fprintf(stderr, "libxbee: Failed to initialize log level from environment (not a number)\n");
			log->logLevel = 0;
		} else {
			log->logLevel = l;
		}
	} else {
#ifdef XBEE_LOG_LEVEL
		log->logLevel = XBEE_LOG_LEVEL;
#else
		log->logLevel = 0;
#endif
	}

#ifndef XBEE_LOG_NO_RX
	if ((e = getenv("XBEE_LOG_RX")) != NULL) {
		int l;
		if (sscanf(e, "%d", &l) != 1) {
			fprintf(stderr, "libxbee: Failed to initialize Rx logging from environment (not a number)\n");
			log->enable_rx = 0;
		} else {
			log->enable_rx = !!l;
		}
	} else {
#ifdef XBEE_LOG_RX_DEFAULT_OFF
		log->enable_rx = 0;
#else
		log->enable_rx = 1;
#endif
	}
#endif

#ifndef XBEE_LOG_NO_TX
	if ((e = getenv("XBEE_LOG_TX")) != NULL) {
		int l;
		if (sscanf(e, "%d", &l) != 1) {
			fprintf(stderr, "libxbee: Failed to initialize Tx logging from environment (not a number)\n");
			log->enable_tx = 0;
		} else {
			log->enable_tx = !!l;
		}
	} else {
#ifdef XBEE_LOG_TX_DEFAULT_OFF
		log->enable_tx = 0;
#else
		log->enable_tx = 1;
#endif
	}
#endif

#ifndef XBEE_LOG_NO_COLOR
	if ((e = getenv("XBEE_LOG_COLOR")) != NULL) {
		int l;
		if (sscanf(e, "%d", &l) != 1) {
			fprintf(stderr, "libxbee: Failed to initialize logging colorization from environment (not a number)\n");
			log->enable_color = 1;
		} else {
			log->enable_color = !!l;
		}
	} else {
		log->enable_color = 1;
	}
	log->f_isatty = !!isatty(fileno(log->f));
	log->use_color = !!(log->enable_color && log->f_isatty);
#endif
	
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

EXPORT xbee_err xbee_logTargetSet(struct xbee *xbee, FILE *f) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	xbee_mutex_lock(&xbee->log->mutex);
	xbee->log->f = f;
#ifndef XBEE_LOG_NO_COLOR
	xbee->log->f_isatty = !!isatty(fileno(xbee->log->f));
	xbee->log->use_color = !!(xbee->log->enable_color && xbee->log->f_isatty);
#endif
	xbee_mutex_unlock(&xbee->log->mutex);
	xbee_log(xbee->log->logLevel, "Set log target to: %p (fd:%d)", f, xsys_fileno(f));
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_logTargetGet(struct xbee *xbee, FILE **f) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	*f = xbee->log->f;
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_logLevelSet(struct xbee *xbee, int level) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	xbee_mutex_lock(&xbee->log->mutex);
	xbee->log->logLevel = level;
	xbee_mutex_unlock(&xbee->log->mutex);
	xbee_log(xbee->log->logLevel, "Set log level to: %d", level);
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_logLevelGet(struct xbee *xbee, int *level) {
	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;
	
	*level = xbee->log->logLevel;
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_logRxSet(struct xbee *xbee, int enable) {
#ifdef XBEE_LOG_NO_RX
	return XBEE_ENOTIMPLEMENTED;
#else

	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;

	xbee_mutex_lock(&xbee->log->mutex);
	xbee->log->enable_rx = !!enable;
	xbee_mutex_unlock(&xbee->log->mutex);
	xbee_log(xbee->log->logLevel, "Set logging of Rx data to: %s", (enable?"Enabled":"Disabled"));

	return XBEE_ENONE;
#endif /* XBEE_LOG_NO_RX */
}

EXPORT xbee_err xbee_logRxGet(struct xbee *xbee, int *enabled) {
#ifdef XBEE_LOG_NO_RX
	return XBEE_ENOTIMPLEMENTED;
#else

	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;

	xbee_mutex_lock(&xbee->log->mutex);
	*enabled = !!xbee->log->enable_rx;
	xbee_mutex_unlock(&xbee->log->mutex);

	return XBEE_ENONE;
#endif /* XBEE_LOG_NO_RX */
}

EXPORT xbee_err xbee_logTxSet(struct xbee *xbee, int enable) {
#ifdef XBEE_LOG_NO_TX
	return XBEE_ENOTIMPLEMENTED;
#else

	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;

	xbee_mutex_lock(&xbee->log->mutex);
	xbee->log->enable_tx = !!enable;
	xbee_mutex_unlock(&xbee->log->mutex);
	xbee_log(xbee->log->logLevel, "Set logging of Tx data to: %s", (enable?"Enabled":"Disabled"));

	return XBEE_ENONE;
#endif /* XBEE_LOG_NO_TX */
}

EXPORT xbee_err xbee_logTxGet(struct xbee *xbee, int *enabled) {
#ifdef XBEE_LOG_NO_TX
	return XBEE_ENOTIMPLEMENTED;
#else

	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;

	xbee_mutex_lock(&xbee->log->mutex);
	*enabled = !!xbee->log->enable_tx;
	xbee_mutex_unlock(&xbee->log->mutex);

	return XBEE_ENONE;
#endif /* XBEE_LOG_NO_TX */
}

EXPORT xbee_err xbee_logColorSet(struct xbee *xbee, int enable) {
#ifdef XBEE_LOG_NO_COLOR
	return XBEE_ENOTIMPLEMENTED;
#else

	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;

	xbee_mutex_lock(&xbee->log->mutex);
	xbee->log->enable_color = !!enable;
	xbee->log->use_color = xbee->log->enable_color & xbee->log->f_isatty;
	xbee_mutex_unlock(&xbee->log->mutex);
	xbee_log(xbee->log->logLevel, "Set colorized logging to: %s", (enable?"Enabled":"Disabled"));
	if (xbee->log->enable_color && !xbee->log->use_color) {
		xbee_log(xbee->log->logLevel, "Not using colorized logging... (is the target a TTY?)");
	}

	return XBEE_ENONE;
#endif /* XBEE_LOG_NO_COLOR */
}

EXPORT xbee_err xbee_logColorGet(struct xbee *xbee, int *enabled) {
#ifdef XBEE_LOG_NO_COLOR
	return XBEE_ENOTIMPLEMENTED;
#else

	if (!xbee) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	if (!xbee->log) return XBEE_ENOTIMPLEMENTED;

	xbee_mutex_lock(&xbee->log->mutex);
	*enabled = !!xbee->log->enable_color;
	xbee_mutex_unlock(&xbee->log->mutex);

	return XBEE_ENONE;
#endif /* XBEE_LOG_NO_COLOR */
}

/* ######################################################################### */

xbee_err _xbee_logWrite(struct xbee_log *log, const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *preStr, char *format, va_list ap) {
	char tBuf[XBEE_LOG_MAXLEN];
	int len;
	const char * const truncStr = XBEE_LOG_TRUNC_STR;
	static int truncLen = 0;
	
	if (!log || !file || !function || !xbee || !preStr || !format) return XBEE_EMISSINGPARAM;
	if (!log->f) return XBEE_EINVAL;
	
	len = vsnprintf(tBuf, XBEE_LOG_MAXLEN, format, ap);
	
	if (len >= XBEE_LOG_MAXLEN) {
		if (truncLen == 0) {
			truncLen = strlen(truncStr);
		}
		strcpy(&(tBuf[XBEE_LOG_MAXLEN - (truncLen + 1)]), truncStr);
	}
	
	xbee_mutex_lock(&log->mutex);
	
#ifndef XBEE_LOG_NO_COLOR
	if (log->use_color) {
		if (!xbee) {
			fprintf(log->f, "%s%c[36m%3d%c[90m#[%c[32m%s:%d%c[90m]%c[33m %s()%c[90m:%c[0m %s\n",
				preStr, 27, minLevel, 27, 27, file, line, 27, 27, function, 27, 27,                   tBuf);
		} else if (xbee_validate(xbee) == XBEE_ENONE) {
			fprintf(log->f, "%s%c[36m%3d%c[90m#[%c[32m%s:%d%c[90m]%c[33m %s()%c[0m %c[35m%p%c[90m:%c[0m %s\n",
				preStr, 27, minLevel, 27, 27, file, line, 27, 27, function, 27, 27, xbee, 27, 27,     tBuf);
		} else {
			fprintf(log->f, "%s%c[36m%3d%c[90m#[%c[32m%s:%d%c[90m]%c[33m %s()%c[31m !%c[35m%p%c[31m!%c[90m:%c[0m %s\n",
				preStr, 27, minLevel, 27, 27, file, line, 27, 27, function, 27, 27, xbee, 27, 27, 27, tBuf);
		}
	} else {
#endif /* !XBEE_LOG_NO_COLOR */
		if (!xbee) {
			fprintf(log->f, "%s%3d#[%s:%d] %s(): %s\n",      preStr, minLevel, file, line, function,       tBuf);
		} else if (xbee_validate(xbee) == XBEE_ENONE) {
			fprintf(log->f, "%s%3d#[%s:%d] %s() %p: %s\n",   preStr, minLevel, file, line, function, xbee, tBuf);
		} else {
			fprintf(log->f, "%s%3d#[%s:%d] %s() !%p!: %s\n", preStr, minLevel, file, line, function, xbee, tBuf);
		}
#ifndef XBEE_LOG_NO_COLOR
	}
#endif /* !XBEE_LOG_NO_COLOR */
	fflush(log->f);
	
	xbee_mutex_unlock(&log->mutex);
	
	return XBEE_ENONE;
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

xbee_err _xbee_logData(const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *label, unsigned char *data, size_t length) {
	int i;
	int l;
	/* format:
		0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00  | ........
	*/
	char lineBufA[41];
	char lineBufB[9];
	
	/* prepare the format string */
	for (l = 0; l < sizeof(lineBufA) - 1; l++) {
		switch (l % 5) {
			case 0: case 2: case 3:
				lineBufA[l] = '0'; break;
			case 1:
				lineBufA[l] = 'x'; break;
			case 4:
				lineBufA[l] = ' '; break;
		}
	}
	lineBufA[l] = '\0';
	lineBufB[sizeof(lineBufB) - 1] = '\0';
	
	xbee_log(25, "%s length: %d", label, length);
	
	for (i = 0; i < length; i += l) {
		/* fill in the data */
		for (l = 0; l < 8 && i + l < length; l++) {
			snprintf(&(lineBufA[(5 * l) + 2]), 3, "%02X", data[i + l]);
			lineBufA[(5 * l) + 4] = ' ';
			lineBufB[l] = ((data[i + l] >= ' ' && data[i + l] <= '~')?data[i + l]:'.');
		}
		/* wipe out the unneeded space */
		for (; l < 8; l++) {
			strncpy(&(lineBufA[5 * l]), "     ", 6);
			lineBufB[l] = ' ';
		}
		xbee_log(25, "%s: 0x%04X : %s | %s", label, i, lineBufA, lineBufB);
	}
	
	return XBEE_ENONE;
}

#endif /* XBEE_DISABLE_LOGGING */
