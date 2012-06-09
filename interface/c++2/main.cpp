#include <iostream>

#include "libxbee.h"

class myConnection: public libxbee::ConCallback {
	public:
		explicit myConnection(libxbee::XBee &parent, std::string type);
		void xbee_conCallback(struct xbee_pkt *pkt, void **data);
};

myConnection::myConnection(libxbee::XBee &parent, std::string type): libxbee::ConCallback(parent, type) { }

void myConnection::xbee_conCallback(struct xbee_pkt *pkt, void **data) {
	std::cout << "Callback\n";
}

void a(libxbee::XBee &xbee) {
	libxbee::Con con(xbee, "Local AT");
	sleep(2);
	con.Tx("NI");
	sleep(2);
}
void b(libxbee::XBee &xbee) {
	libxbee::Con *con;
	
	con = new libxbee::Con(xbee, "Local AT");
	sleep(2);
	con->Tx("NI");
	sleep(2);
	delete con;
}
void c(libxbee::XBee &xbee) {
	myConnection con(xbee, "Local AT");
	sleep(2);
	con.Tx("NI");
	sleep(2);
}

int main(int argc, char *argv[]) {
	libxbee::XBee xbee("xbee1", "/dev/ttyUSB0", 57600);

	//a(xbee);
	//b(xbee);
	c(xbee);
	
	sleep(2);
	
	return 0;
}