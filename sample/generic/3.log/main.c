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
