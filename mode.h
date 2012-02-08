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

struct xbee_mode {
	const char *name;
	
	xbee_err (*init)(struct xbee *xbee, va_list ap);
	xbee_err (*shutdown)(struct xbee *xbee);
	
	xbee_err (*rx_io)(struct xbee *xbee, struct xbee_buf **buf); /* retrieves raw buffers from the I/O device */
	xbee_err (*rx_pkt)(struct xbee *xbee, struct xbee_buf *buf, struct xbee_conAddress **address, struct xbee_pkt **pkt); /* translates raw buffers into address/packet pairs */
	
	xbee_err (*tx_io)(struct xbee *xbee, struct xbee_buf *buf); /* transmits raw buffers to the I/O device */
	xbee_err (*tx_pkt)(struct xbee *xbee, struct xbee_conAddress *address, struct xbee_buf **buf); /* translates address/payload pairs into raw buffers */
	
	xbee_err (*thread)(struct xbee *xbee, int *restart, void *arg);
};

xbee_err xbee_modeRetrieve(char *name, const struct xbee_mode **retMode);

#endif /* __XBEE_MODE_H */