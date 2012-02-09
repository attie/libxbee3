#ifndef __XBEE_MODE_H
#define __XBEE_MODE_H

/*
  libxbee - a C library to aid the use of Digi's XBee wireless modules
            running in API mode (AP=2).

  Copyright (C) 2009  Attie Grande (attie@attie.co.uk)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

struct xbee_modeDataHandlerRx {
	unsigned char identifier;
	/* -------------------------------------------->     IN                           OUT                        OUT  */
	xbee_err (*func)(struct xbee *xbee, struct xbee_buf *buf, struct xbee_conAddress *address, struct xbee_pkt **pkt);
};

struct xbee_modeDataHandlerTx {
	unsigned char identifier;
	/* -------------------------------------------->            IN                        IN                      OUT  */
	xbee_err (*func)(struct xbee *xbee, struct xbee_conAddress *address, struct xbee_buf *iBuf, struct xbee_buf **oBuf);
};

struct xbee_modeConType {
	char *name;
	
	struct xbee_modeDataHandlerRx *rxHandler;
	struct xbee_modeDataHandlerTx *txHandler;
};

struct xbee_mode {
	const char * const name;
	
	const struct xbee_modeConType * const conTypes;
	
	xbee_err (* const init)(struct xbee *xbee, va_list ap);
	xbee_err (* const shutdown)(struct xbee *xbee);
	
	xbee_err (* const rx_io)(struct xbee *xbee, struct xbee_buf **buf); /* retrieves raw buffers from the I/O device */
	xbee_err (* const tx_io)(struct xbee *xbee, struct xbee_buf *buf); /* transmits raw buffers to the I/O device */
	
	xbee_err (* const thread)(struct xbee *xbee, int *restart, void *arg);
};

xbee_err xbee_modeRetrieve(char *name, const struct xbee_mode **retMode);
xbee_err xbee_modeImport(const struct xbee_mode *mode, struct xbee_modeConType **conTypes);
xbee_err xbee_modeLocateConType(const struct xbee_mode *mode, char *name, unsigned char *rxId, unsigned char *txId, struct xbee_modeConType **retType);

#endif /* __XBEE_MODE_H */