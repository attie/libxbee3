#include <iostream>

#include "xbee.h"

using namespace std;

struct xbee_con *libxbee::getCon(const unsigned int conId) {
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

libxbee::libxbee(const char *mode, ...) {
	xbee_err ret;
	va_list ap;
	
	va_start(ap, mode);
	if ((ret = xbee_vsetup(&xbee, mode, ap)) != XBEE_ENONE) throw ret;
	va_end(ap);
	
	connections.begin();
}
libxbee::libxbee(const char *mode, va_list ap) {
	xbee_err ret;
	
	if ((ret = xbee_vsetup(&xbee, mode, ap)) != XBEE_ENONE) throw ret;
	
	connections.begin();
}
libxbee::~libxbee() {
	xbee_shutdown(xbee);
}

/* ########################################################################## */

void libxbee::attachEOFCallback(void (*eofCallback)(struct xbee *xbee, void *rxInfo)) {
	xbee_err ret;
	
	if ((ret = xbee_attachEOFCallback(xbee, eofCallback)) != XBEE_ENONE) throw ret;
}

/* ########################################################################## */

const char *libxbee::mode(void) {
	xbee_err ret;
	const char *mode;
	
	if ((ret = xbee_modeGet(xbee, &mode)) != XBEE_ENONE) throw ret;
	
	return mode;
}

/* ########################################################################## */

unsigned int libxbee::conNew(const char *type) {
	return conNew(type, NULL);
}
unsigned int libxbee::conNew(const char *type, struct xbee_conAddress *address) {
	xbee_err ret;
	struct xbee_con *con;
	unsigned int conId;
	
	if ((ret = xbee_conNew(xbee, &con, type, address)) != XBEE_ENONE) throw ret;
	
	conId = connections.size();
	connections.push_back(con);
	return conId;
}

unsigned char libxbee::conTx(const unsigned int conId, const char *format, ...) {
	va_list ap;
	unsigned char retVal;
	
	va_start(ap, format);
	retVal = conTx(conId, format, ap);
	va_end(ap);
	
	return retVal;
}
unsigned char libxbee::conTx(const unsigned int conId, const char *format, va_list ap) {
	struct xbee_con *con;
	xbee_err ret;
	unsigned char retVal;
	
	con = getCon(conId);
	
	if ((ret = xbee_convTx(con, &retVal, format, ap)) != XBEE_ENONE) throw ret;
	
	return retVal;
}
unsigned char libxbee::conTx(const unsigned int conId, const int len, const unsigned char *data) {
	struct xbee_con *con;
	xbee_err ret;
	unsigned char retVal;
	
	con = getCon(conId);
	
	if ((ret = xbee_connTx(con, &retVal, data, len)) != XBEE_ENONE) throw ret;
	
	return retVal;
}

struct xbee_pkt *libxbee::conRx(const unsigned int conId) {
	return conRx(conId, NULL);
}
struct xbee_pkt *libxbee::conRx(const unsigned int conId, int *remainingPackets) {
	struct xbee_con *con;
	struct xbee_pkt *pkt;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conRx(con, &pkt, remainingPackets)) != XBEE_ENONE) throw ret;
	
	return pkt;
}

void libxbee::conPurge(const unsigned int conId) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conPurge(con)) != XBEE_ENONE) throw ret;
}

enum xbee_conSleepStates libxbee::conSleep(const unsigned int conId) {
	struct xbee_con *con;
	enum xbee_conSleepStates state;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conSleepGet(con, &state)) != XBEE_ENONE) throw ret;
	
	return state;
}
enum xbee_conSleepStates libxbee::conSleep(const unsigned int conId, enum xbee_conSleepStates state) {
	enum xbee_conSleepStates oldState;
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	oldState = conSleep(conId);
	if ((ret = xbee_conSleepSet(con, state)) != XBEE_ENONE) throw ret;
	
	return oldState;
}

void *libxbee::conData(const unsigned int conId) {
	struct xbee_con *con;
	void *data;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conDataGet(con, &data)) != XBEE_ENONE) throw ret;
	
	return data;
}
void *libxbee::conData(const unsigned int conId, void *newData) {
	struct xbee_con *con;
	void *oldData;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conDataSet(con, newData, &oldData)) != XBEE_ENONE) throw ret;
	
	return oldData;

}

void libxbee::conInfoGet(const unsigned int conId, struct xbee_conInfo *info) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conInfoGet(con, info)) != XBEE_ENONE) throw ret;
}

xbee_t_conCallback libxbee::conCallbackSet(const unsigned int conId, xbee_t_conCallback newCallback) {
	struct xbee_con *con;
	xbee_t_conCallback oldCallback;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conCallbackSet(con, newCallback, &oldCallback)) != XBEE_ENONE) throw ret;
	
	return oldCallback;
}
xbee_t_conCallback libxbee::conCallbackGet(const unsigned int conId) {
	struct xbee_con *con;
	xbee_t_conCallback callback;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conCallbackGet(con, &callback)) != XBEE_ENONE) throw ret;
	
	return callback;
}

void libxbee::conSettings(const unsigned int conId, struct xbee_conSettings *newSettings,struct xbee_conSettings *oldSettings) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conSettings(con, newSettings, oldSettings)) != XBEE_ENONE) throw ret;
}

void libxbee::conEnd(const unsigned int conId) {
	struct xbee_con *con;
	xbee_err ret;
	
	con = getCon(conId);
	
	if ((ret = xbee_conEnd(con)) != XBEE_ENONE) throw ret;
	
	connections[conId] = NULL;
}

/* ########################################################################## */

void libxbee::netStart(int port) {
	netStart(port, NULL);
}
void libxbee::netvStart(int fd) {
	netvStart(fd, NULL);
}
void libxbee::netStart(int port, int(*clientFilter)(struct xbee *xbee, const char *remoteHost)) {
	xbee_err ret;
	
	if ((ret = xbee_netStart(xbee, port, clientFilter)) != XBEE_ENONE) throw ret;
}
void libxbee::netvStart(int fd, int(*clientFilter)(struct xbee *xbee, const char *remoteHost)) {
	xbee_err ret;
	
	if ((ret = xbee_netvStart(xbee, fd, clientFilter)) != XBEE_ENONE) throw ret;
}

/* ########################################################################## */

void libxbee::logTargetSet(FILE *f) {
	xbee_err ret;
	
	if ((ret = xbee_logTargetSet(xbee, f)) != XBEE_ENONE) throw ret;
}

FILE *libxbee::logTargetGet(void) {
	FILE *f;
	xbee_err ret;
	
	if ((ret = xbee_logTargetGet(xbee, &f)) != XBEE_ENONE) throw ret;
	
	return f;
}

void libxbee::logLevelSet(int level) {
	xbee_err ret;
	
	if ((ret = xbee_logLevelSet(xbee, level)) != XBEE_ENONE) throw ret;
}

int libxbee::logLevelGet(void) {
	int level;
	xbee_err ret;
	
	if ((ret = xbee_logLevelGet(xbee, &level)) != XBEE_ENONE) throw ret;
	
	return level;
}

void libxbee::logDev(const char *file, int line, const char *function, int minLevel, const char *format, ...) {
	xbee_err ret;
	
	if ((ret = _xbee_logDev(file, line, function, xbee, minLevel, format)) != XBEE_ENONE) throw ret;
}
