#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xbee.h>

int myClientFilter(struct xbee *xbee, char *remoteHost) {
	printf("Connection request from [%s]\n", remoteHost);
	return 0; /* return 0 - accept, anything else means deny  */
}

int main(void) {
	void *d;
	struct xbee *xbee;
	struct xbee_con *con;
	struct xbee_conAddress address;
	xbee_err ret;

	if ((ret = xbee_setup(&xbee, "xbee1", "/dev/ttyUSB0", 57600)) != XBEE_ENONE) {
		printf("ret: %d (%s)\n", ret, xbee_errorToStr(ret));
		return ret;
	}

	if ((ret = xbee_netStart(xbee, 27015, myClientFilter)) != XBEE_ENONE) {
		printf("ret: %d (%s)\n", ret, xbee_errorToStr(ret));
		return ret;
	}

	printf("Ready!... waiting for 30 secs\n");

	sleep(30);

	xbee_netStop(xbee);

	xbee_shutdown(xbee);

	return 0;
}
