#ifndef __XBEE_CONN_H
#define __XBEE_CONN_H

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

extern struct ll_head *conList;

struct xbee_con {
	struct xbee *xbee;
	struct ll_head *pktList;
	struct xbee_modeConType *conType;
	
	void *userData;
	
	xbee_t_conCallback callback;
	
	unsigned char frameId;
	
	enum xbee_conSleepStates sleepState;
	struct xbee_conAddress address;
	struct xbee_conInfo info;
	struct xbee_conSettings settings;
};

xbee_err xbee_conAlloc(struct xbee_con **nCon);
xbee_err xbee_conFree(struct xbee_con *con);

xbee_err xbee_conLink(struct xbee *xbee, struct xbee_con *con);
xbee_err xbee_conUnlink(struct xbee *xbee, struct xbee_con *con);

#endif /* __XBEE_CONN_H */