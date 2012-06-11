#include <iostream>

#include "libxbee.h"

/* ========================================================================== */

class myConnection: public libxbee::ConCallback {
	public:
		explicit myConnection(libxbee::XBee &parent, std::string type);
		void xbee_conCallback(struct xbee_pkt **pkt, void **data);
};

myConnection::myConnection(libxbee::XBee &parent, std::string type): libxbee::ConCallback(parent, type) { }

void myConnection::xbee_conCallback(struct xbee_pkt **pkt, void **data) {
	std::cout << "Callback!!\n";
	int i;
	for (i = 0; i < (*pkt)->dataLen; i++) {
		std::cout << (*pkt)->data[i];
	}
	std::cout << "\n";
}

/* ========================================================================== */

int main(int argc, char *argv[]) {
	try {
		/* setup libxbee */
		libxbee::XBee xbee("xbee1", "/dev/ttyUSB0", 57600);

		/* make a connection */
		myConnection con(xbee, "Local AT");

		/* send data */
		con.Tx("NI");

		sleep(1);
	} catch (xbee_err err) {
		std::cout << "Error " << err << "\n";
	}
	
	return 0;
}
