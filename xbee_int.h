#ifndef __XBEE_INT_H
#define __XBEE_INT_H

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

extern struct ll_head *xbeeList;
extern struct ll_head *needsFree;

struct xbee {
	int die;
	
	struct xbee_frameBlock *fBlock;
	struct xbee_log *log;
	
	struct xbee_modeConType *conTypes; /* copied in from the mode */
	const struct xbee_mode *mode;
	void *modeData; /* for use by the mode */
	
	struct xbee_txInfo *tx;
	struct xbee_rxInfo *rx;
};

xbee_err xbee_alloc(struct xbee **nXbee);
xbee_err xbee_free(struct xbee *xbee);

#endif /* __XBEE_INT_H */
