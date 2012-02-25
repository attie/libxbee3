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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../mode.h"
#include "../../conn.h"
#include "../../log.h"
#include "mode.h"
#include "support.h"

static xbee_err getConTypeId(struct xbee_modeConType *conTypes, struct xbee_modeConType *conType, unsigned char *conTypeId) {
	int i;
	
	printf("%p\n", conType);
	for (i = 0; conTypes[i].name; i++) {
		printf("%s <-> %s\n", conType->name, conTypes[i].name);
		if (&conTypes[i] == conType) {
			if (i > 255) return XBEE_ERANGE;
			*conTypeId = i;
			return XBEE_ENONE;
		}
	}
	return XBEE_ENOTEXISTS;
}

/* ######################################################################### */

xbee_err xbee_netSupport_conNew(struct xbee *xbee, struct xbee_interface *interface, struct xbee_modeConType *conType, struct xbee_conAddress *address, int *conIdentifier) {
	xbee_err ret;
	unsigned char *buf;
	int len;
	unsigned char conTypeId;
	struct xbee_pkt *pkt;
	struct xbee_modeData *data;
	
	data = xbee->modeData;
	
	if (getConTypeId(xbee->iface.conTypes, conType, &conTypeId) != XBEE_ENONE) return XBEE_EINVAL;
	if (conTypeId == 0) return XBEE_ENONE; /* backchannel (0) is always successful */
	
	len = 1 + sizeof(*address);
	if ((buf = malloc(len)) == NULL) return XBEE_ENOMEM;
	
	buf[0] = conTypeId;
	memcpy(&(buf[1]), address, sizeof(*address));
	
	if (xbee_connTx(data->bc_conNew, NULL, buf, len) != XBEE_ENONE) return XBEE_EREMOTE;
	
	free(buf);
	
	if (xbee_conRx(data->bc_conNew, &pkt, NULL) != XBEE_ENONE) return XBEE_EREMOTE;
	
	if (pkt->dataLen == 2) {
		int conId;
		conId = 0;
		conId |= (pkt->data[0] <<  8) & 0xFF00;
		conId |= (pkt->data[1]      ) & 0xFF;
		*conIdentifier = conId;
		ret = XBEE_ENONE;
	} else {
		ret = XBEE_EREMOTE;
	}
	
	xbee_pktFree(pkt);
	
	return ret;
}

xbee_err xbee_netSupport_conValidate(struct xbee_con *con) {
	unsigned char conTypeId;
	if (getConTypeId(con->xbee->iface.conTypes, con->conType, &conTypeId) != XBEE_ENONE) return XBEE_EINVAL;
	if (conTypeId == 0) return XBEE_ENONE; /* backchannel (0) is always successful */
	
	
	
	return XBEE_ENONE;
}

xbee_err xbee_netSupport_conSleepSet(struct xbee_con *con, enum xbee_conSleepStates state) {
	unsigned char conTypeId;
	if (getConTypeId(con->xbee->iface.conTypes, con->conType, &conTypeId) != XBEE_ENONE) return XBEE_EINVAL;
	if (conTypeId == 0) return XBEE_ENONE; /* backchannel (0) is always successful */
	
	
	
	return XBEE_ENONE;
}

xbee_err xbee_netSupport_conSleepGet(struct xbee_con *con) {
	unsigned char conTypeId;
	if (getConTypeId(con->xbee->iface.conTypes, con->conType, &conTypeId) != XBEE_ENONE) return XBEE_EINVAL;
	if (conTypeId == 0) return XBEE_ENONE; /* backchannel (0) is always successful */
	
	
	
	return XBEE_ENONE;
}

xbee_err xbee_netSupport_conSettings(struct xbee_con *con, struct xbee_conSettings *newSettings) {
	unsigned char conTypeId;
	if (getConTypeId(con->xbee->iface.conTypes, con->conType, &conTypeId) != XBEE_ENONE) return XBEE_EINVAL;
	if (conTypeId == 0) return XBEE_ENONE; /* backchannel (0) is always successful */
	
	
	
	return XBEE_ENONE;
}

xbee_err xbee_netSupport_conEnd(struct xbee_con *con) {
	unsigned char conTypeId;
	if (getConTypeId(con->xbee->iface.conTypes, con->conType, &conTypeId) != XBEE_ENONE) return XBEE_EINVAL;
	if (conTypeId == 0) return XBEE_ENONE; /* backchannel (0) is always successful */
	
	
	
	return XBEE_ENONE;
}
