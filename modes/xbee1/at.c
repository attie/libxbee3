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

#include "../../internal.h"
#include "../../xbee_int.h"
#include "../../mode.h"
#include "../common.h"
#include "at.h"


/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_s1_localAt_rx  = {
	.identifier = 0x88,
	.func = NULL,
};
const struct xbee_modeDataHandlerTx xbee_s1_localAt_tx  = {
	.identifier = 0x08,
	.func = NULL,
};
const struct xbee_modeConType xbee_s1_localAt = {
	.name = "Local AT",
	.allowFrameId = 1,
	.useTimeout = 0,
	.rxHandler = &xbee_s1_localAt_rx,
	.txHandler = &xbee_s1_localAt_tx,
};

/* ######################################################################### */

const struct xbee_modeDataHandlerTx xbee_s1_localAtQueued_tx  = {
	.identifier = 0x09,
	.func = NULL,
};
const struct xbee_modeConType xbee_s1_localAtQueued = {
	.name = "Local AT (queued)",
	.allowFrameId = 1,
	.useTimeout = 0,
	.rxHandler = NULL,
	.txHandler = &xbee_s1_localAtQueued_tx,
};

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_s1_remoteAt_rx  = {
	.identifier = 0x97,
	.func = NULL,
};
const struct xbee_modeDataHandlerTx xbee_s1_remoteAt_tx  = {
	.identifier = 0x17,
	.func = NULL,
};
const struct xbee_modeConType xbee_s1_remoteAt = {
	.name = "Remote AT",
	.allowFrameId = 1,
	.useTimeout = 0,
	.rxHandler = &xbee_s1_remoteAt_rx,
	.txHandler = &xbee_s1_remoteAt_tx,
};
