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
#include <sys/socket.h>

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../log.h"
#include "../../net_io.h"
#include "../../net_handlers.h"
#include "../../mode.h"
#include "../../conn.h"
#include "../../frame.h"
#include "../../pkt.h"
#include "mode.h"
#include "net.h"

static xbee_err init(struct xbee *xbee, va_list ap);
static xbee_err prepare(struct xbee *xbee);
static xbee_err mode_shutdown(struct xbee *xbee);

/* ######################################################################### */

static xbee_err init(struct xbee *xbee, va_list ap) {
	xbee_err ret;
	char *t;
	struct xbee_modeData *data;
	if (!xbee) return XBEE_EMISSINGPARAM;
	
	if ((data = malloc(sizeof(*data))) == NULL) return XBEE_ENOMEM;
	memset(data, 0, sizeof(*data));
	xbee->modeData = data;
	
	ret = XBEE_ENONE;
	
	/* get the hostname */
	t = va_arg(ap, char*);
	if ((data->netInfo.host = malloc(strlen(t) + 1)) == NULL) { ret = XBEE_ENOMEM; goto die; }
	strcpy(data->netInfo.host, t);
	
	/* get the port number */
	data->netInfo.port = va_arg(ap, int);
	
	/* setup the network interface */
	if ((ret = xbee_netSetup(&data->netInfo)) != XBEE_ENONE) goto die;
	
	return XBEE_ENONE;
die:
	mode_shutdown(xbee);
	return ret;
}

static xbee_err prepare(struct xbee *xbee) {
	xbee_err ret;
	unsigned char retVal;
	struct xbee_modeData *data;
	struct xbee_conAddress address;
	struct xbee_pkt *pkt;
	int callbackCount;
	int i, pos, slen;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (!xbee->mode || !xbee->modeData) return XBEE_EINVAL;
	
	data = xbee->modeData;
	
	/* create the 'start' backchannel connection - this is ALWAYS ON ENDPOINT 0x00 */
	memset(&address, 0, sizeof(address));
	address.endpoints_enabled = 1;
	address.endpoint_local = 0;
	address.endpoint_remote = 0;
	if ((ret = _xbee_conNew(xbee, &xbee->iface, 1, &data->bc_start, "backchannel", &address)) != XBEE_ENONE) return ret;

	/* transmit our libxbee_commit string - the git commit id */
	if ((ret = xbee_conTx(data->bc_start, &retVal, "%s", libxbee_commit)) != XBEE_ENONE) {
		switch (retVal) {
			case 1:
				xbee_log(0, "The server is running a different version of libxbee");
				break;
			case 2:
				xbee_log(0, "The server encountered an internal error");
				break;
			default:
				xbee_log(0, "Failed to initialize connection to server for an unknown reason...");
		}
		return ret;
	}
	
	/* grab the returned data (an in-order list of the back channel endpoints, starting at 0x01) */
	if ((ret = xbee_conRx(data->bc_start, &pkt, NULL)) != XBEE_ENONE) return ret;
	
	callbackCount = pkt->data[0];
	
	memset(&address, 0, sizeof(address));
	address.endpoints_enabled = 1;
	
	for (pos = 1, i = 1; pos < pkt->dataLen; pos += slen + 1, i++) {
		char *name;
		struct xbee_con **retCon;
		
		name = (char *)&(pkt->data[pos]);
		slen = strlen(name);
		
		/* check for a buffer overflow */
		if (slen > pkt->dataLen - pos) {
			slen = pkt->dataLen - pos;
			name[slen] = '\0';
		}
		
		retCon = NULL;
		
		/* try to match the string with an element in struct xbee_modeData */
#define TRY(conName)  if (!data->bc_##conName && !strncasecmp(name, #conName, slen))
		TRY (connTx) {
			retCon = &data->bc_connTx;
		} else TRY (conRx) {
			retCon = &data->bc_conRx;
		} else TRY (conValidate) {
			retCon = &data->bc_conValidate;
		} else TRY (conSleep) {
			retCon = &data->bc_conSleep;
		} else TRY (conInfoGet) {
			retCon = &data->bc_conInfoGet;
		} else TRY (conSettings) {
			retCon = &data->bc_conSettings;
		} else TRY (conNew) {
			retCon = &data->bc_conNew;
		} else TRY (conEnd) {
			retCon = &data->bc_conEnd;
		} else TRY (conGetTypes) {
			retCon = &data->bc_conGetTypes;
		} else TRY (echo) {
			retCon = &data->bc_echo;
		}
#undef TRY

		/* if we dont know about that type, then continue - unlikely, but possible
		   e.g: if XBEE_NO_NET_STRICT_VERSIONS is set */
		if (!retCon) continue;
		
		/* setup the connection */
		address.endpoint_local = i;
		address.endpoint_remote = i;
		if ((ret = _xbee_conNew(xbee, &xbee->iface, 1, retCon, "backchannel", &address)) != XBEE_ENONE) return ret;
	}
	
	xbee_pktFree(pkt);
	
	/* check that we aren't missing any connections */
	if (data->bc_start == NULL)        return XBEE_EUNKNOWN;
	if (data->bc_connTx == NULL)       return XBEE_EUNKNOWN;
	if (data->bc_conRx == NULL)        return XBEE_EUNKNOWN;
	if (data->bc_conValidate == NULL)  return XBEE_EUNKNOWN;
	if (data->bc_conSleep == NULL)     return XBEE_EUNKNOWN;
	if (data->bc_conInfoGet == NULL)   return XBEE_EUNKNOWN;
	if (data->bc_conSettings == NULL)  return XBEE_EUNKNOWN;
	if (data->bc_conNew == NULL)       return XBEE_EUNKNOWN;
	if (data->bc_conEnd == NULL)       return XBEE_EUNKNOWN;
	if (data->bc_conGetTypes == NULL)  return XBEE_EUNKNOWN;
	if (data->bc_echo == NULL)         return XBEE_EUNKNOWN;
	
	return XBEE_ENONE;
}

static xbee_err mode_shutdown(struct xbee *xbee) {
	struct xbee_modeData *data;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (!xbee->mode || !xbee->modeData) return XBEE_EINVAL;
	
	data = xbee->modeData;
	
	if (data->netInfo.f) xsys_fclose(data->netInfo.f);
	if (data->netInfo.fd != -1) {	
		shutdown(data->netInfo.fd, SHUT_RDWR);
		xsys_close(data->netInfo.fd);
	}
	if (data->netInfo.host) free(data->netInfo.host);
	if (data->netInfo.txBuf) free(data->netInfo.txBuf);
	free(xbee->modeData);
	xbee->modeData = NULL;

	return XBEE_ENONE;
}

/* ######################################################################### */

const struct xbee_modeConType xbee_net_backchannel = {
	.name = "Backchannel",
	.internal = 1,
	.allowFrameId = 1, /* this needs redeclaring, because this is enabled for the client */
	.useTimeout = 1,
	.timeout = {
		.tv_sec = 5,
		.tv_nsec = 0,
	},
	.rxHandler = &xbee_netServer_backchannel_rx,
	.txHandler = &xbee_netServer_backchannel_tx,
};

static const struct xbee_modeConType *conTypes[] = {
	&xbee_net_backchannel,
	NULL,
};

const struct xbee_mode mode_net = {
	.name = "net",
	
	.conTypes = conTypes,
	
	.init = init,
	.prepare = prepare,
	.shutdown = mode_shutdown,
	
	.rx_io = xbee_netRx,
	.tx_io = xbee_netTx,
	
	.thread = NULL,
};
