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

/* structs that will be defined/used later */
struct xbee;
struct xbee_con;
enum xbee_conSleepStates;
struct xbee_conAddress;
struct xbee_conInfo;
struct xbee_conSettings;
struct xbee_pkt;
typedef enum xbee_errors xbee_err;

/* ######################################################################### */
/* --- ver.c --- */
extern const char libxbee_revision[];
extern const char libxbee_commit[];
extern const char libxbee_committer[];
extern const char libxbee_buildtime[];


/* ######################################################################### */
/* --- xbee.c --- */
xbee_err xbee_validate(struct xbee *xbee);
xbee_err xbee_setup(struct xbee **retXbee, char *mode, ...);
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
xbee_err xbee_conNew(struct xbee *xbee, struct xbee_con **retCon, char *type, struct xbee_conAddress *address);
xbee_err xbee_conValidate(struct xbee_con *con);
/* - */
xbee_err xbee_conTx(struct xbee_con *con, unsigned char *retVal, char *format, ...);
xbee_err xbee_convTx(struct xbee_con *con, unsigned char *retVal, char *format, va_list args);
xbee_err xbee_connTx(struct xbee_con *con, unsigned char *retVal, unsigned char *buf, int len);
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
xbee_err xbee_pktAnalogGet(struct xbee_pkt *pkt, int channel, int index, int *retVal);
xbee_err xbee_pktDigitalGet(struct xbee_pkt *pkt, int channel, int index, int *retVal);


/* ######################################################################### */
/* --- net.c --- */
xbee_err xbee_netStart(struct xbee *xbee, int port);
xbee_err xbee_netvStart(struct xbee *xbee, int fd);
xbee_err xbee_netStop(struct xbee *xbee);


/* ######################################################################### */
/* --- log.c --- */
xbee_err xbee_logSetTarget(struct xbee *xbee, FILE *f);
xbee_err xbee_logGetTarget(struct xbee *xbee, FILE **f);
xbee_err xbee_logSetLevel(struct xbee *xbee, int level);
xbee_err xbee_logGetLevel(struct xbee *xbee, int *level);

#ifndef __XBEE_INTERNAL_H
xbee_err _xbee_logDev(const char *file, int line, const char *function, struct xbee *xbee, int minLevel, char *format, ...);
#define xbee_log(...) _xbee_logDev(__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#endif


/* ########################################################################## */
/* --- error.c --- */
char *xbee_errorToStr(xbee_err error);


/* ######################################################################### */
/* ######################################################################### */
/* tasty structs 'n stuff */

enum xbee_conSleepStates {
	CON_AWAKE,
	CON_SNOOZE,
	CON_SLEEP,
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
	int failedTx;
	
	int lastRssi;
	int lastRxTime;
};

struct xbee_conSettings {
	unsigned char disableAck       : 1;
	unsigned char broadcastPAN     : 1;
	unsigned char queueChanges     : 1;
	unsigned char multicast        : 1;
	unsigned char broadcastRadius  : 1;
	unsigned char noBlock          : 1;
};

/* ######################################################################### */

struct xbee_pkt {
	struct xbee *xbee;
	struct xbee_con *con;

	unsigned char status;
	unsigned char settings;
	unsigned char rssi; /* print as "RSSI: -%d\n" */
	
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
	XBEE_EIORETRIES            = -6,
	XBEE_EOPENFAILED           = -7,
	XBEE_EIO                   = -8,
	XBEE_ESEMAPHORE            = -9,
	XBEE_ELINKEDLIST           = -10,
	XBEE_ETHREAD               = -11,
	XBEE_ENOXBEE               = -12,
	XBEE_EMISSINGPARAM         = -13,
	XBEE_EINVALBAUDRATE        = -14,
	XBEE_ESETUP                = -15,
	XBEE_ELENGTH               = -16,
	XBEE_EINVAL                = -17,
	XBEE_EBUSY                 = -18,
	XBEE_ENOMODE               = -19,
	XBEE_EFAILED               = -20,
	XBEE_ECANTTX               = -21,
	XBEE_ENOTREADY             = -22,
	XBEE_ECALLBACK             = -23,
	XBEE_EUNESCAPED_START      = -24,
	XBEE_ETIMEOUT              = -25,
	XBEE_EMUTEX                = -26,
	XBEE_EINUSE                = -27,
	XBEE_ERANGE                = -28,
	XBEE_EEXISTS               = -29,
	XBEE_ENOTEXISTS            = -30,
	XBEE_ESOCKET               = -31,
	XBEE_ENOTIMPLEMENTED       = -32,
	XBEE_ESTALE                = -33,
	XBEE_ETX                   = -34,
	XBEE_EWOULDBLOCK           = -35,
	XBEE_ENOFREEFRAMEID        = -36,
};

/* ######################################################################### */


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __XBEE_H */
