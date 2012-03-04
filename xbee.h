#ifndef __XBEE_H
#define __XBEE_H

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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>

/* ######################################################################### */

/* structs that won't be defined in user-space */
struct xbee;
struct xbee_con;


/* ######################################################################### */
/* tasty structs 'n stuff */

/* these must increment away from 'awake' */
enum xbee_conSleepStates {
	CON_AWAKE  = 0,
	CON_SNOOZE = 1,
	CON_SLEEP  = 2,
};

struct xbee_conAddress {
	unsigned char addr16_enabled;
	unsigned char addr16[2];
	
	unsigned char addr64_enabled;
	unsigned char addr64[8];
	
	unsigned char endpoints_enabled;
	unsigned char endpoint_local;
	unsigned char endpoint_remote;
};

struct xbee_conInfo {
	int countRx;
	int countTx;
	
	time_t lastRxTime;
};

struct xbee_conSettings {
	unsigned char disableAck       : 1;
	unsigned char broadcastPAN     : 1;
	unsigned char queueChanges     : 1;
	unsigned char multicast        : 1;
	unsigned char noBlock          : 1;
	unsigned char broadcastRadius;
};

/* ######################################################################### */

struct xbee_pkt {
	struct xbee *xbee;
	struct xbee_con *con;

	unsigned char status;
	unsigned char options;
	unsigned char rssi; /* print as "RSSI: -%d\n" */
	unsigned char frameId;
	
	unsigned char atCommand[2];
	
	struct ll_head *dataItems;
	
	int dataLen;
	unsigned char data[1];
};

/* ######################################################################### */

enum xbee_errors {
	XBEE_ENONE                 =  0,
	XBEE_EUNKNOWN              = -1,
	
	XBEE_ENOMEM                = -2,
	
	XBEE_ESELECT               = -3,
	XBEE_ESELECTINTERRUPTED    = -4,
	
	XBEE_EEOF                  = -5,
	XBEE_EIO                   = -6,
	
	XBEE_ESEMAPHORE            = -7,
	XBEE_EMUTEX                = -8,
	XBEE_ETHREAD               = -9,
	XBEE_ELINKEDLIST           = -10,
	
	XBEE_ESETUP                = -11,
	XBEE_EMISSINGPARAM         = -12,
	XBEE_EINVAL                = -13,
	XBEE_ERANGE                = -14,
	XBEE_ELENGTH               = -15,
	
	XBEE_EFAILED               = -18,
	XBEE_ETIMEOUT              = -17,
	XBEE_EWOULDBLOCK           = -16,
	XBEE_EINUSE                = -19,
	XBEE_EEXISTS               = -20,
	XBEE_ENOTEXISTS            = -21,
	XBEE_ENOFREEFRAMEID        = -22,
	
	XBEE_ESTALE                = -23,
	XBEE_ENOTIMPLEMENTED       = -24,
	
	XBEE_ETX                   = -25,
	
	XBEE_EREMOTE               = -26,
	
	XBEE_ESLEEPING             = -27,
};
typedef enum xbee_errors xbee_err;

/* ######################################################################### */
/* ######################################################################### */
/* --- ver.c --- */
extern const char libxbee_revision[];
extern const char libxbee_commit[];
extern const char libxbee_committer[];
extern const char libxbee_buildtime[];


/* ######################################################################### */
/* --- xbee.c --- */
xbee_err xbee_validate(struct xbee *xbee);
xbee_err xbee_setup(struct xbee **retXbee, const char *mode, ...);
xbee_err xbee_vsetup(struct xbee **retXbee, const char *mode, va_list ap);
xbee_err xbee_attachEOFCallback(struct xbee *xbee, void (*eofCallback)(struct xbee *xbee, void *rxInfo));
xbee_err xbee_shutdown(struct xbee *xbee);


/* ######################################################################### */
/* --- mode.c --- */
xbee_err xbee_modeGetList(char ***retList);
xbee_err xbee_modeGet(struct xbee *xbee, const char **mode);


/* ######################################################################### */
/* --- conn.c --- */
typedef void(*xbee_t_conCallback)(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data);

xbee_err xbee_conGetTypes(struct xbee *xbee, char ***retList);
/* - */
xbee_err xbee_conNew(struct xbee *xbee, struct xbee_con **retCon, const char *type, struct xbee_conAddress *address);
xbee_err xbee_conValidate(struct xbee_con *con);
/* - */
xbee_err xbee_conTx(struct xbee_con *con, unsigned char *retVal, const char *format, ...);
xbee_err xbee_convTx(struct xbee_con *con, unsigned char *retVal, const char *format, va_list args);
xbee_err xbee_connTx(struct xbee_con *con, unsigned char *retVal, const unsigned char *buf, int len);
xbee_err xbee_conRx(struct xbee_con *con, struct xbee_pkt **retPkt, int *remainingPackets);
/* - */
xbee_err xbee_conSleepSet(struct xbee_con *con, enum xbee_conSleepStates state);
xbee_err xbee_conSleepGet(struct xbee_con *con, enum xbee_conSleepStates *state);
/* - */
xbee_err xbee_conDataSet(struct xbee_con *con, void *newData, void **oldData);
xbee_err xbee_conDataGet(struct xbee_con *con, void **curData);
/* - */
xbee_err xbee_conInfoGet(struct xbee_con *con, struct xbee_conInfo *info);
/* - */
xbee_err xbee_conCallbackSet(struct xbee_con *con, xbee_t_conCallback newCallback, xbee_t_conCallback *oldCallback);
xbee_err xbee_conCallbackGet(struct xbee_con *con, xbee_t_conCallback *curCallback);
/* - */
xbee_err xbee_conSettings(struct xbee_con *con, struct xbee_conSettings *newSettings, struct xbee_conSettings *oldSettings);
/* - */
xbee_err xbee_conEnd(struct xbee_con *con);


/* ######################################################################### */
/* --- pkt.c --- */
xbee_err xbee_pktFree(struct xbee_pkt *pkt);
xbee_err xbee_pktValidate(struct xbee_pkt *pkt);
xbee_err xbee_pktDataGet(struct xbee_pkt *pkt, const char *key, int id, int index, void **retData);
xbee_err xbee_pktAnalogGet(struct xbee_pkt *pkt, int channel, int index, int *retVal);
xbee_err xbee_pktDigitalGet(struct xbee_pkt *pkt, int channel, int index, int *retVal);


/* ######################################################################### */
/* --- net.c --- */
xbee_err xbee_netStart(struct xbee *xbee, int port, int(*clientFilter)(struct xbee *xbee, const char *remoteHost));
xbee_err xbee_netvStart(struct xbee *xbee, int fd, int(*clientFilter)(struct xbee *xbee, const char *remoteHost));
xbee_err xbee_netStop(struct xbee *xbee);


/* ######################################################################### */
/* --- log.c --- */
xbee_err xbee_logTargetSet(struct xbee *xbee, FILE *f);
xbee_err xbee_logTargetGet(struct xbee *xbee, FILE **f);
xbee_err xbee_logLevelSet(struct xbee *xbee, int level);
xbee_err xbee_logLevelGet(struct xbee *xbee, int *level);

#ifndef __XBEE_INTERNAL_H
xbee_err _xbee_logDev(const char *file, int line, const char *function, struct xbee *xbee, int minLevel, const char *format, ...);
#define xbee_log(...) _xbee_logDev(__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#endif


/* ########################################################################## */
/* --- error.c --- */
const char *xbee_errorToStr(xbee_err error);


/* ######################################################################### */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __XBEE_H */
