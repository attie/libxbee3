/*
	libxbee - a C/C++ library to aid the use of Digi's XBee wireless modules
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <xbee.h>

void redirect_stderr(void) {
	int fd = open("/dev/null", O_RDWR);
	dup2(fileno(stderr), fd);
}

int main(void) {
	char **modes;
	int i;

	if (xbee_modeGetList(&modes) != XBEE_ENONE) {
		printf("xbee_modeGetList() returned an error...\n");
		return 1;
	}

	for (i = 0; modes[i]; i++) {
		struct xbee *xbee;
		char **types;
		int o;

		printf("mode %d - %s\n", i, modes[i]);

		if (!strcmp(modes[i], "net")) continue;
		if (!strcmp(modes[i], "debug")) continue;

		if (xbee_setup(&xbee, "debug", modes[i]) != XBEE_ENONE) {
			printf("   couldn't startup libxbee...\n");
			continue;
		}

		if (xbee_conGetTypes(xbee, &types) != XBEE_ENONE) {
			printf("   couldn't get a list of con types...\n");
			continue;
		}

		for (o = 0; types[o]; o++) {
			printf("  type %d - %s\n", o, types[o]);
		}

		free(types);

		xbee_shutdown(xbee);

	}

	free(modes);

	return 0;
}
