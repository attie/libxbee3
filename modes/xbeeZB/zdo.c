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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../ll.h"
#include "../../log.h"
#include "mode.h"
#include "zdo.h"

/* ######################################################################### */

void xbee_sZB_zdo_callback(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	int i;
	xbee_log(1, "New ZDO message!");
	if (!((*pkt)->address.cluster_id & 0x8000)) {
		xbee_log(2, "Client message - 0x%04X", (*pkt)->address.cluster_id);
	} else {
		xbee_log(2, "Server message - 0x%04X", (*pkt)->address.cluster_id);
	}
	switch ((*pkt)->address.cluster_id & 0x7FFF) {
		case 0x0000: xbee_log(3, "   NWK_addr"); break;
		case 0x0001: xbee_log(3, "   IEEE_addr"); break;
		case 0x0002: xbee_log(3, "   Node_Desc"); break;
		case 0x0003: xbee_log(3, "   Power_Desc"); break;
		case 0x0004: xbee_log(3, "   Simple_Desc"); break;
		case 0x0005: xbee_log(3, "   Active_EP"); break; /* <--- */
		case 0x0006: xbee_log(3, "   Match_Desc"); break;
		
		case 0x0010: xbee_log(3, "   Complex_Desc"); break;
		case 0x0011: xbee_log(3, "   User_Desc"); break;
		case 0x0012: xbee_log(3, "   Discovery_Cache"); break;
		case 0x0013: xbee_log(3, "   Device_annce"); break; /* <--- */
		case 0x0014: xbee_log(3, "   User_Desc_set"); break;
		case 0x0015: xbee_log(3, "   System_Server_Discovery"); break;
		case 0x0016: xbee_log(3, "   Discovery_store"); break;
		case 0x0017: xbee_log(3, "   Node_Desc_store"); break;
		case 0x0018: xbee_log(3, "   Power_Desc_store"); break;
		case 0x0019: xbee_log(3, "   Active_EP_store"); break;
		case 0x001A: xbee_log(3, "   Simple_Desc_store"); break;
		case 0x001B: xbee_log(3, "   Remove_node_cache"); break;
		case 0x001C: xbee_log(3, "   Find_node_cache"); break;
		case 0x001D: xbee_log(3, "   Extended_Simple_Desc"); break;
		case 0x001E: xbee_log(3, "   Extended_Active_EP"); break;
		
		case 0x0020: xbee_log(3, "   End_Device_Bind"); break;
		case 0x0021: xbee_log(3, "   Bind"); break;
		case 0x0022: xbee_log(3, "   Unbind"); break;
		case 0x0023: xbee_log(3, "   Bind_Register"); break;
		case 0x0024: xbee_log(3, "   Replace_Device"); break;
		case 0x0025: xbee_log(3, "   Store_Bkup_Bind_Entry"); break;
		case 0x0026: xbee_log(3, "   Remove_Bkup_Bind_Entry"); break;
		case 0x0027: xbee_log(3, "   Backup_Bind_Table"); break;
		case 0x0028: xbee_log(3, "   Recover_Bind_Table"); break;
		case 0x0029: xbee_log(3, "   Backup_Source_Bind"); break;
		case 0x002A: xbee_log(3, "   Recover_Source_Bind"); break;
		
		case 0x0030: xbee_log(3, "   Mgmt_NWK_Disk"); break;
		case 0x0031: xbee_log(3, "   Mgmt_Lqi"); break;
		case 0x0032: xbee_log(3, "   Mgmt_Rtg"); break;
		case 0x0033: xbee_log(3, "   Mgmt_Bind"); break;
		case 0x0034: xbee_log(3, "   Mgmt_Leave"); break;
		case 0x0035: xbee_log(3, "   Mgmt_Direct_Join"); break;
		case 0x0036: xbee_log(3, "   Mgmt_Permit_Joining"); break;
		case 0x0037: xbee_log(3, "   Mgmt_Cache"); break;
		case 0x0038: xbee_log(3, "   Mgmt_NWK_Update"); break;
		default:     xbee_log(3, "   ** Unknown ZDO packet received **"); break;
	}
	
	for (i = 0; i < (*pkt)->dataLen; i++) {
		xbee_log(10 "%3d: 0x%02X", i, (*pkt)->data[i]);
	}
}

/* ######################################################################### */

xbee_err xbee_sZB_zdo_init(struct xbee *xbee) {
	xbee_err ret;
	struct xbee_modeData *modeData;
	struct xbee_zdoData *data;
	struct xbee_conAddress addr;
	
	if ((modeData = xbee->modeData) == NULL) return XBEE_EINVAL;
	if ((data = modeData->zdoData) == NULL) {
		if ((data = modeData->zdoData = malloc(sizeof(*data))) == NULL) return XBEE_ENOMEM;
		memset(data, 0, sizeof(*data));
	}
	
	/* address ALL ZDO packets */
	memset(&addr, 0, sizeof(addr));
	addr.addr16_enabled    = 1; /* covered by wildcard */
	addr.addr16_wildcard   = 1;
	addr.addr64_enabled    = 1; /* covered by wildcard */
	addr.addr64_wildcard   = 1;
	addr.endpoints_enabled = 1; /* 0 from memset */
	addr.profile_enabled   = 1; /* 0 from memset */
	addr.cluster_enabled   = 1; /* covered by wildcard */
	addr.cluster_wildcard  = 1;
	
	if ((ret = xbee_conNew(xbee, &data->con, "Data (explicit)", &addr)) != XBEE_ENONE) return ret;
	if ((ret = xbee_conCallbackSet(data->con, xbee_sZB_zdo_callback, NULL)) != XBEE_ENONE) return ret;
	
	return XBEE_ENONE;
}

void xbee_sZB_zdo_shutdown(struct xbee *xbee) {
	struct xbee_modeData *modeData;
	struct xbee_zdoData *data;
	
	if ((modeData = xbee->modeData) == NULL) return;
	if ((data = modeData->zdoData) == NULL) return;
	modeData->zdoData = NULL;
	
	if (data->con) xbee_conEnd(data->con);
	
	free(data);
}
