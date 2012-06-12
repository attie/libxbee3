#include <iostream>
#include <string.h>

#include "libxbee.h"

#define LOCAL_CONNECTION
#define USE_CALLBACKS

#ifdef USE_CALLBACKS

/* ========================================================================== */

class myConnection: public libxbee::ConCallback {
	public:
		explicit myConnection(libxbee::XBee &parent, std::string type, struct xbee_conAddress *address = NULL): libxbee::ConCallback(parent, type, address) {};
		void xbee_conCallback(struct xbee_pkt **pkt, void **data);
};

void myConnection::xbee_conCallback(struct xbee_pkt **pkt, void **data) {
	std::cout << "Callback!!\n";
	int i;
	for (i = 0; i < (*pkt)->dataLen; i++) {
		std::cout << (*pkt)->data[i];
	}
	std::cout << "\n";
}
#endif /* USE_CALLBACKS */

/* ========================================================================== */

int main(int argc, char *argv[]) {
	int i;

	try {

		/* setup libxbee */
		libxbee::XBee xbee("xbee1", "/dev/ttyUSB0", 57600);


		/* make a connection */
#ifdef LOCAL_CONNECTION
  #ifdef USE_CALLBACKS
		myConnection con(xbee, "Local AT"); /* with a callback */
  #else
		libxbee::Con con(xbee, "Local AT"); /* without a callback */
  #endif
#else /* LOCAL_CONNECTION */
		struct xbee_conAddress addr;
		memset(&addr, 0, sizeof(addr));
		addr.addr64_enabled = 1;
		addr.addr64[0] = 0x00;
		addr.addr64[1] = 0x13;
		addr.addr64[2] = 0xA2;
		addr.addr64[3] = 0x00;
		addr.addr64[4] = 0x40;
		addr.addr64[5] = 0x33;
		addr.addr64[6] = 0xCA;
		addr.addr64[7] = 0xCB;
  #ifdef USE_CALLBACKS
		myConnection con(xbee, "Remote AT", &addr); /* with a callback */
  #else
		libxbee::Con con(xbee, "Remote AT", &addr); /* without a callback */
  #endif
#endif /* LOCAL_CONNECTION */


		/* send data */
		//con.Tx("NI"); /* like this */
		con << "NI";    /* or like this */
		sleep(1);


#ifndef USE_CALLBACKS
		libxbee::Pkt pkt;

		try {
			//con >> pkt; /* like this */
			pkt << con;   /* or this */
		} catch (xbee_err err) {
			std::cout << "Error on Rx! " << err << "\n";
			return 1;
		}

		try {
			std::cout << "Packet length: " << pkt.size() << "\n";
			for (i = 0; i < pkt.size(); i++) {
				std::cout << "  " << i << " " << pkt[i] << "\n";
			}
		} catch (xbee_err err) {
			std::cout << "Error accessing packet! " << err << "\n";
			return 1;
		}

		sleep(1);
#endif /* !USE_CALLBACKS */
	} catch (xbee_err err) {
		std::cout << "Error " << err << "\n";
	}
	
	return 0;
}
