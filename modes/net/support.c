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
#include "../../log.h"
#include "mode.h"
#include "support.h"

xbee_err xbee_netSupport_conNew(struct xbee *xbee, struct xbee_interface *interface, struct xbee_modeConType *conType, struct xbee_conAddress *address) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_netSupport_conValidate(struct xbee_con *con) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_netSupport_conSleepSet(struct xbee_con *con, enum xbee_conSleepStates state) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_netSupport_conSleepGet(struct xbee_con *con) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_netSupport_conSettings(struct xbee_con *con, struct xbee_conSettings *newSettings) {
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_netSupport_conEnd(struct xbee_con *con) {
	return XBEE_ENOTIMPLEMENTED;
}
