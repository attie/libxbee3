#include <iostream>
#include <xbee.h>

using namespace std;

int main(int argc, char *argv[]) {
	libxbee *xbee = NULL;
	int conid;

	try {
		int conId;
		int retVal;
		int i;
		struct xbee_pkt *pkt;
		
		xbee = new libxbee("xbee1", "/dev/ttyUSB0", 57600);
		
		conId = xbee->conNew("Local AT");
		
		if ((retVal = xbee->conTx(conId, "NI")) != 0) {
			printf("conTx(\"NI\"): %d\n", retVal);
			return 1;
		}
		
		if ((pkt = xbee->conRx(conId)) == NULL) {
			printf("conRx(): returned NULL\n");
			return 1;
		}
		
		for (i = 0; i < pkt->dataLen; i++) {
			printf("  %3d: 0x%02X '%c'\n", i, pkt->data[i], pkt->data[i]);
		}
		
		xbee_pktFree(pkt);
		
	} catch (xbee_err ret) {
		printf("libxbee(): %d - %s\n", ret, xbee_errorToStr(ret));
	}
	
	if (xbee) delete xbee;
	
	return 0;
}
