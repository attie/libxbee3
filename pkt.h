#ifndef __XBEE_PKT_H
#define __XBEE_PKT_H

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

struct pkt_dataKey;

extern struct ll_head *pktList;

xbee_err xbee_pktAlloc(struct xbee *xbee, struct xbee_pkt *oPkt, int dataLen);

xbee_err xbee_pktDataKeyAdd(struct xbee_pkt *pkt, char *key, int id, struct pkt_dataKey **retKey, void (*freeCallback)(void*));
xbee_err xbee_pktDataKeyGet(struct xbee_pkt *pkt, char *key, int id, struct pkt_dataKey **retKey);

xbee_err xbee_pktDataAdd(struct xbee_pkt *pkt, char *key, int id, void *data, void (*freeCallback)(void*));
xbee_err xbee_pktDataGet(struct xbee_pkt *pkt, char *key, int id, int index, void **retData);

xbee_err xbee_pktAnalogAdd(struct xbee_pkt *pkt, int channel, int value);
xbee_err xbee_pktDigitalAdd(struct xbee_pkt *pkt, int channel, int value);

#endif /* __XBEE_PKT_H */
