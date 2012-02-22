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
#include "../../mode.h"
#include "../../frame.h"
#include "../../pkt.h"
#include "mode.h"
#include "net.h"

static xbee_err init(struct xbee *xbee, va_list ap);
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
	
	t = va_arg(ap, char*);
	if ((data->netInfo.host = malloc(strlen(t) + 1)) == NULL) { ret = XBEE_ENOMEM; goto die; }
	strcpy(data->netInfo.host, t);
	
	data->netInfo.port = va_arg(ap, int);
	
	if ((ret = xbee_netSetup(&data->netInfo)) != XBEE_ENONE) goto die;
	
	return XBEE_ENONE;
die:
	mode_shutdown(xbee);
	return ret;
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

xbee_err xbee_netRxIo(struct xbee *xbee, struct xbee_buf **buf) {
	sleep(10);
	return XBEE_ENOTIMPLEMENTED;
}

/* ######################################################################### */

xbee_err xbee_netTxIo(struct xbee *xbee, struct xbee_buf *buf) {
	sleep(10);
	return XBEE_ENOTIMPLEMENTED;
}

/* ######################################################################### */

static const struct xbee_modeConType *conTypes[] = {
	NULL
};

struct xbee_mode mode_net = {
	.name = "net",
	
	.conTypes = conTypes,
	
	.init = init,
	.shutdown = mode_shutdown,
	
	.rx_io = xbee_netRxIo,
	.tx_io = xbee_netTxIo,
	
	.thread = NULL,
};
