#ifndef ZDO_H
#define ZDO_H
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
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with libxbee. If not, see <http://www.gnu.org/licenses/>.
*/

struct xbee_zdo_data {
	struct xbee_con *tx, *rx;
	short cluster;
	void *userData;
};

xbee_err xbee_zdo_conNew(struct xbee *xbee, struct xbee_con **txCon, struct xbee_conAddress *address, short cluster, xbee_t_conCallback rxCallback);
xbee_err xbee_zdo_conDataSet(struct xbee_con *con, void *newData, void **oldData);
xbee_err xbee_zdo_conDataGet(struct xbee_con *con, void **curData);
xbee_err xbee_zdo_conEnd(struct xbee_con *con);

#endif /* ZDO_H */
