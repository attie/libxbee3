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
#include "io.h"


/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_s1_16bitIo_rx  = {
	.identifier = 0x83,
	.func = NULL,
};
const struct xbee_modeConType xbee_s1_16bitIo = {
	.name = "16-bit I/O",
	.rxHandler = &xbee_s1_16bitIo_rx,
	.txHandler = NULL,
};

/* ######################################################################### */

const struct xbee_modeDataHandlerRx xbee_s1_64bitIo_rx  = {
	.identifier = 0x82,
	.func = NULL,
};
const struct xbee_modeConType xbee_s1_64bitIo = {
	.name = "64-bit I/O",
	.rxHandler = &xbee_s1_64bitIo_rx,
	.txHandler = NULL,
};
