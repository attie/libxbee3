#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xbee.h>

int main(void) {
	struct xbee *xbee;
	char **conTypes;
	xbee_err ret;
	int i;

	if ((ret = xbee_setup(&xbee, "xbee1", "/dev/ttyUSB0", 57600)) != XBEE_ENONE) {
		printf("ret: %d (%s)\n", ret, xbee_errorToStr(ret));
		return ret;
	}

	if ((ret = xbee_conGetTypes(xbee, &conTypes)) != XBEE_ENONE) {
		printf("ret: %d (%s)\n", ret, xbee_errorToStr(ret));
		return ret;
	}
	
	for (i = 0; conTypes[i]; i++) {
		printf("connection type %d - %s\n", i, conTypes[i]);
	}
	
	free(conTypes);

	xbee_shutdown(xbee);

	return 0;
}
