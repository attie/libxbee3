#ifndef __XBEE_MODE_H
#define __XBEE_MODE_H

/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

struct xbee_modeDataHandlerRx {
	unsigned char identifier;
	xbee_err (*func)(struct xbee *xbee,
	                 void *arg,
	                 unsigned char identifier,
	     /* IN */    struct xbee_buf *buf,
	     /* OUT */   struct xbee_frameInfo *frameInfo,
	     /* OUT */   struct xbee_conAddress *address,
	     /* OUT */   struct xbee_pkt **pkt);
	unsigned char needsFree;
};

struct xbee_modeDataHandlerTx {
	unsigned char identifier;
	xbee_err (*func)(struct xbee *xbee,
	                 struct xbee_con *con,
	                 void *arg,
	                 unsigned char identifier,
	     /* IN */    unsigned char frameId,
	     /* IN */    struct xbee_conAddress *address,
	     /* IN */    struct xbee_conSettings *settings,
	     /* IN */    const unsigned char *buf,
	     /* IN */    int len,
	     /* OUT */   struct xbee_buf **oBuf);
	unsigned char needsFree;
};

struct xbee_modeConType {
	const char *name;
	int nameNeedsFree;
	
	struct ll_head *conList;
	
	const unsigned char internal     : 1;
	const unsigned char allowFrameId : 1;
	const unsigned char useTimeout   : 1;
	const struct timespec timeout;
	
	xbee_err (*address_validator)(struct xbee_conAddress *address);
	
	const struct xbee_modeDataHandlerRx *rxHandler;
	const struct xbee_modeDataHandlerTx *txHandler;
};

struct xbee_modeSupport {
	xbee_err (* const conNew)(struct xbee *xbee, struct xbee_interface *interface, struct xbee_modeConType *conType, struct xbee_conAddress *address, int *conIdentifier);
	xbee_err (* const conValidate)(struct xbee_con *con);
	xbee_err (* const conSleepSet)(struct xbee_con *con, enum xbee_conSleepStates state);
	xbee_err (* const conSleepGet)(struct xbee_con *con);
	xbee_err (* const conSettings)(struct xbee_con *con, struct xbee_conSettings *newSettings);
	xbee_err (* const conEnd)(struct xbee_con *con);
};

struct xbee_mode {
	const char * const name;
	
	const struct xbee_modeConType ** const conTypes;
	
	xbee_err (* const init)(struct xbee *xbee, va_list ap);
	xbee_err (* const prepare)(struct xbee *xbee);
	xbee_err (* const shutdown)(struct xbee *xbee);
	
	xbee_err (* const rx_io)(struct xbee *xbee, void *arg, struct xbee_buf **buf); /* retrieves raw buffers from the I/O device */
	xbee_err (* const tx_io)(struct xbee *xbee, void *arg, struct xbee_buf *buf); /* transmits raw buffers to the I/O device */
	
	xbee_err (* const thread)(struct xbee *xbee, int *restart, void *arg);
	
	struct xbee_modeSupport support;
};

xbee_err xbee_modeRetrieve(const char *name, const struct xbee_mode **retMode);

xbee_err xbee_modeImport(struct xbee_modeConType **retConTypes, const struct xbee_mode *mode);
xbee_err xbee_modeAddConType(struct xbee_modeConType **extConTypes, const struct xbee_modeConType *newConType);
xbee_err xbee_modeCleanup(struct xbee_modeConType *conTypes);

xbee_err xbee_modeLocateConType(struct xbee_modeConType *conTypes, int allowInternal, const char *name, const unsigned char *rxId, const unsigned char *txId, struct xbee_modeConType **retType);

/* list of potential modes... basically a list of subdirectories in './modes/' */
extern const struct xbee_mode mode_xbee1;
extern const struct xbee_mode mode_xbee2;
extern const struct xbee_mode mode_net;

#endif /* __XBEE_MODE_H */
