#ifndef __XBEE_LOG_H
#define __XBEE_LOG_H

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

#ifdef XBEE_DISABLE_LOGGING

#define xbee_log(...)

/* ######################################################################### */
#else /* XBEE_DISABLE_LOGGING */
/* ######################################################################### */

#define XBEE_LOG_MAXLEN    1024
#define XBEE_LOG_TRUNC_STR "].."

struct xbee_log {
	xsys_mutex mutex;
	FILE *f;

	unsigned int logLevel;

#ifndef XBEE_LOG_NO_RX
	unsigned int enable_rx    : 1;
#endif
#ifndef XBEE_LOG_NO_TX
	unsigned int enable_tx    : 1;
#endif
#ifndef XBEE_LOG_NO_COLOR
	unsigned int enable_color : 1;
	unsigned int f_isatty     : 1;
	unsigned int use_color    : 1;
#endif
};

xbee_err xbee_logAlloc(struct xbee_log **nLog);
xbee_err xbee_logFree(struct xbee_log *log);

#define xbee_logTest(curLevel, minLevel)  ((curLevel >= minLevel) ? 0 : 1)

xbee_err _xbee_log(const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *format, ...);
#define xbee_log(...) _xbee_log(__FILE__, __LINE__, __FUNCTION__, xbee, __VA_ARGS__)

xbee_err _xbee_logData(const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *label, unsigned char *data, size_t length);
#define xbee_logData(minLevel, label, data, length) _xbee_logData(__FILE__, __LINE__, __FUNCTION__, xbee, minLevel, label, data, length)

#endif /* XBEE_DISABLE_LOGGING */
#endif /* __XBEE_LOG_H */
