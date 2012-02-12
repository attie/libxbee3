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
#include "../common.h"
#include "mode.h"

static xbee_err shutdown(struct xbee *xbee);
static xbee_err init(struct xbee *xbee, va_list ap);

/* ######################################################################### */

static xbee_err init(struct xbee *xbee, va_list ap) {
	xbee_err ret;
	char *t;
	struct xbee_modeData *data;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	
	if ((data = malloc(sizeof(*data))) == NULL) return XBEE_ENOMEM;
	
	ret = XBEE_ENONE;
	
	/* currently I don't see a better way than this - using va_arg()... which is gross */	
	t = va_arg(ap, char*);
	if ((data->serialInfo.device = malloc(strlen(t) + 1)) == NULL) { ret = XBEE_ENOMEM; goto die; }
	strcpy(data->serialInfo.device, t);
	
	data->serialInfo.baudrate = va_arg(ap, int);
	
	if ((ret = xbee_serialSetup(&data->serialInfo)) != XBEE_ENONE) goto die;
	
	xbee->modeData = data;
	return XBEE_ENONE;
die:
	shutdown(xbee);
	return ret;
}

static xbee_err shutdown(struct xbee *xbee) {
	struct xbee_modeData *data;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (!xbee->mode || !xbee->modeData) return XBEE_EINVAL;
	
	data = xbee->modeData;
	
	if (data->serialInfo.f) fclose(data->serialInfo.f);
	if (data->serialInfo.fd != -1) close(data->serialInfo.fd);
	if (data->serialInfo.device) free(data->serialInfo.device);
	free(xbee->modeData);
	
	return XBEE_ENONE;
}

/* ######################################################################### */

static struct xbee_modeConType conTypes[] = {
	{
		.name = NULL,
		.rxHandler = NULL,
		.txHandler = NULL,
	}
};

struct xbee_mode mode_xbee1 = {
	.name = "xbee1",
	
	.conTypes = conTypes,
	
	.init = init,
	.shutdown = shutdown,
	
	.rx_io = xbee_xbeeRxIo,
	.tx_io = xbee_xbeeTxIo,
	
	.thread = NULL,
};

