/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009	Attie Grande (attie@attie.co.uk)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.	If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include <xbee.h>

int main(void) {
	struct xbee *xbee;

	if (xbee_setup(&xbee, "xbee1", "/dev/ttyUSB0", 57600) != XBEE_ENONE) {
		printf("xbee_setup() returned an error...\n");
		return 1;
	}

	xbee_log(xbee, -1, "Hello World!");

	return 0;
}
