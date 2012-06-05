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

/* this may be moved into libxbee:xbeeZB
     but for now it's implemented in your space! */

/* while this is implemented in your space, you MUST NOT use the standard
     xbee_conDataSet() and xbee_conDataGet() functions. Instead you must use
     the xbee_zdo_*() alternatives */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <xbee.h>
#include "zdo.h"

xbee_err xbee_zdo_modeValid(struct xbee *xbee) {
	xbee_err ret;
	const char *mode;
	
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if ((ret = xbee_modeGet(xbee, &mode)) != XBEE_ENONE) return ret;
	
	if (!strcasecmp(mode, "xbeeZB")) return XBEE_ENONE;
	return XBEE_EINVAL;
}

xbee_err xbee_zdo_conNew(struct xbee *xbee, struct xbee_con **txCon, struct xbee_conAddress *address, short cluster, xbee_t_conCallback rxCallback) {
	xbee_err ret;
	struct xbee_conAddress iAddress;
	struct xbee_zdo_data *data;
	
	if (!xbee || !txCon || !address || !rxCallback) return XBEE_EMISSINGPARAM;
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_zdo_modeValid(xbee) != XBEE_ENONE) return XBEE_EINVAL;
	if (!address->addr64_enabled) return XBEE_EINVAL;
	
	if ((data = malloc(sizeof(*data))) == NULL) {
		return XBEE_ENOMEM;
	}
	memset(data, 0, sizeof(*data));
	data->cluster = cluster & 0x7FFF;
	
	memset(&iAddress, 0, sizeof(iAddress));
	
	iAddress.addr64_enabled = 1;
	memcpy(iAddress.addr64, address->addr64, 8);
	
	iAddress.endpoints_enabled = 1;
	iAddress.endpoint_local = 0x00;
	iAddress.endpoint_remote = 0x00;
	
	iAddress.profile_enabled = 1;
	iAddress.profile_id = 0x0000;
	
	iAddress.cluster_enabled = 1;
	
	/* build the transmit connection */
	iAddress.cluster_id = 0x0000 | data->cluster;
	if ((ret = xbee_conNew(xbee, &data->tx, "Data (explicit)", &iAddress)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conNew(tx) returned: %d (%s)", ret, xbee_errorToStr(ret));
	}
	if ((ret = xbee_conDataSet(data->tx, data, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conDataSet(tx) returned: %d", ret);
		goto die;
	}
	
	/* build the receive connection */
	iAddress.cluster_id = 0x8000 | data->cluster;
	if ((ret = xbee_conNew(xbee, &data->rx, "Data (explicit)", &iAddress)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conNew(rx) returned: %d (%s)", ret, xbee_errorToStr(ret));
		goto die;
	}
	if ((ret = xbee_conDataSet(data->rx, data, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conDataSet(rx) returned: %d", ret);
		goto die;
	}
	if ((ret = xbee_conCallbackSet(data->rx, rxCallback, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conCallbackSet(rx) returned: %d", ret);
		goto die;
	}
	
	*txCon = data->tx;
	
	return XBEE_ENONE;
	
die:
	if (data) {
		if (data->tx) xbee_conEnd(data->tx);
		if (data->rx) xbee_conEnd(data->rx);
		free(data);
	}
	return ret;
}

xbee_err xbee_zdo_conDataSet(struct xbee_con *con, void *newData, void **oldData) {
	struct xbee_zdo_data *data;
	if (!con) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_conDataGet(con, (void**)&data) != XBEE_ENONE || data == NULL) return XBEE_EINVAL;
	if (oldData) *oldData = data->userData;
	data->userData = newData;
	return XBEE_ENONE;
}

xbee_err xbee_zdo_conDataGet(struct xbee_con *con, void **curData) {
	struct xbee_zdo_data *data;
	if (!con || !curData) return XBEE_EMISSINGPARAM;
	if (xbee_conValidate(con) != XBEE_ENONE) return XBEE_EINVAL;
	if (xbee_conDataGet(con, (void**)&data) != XBEE_ENONE || data == NULL) return XBEE_EINVAL;
	*curData = data->userData;
	return XBEE_ENONE;
}

xbee_err xbee_zdo_conEnd(struct xbee_con *con) {
	xbee_err ret;
	struct xbee_zdo_data *data;

	if (xbee_conDataGet(con, (void**)&data) != XBEE_ENONE || data == NULL) return XBEE_EINVAL;
	if (data->tx == NULL || data->rx == NULL) return XBEE_EINVAL;
	
	xbee_conEnd(data->tx);
	xbee_conEnd(data->rx);
	free(data);
	
	return XBEE_ENONE;
}