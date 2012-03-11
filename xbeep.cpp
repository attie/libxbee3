#include <iostream>

#include "xbee.h"
#include "xsys.h"

using namespace std;

struct xbee_con *xbeep::getCon(const unsigned int conId) {
	struct xbee_con *con;
	if (conId >= connections.size()) throw XBEE_EINVAL;
	con = NULL;
	try {
		con = connections[conId];
	} catch (...) {
		throw XBEE_EINVAL;
	}
	if (!con) throw XBEE_EINVAL;
	return con;
}

/* ########################################################################## */

EXPORT xbeep::xbeep(const char *mode, ...) {
	xbee_err ret;
	va_list ap;
	
	va_start(ap, mode);
	if ((ret = xbee_vsetup(&xbee, mode, ap)) != XBEE_ENONE) throw ret;
	va_end(ap);
	
	connections.begin();
}
EXPORT xbeep::xbeep(const char *mode, va_list ap) {
	xbee_err ret;
	
	if ((ret = xbee_vsetup(&xbee, mode, ap)) != XBEE_ENONE) throw ret;
	
	connections.begin();
}
EXPORT xbeep::~xbeep() {
	xbee_shutdown(xbee);
}

/* ########################################################################## */

EXPORT void xbeep::attachEOFCallback(void (*eofCallback)(struct xbee *xbee, void *rxInfo)) {
	xbee_err ret;
	
	if ((ret = xbee_attachEOFCallback(xbee, eofCallback)) != XBEE_ENONE) throw ret;
}

/* ########################################################################## */

EXPORT const char *xbeep::mode(void) {
	xbee_err ret;
	const char *mode;
	
	if ((ret = xbee_modeGet(xbee, &mode)) != XBEE_ENONE) throw ret;
	
	return mode;
}

/* ########################################################################## */

EXPORT unsigned int xbeep::conNew(const char *type) {
	return conNew(type, NULL);
}
EXPORT unsigned int xbeep::conNew(const char *type, struct xbee_conAddress *address) {
	xbee_err ret;
	struct xbee_con *con;
	unsigned int conId;
	
	if ((ret = xbee_conNew(xbee, &con, type, address)) != XBEE_ENONE) throw ret;
	
	conId = connections.size();
	connections.push_back(con);
	return conId;
}

EXPORT unsigned char xbeep::conTx(const unsigned int conId, const char *format, ...) {
	va_list ap;
	unsigned char retVal;
	
	va_start(ap, format);
	retVal = conTx(conId, format, ap);
	va_end(ap);
	
	return retVal;
}
EXPORT unsigned char xbeep::conTx(const unsigned int conId, const char *format, va_list ap) {
	struct xbee_con *con;
	xbee_err ret;
	unsigned char retVal;
	
	con = getCon(conId);
	
	if ((ret = xbee_convTx(con, &retVal, format, ap)) != XBEE_ENONE) throw ret;
	
	return retVal;
}
EXPORT unsigned char xbeep::conTx(const unsigned int conId, const int len, const unsigned char *data) {
	struct xbee_con *con;
	xbee_err ret;
	unsigned char retVal;
	
	con = getCon(conId);
	
	if ((ret = xbee_connTx(con, &retVal, data, len)) != XBEE_ENONE) throw ret;
	
	return retVal;
}

EXPORT struct xbee_pkt *xbeep::conRx(const unsigned int conId) {
	return conRx(conId, NULL);
}
EXPORT struct xbee_pkt *xbeep::conRx(const unsigned int conId, int *remainingPackets) {
	struct xbee_con *con;
	struct xbee_pkt *pkt;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conRx(con, &pkt, remainingPackets)) != XBEE_ENONE) throw ret;
	
	return pkt;
}

EXPORT void xbeep::conPurge(const unsigned int conId) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conPurge(con)) != XBEE_ENONE) throw ret;
}

EXPORT enum xbee_conSleepStates xbeep::conSleep(const unsigned int conId) {
	struct xbee_con *con;
	enum xbee_conSleepStates state;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conSleepGet(con, &state)) != XBEE_ENONE) throw ret;
	
	return state;
}
EXPORT enum xbee_conSleepStates xbeep::conSleep(const unsigned int conId, enum xbee_conSleepStates state) {
	enum xbee_conSleepStates oldState;
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	oldState = conSleep(conId);
	if ((ret = xbee_conSleepSet(con, state)) != XBEE_ENONE) throw ret;
	
	return oldState;
}

EXPORT void *xbeep::conData(const unsigned int conId) {
	struct xbee_con *con;
	void *data;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conDataGet(con, &data)) != XBEE_ENONE) throw ret;
	
	return data;
}
EXPORT void *xbeep::conData(const unsigned int conId, void *newData) {
	struct xbee_con *con;
	void *oldData;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conDataSet(con, newData, &oldData)) != XBEE_ENONE) throw ret;
	
	return oldData;

}

EXPORT void xbeep::conInfoGet(const unsigned int conId, struct xbee_conInfo *info) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conInfoGet(con, info)) != XBEE_ENONE) throw ret;
}

EXPORT xbee_t_conCallback xbeep::conCallbackSet(const unsigned int conId, xbee_t_conCallback newCallback) {
	struct xbee_con *con;
	xbee_t_conCallback oldCallback;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conCallbackSet(con, newCallback, &oldCallback)) != XBEE_ENONE) throw ret;
	
	return oldCallback;
}
EXPORT xbee_t_conCallback xbeep::conCallbackGet(const unsigned int conId) {
	struct xbee_con *con;
	xbee_t_conCallback callback;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conCallbackGet(con, &callback)) != XBEE_ENONE) throw ret;
	
	return callback;
}

EXPORT void xbeep::conSettings(const unsigned int conId, struct xbee_conSettings *newSettings,struct xbee_conSettings *oldSettings) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conSettings(con, newSettings, oldSettings)) != XBEE_ENONE) throw ret;
}

EXPORT void xbeep::conEnd(const unsigned int conId) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conEnd(con)) != XBEE_ENONE) throw ret;
	
	connections[conId] = NULL;
}

/* ########################################################################## */

EXPORT void xbeep::netStart(int port) {
	netStart(port, NULL);
}
EXPORT void xbeep::netvStart(int fd) {
	netvStart(fd, NULL);
}
EXPORT void xbeep::netStart(int port, int(*clientFilter)(struct xbee *xbee, const char *remoteHost)) {
	xbee_err ret;
	
	if ((ret = xbee_netStart(xbee, port, clientFilter)) != XBEE_ENONE) throw ret;
}
EXPORT void xbeep::netvStart(int fd, int(*clientFilter)(struct xbee *xbee, const char *remoteHost)) {
	xbee_err ret;
	
	if ((ret = xbee_netvStart(xbee, fd, clientFilter)) != XBEE_ENONE) throw ret;
}

/* ########################################################################## */

EXPORT void xbeep::logTargetSet(FILE *f) {
	xbee_err ret;
	
	if ((ret = xbee_logTargetSet(xbee, f)) != XBEE_ENONE) throw ret;
}

EXPORT FILE *xbeep::logTargetGet(void) {
	FILE *f;
	xbee_err ret;
	
	if ((ret = xbee_logTargetGet(xbee, &f)) != XBEE_ENONE) throw ret;
	
	return f;
}

EXPORT void xbeep::logLevelSet(int level) {
	xbee_err ret;
	
	if ((ret = xbee_logLevelSet(xbee, level)) != XBEE_ENONE) throw ret;
}

EXPORT int xbeep::logLevelGet(void) {
	int level;
	xbee_err ret;
	
	if ((ret = xbee_logLevelGet(xbee, &level)) != XBEE_ENONE) throw ret;
	
	return level;
}

EXPORT void xbeep::logDev(const char *file, int line, const char *function, int minLevel, const char *format, ...) {
	xbee_err ret;
	
	if ((ret = _xbee_logDev(file, line, function, xbee, minLevel, format)) != XBEE_ENONE) throw ret;
}
