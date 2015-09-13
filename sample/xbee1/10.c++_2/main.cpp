#include <iostream>
#include <string.h>
#include <unistd.h>

#include <xbeep.h>

class myConnection: public libxbee::ConCallback {
	public:
		explicit myConnection(libxbee::XBee &parent, std::string type, struct xbee_conAddress *address = NULL): libxbee::ConCallback(parent, type, address) {};
		void xbee_conCallback(libxbee::Pkt **pkt);
		std::string myData;
};

void myConnection::xbee_conCallback(libxbee::Pkt **pkt) {
	std::cout << "Callback!!\n";
	int i;
	for (i = 0; i < (*pkt)->size(); i++) {
		std::cout << (**pkt)[i];
	}
	std::cout << "\n";

	/* if you want to keep the packet, then you MUST do the following:
	      libxbee::Pkt *myhandle = *pkt;
	      *pkt = NULL;
	   and then later, you MUST delete the packet to free up the memory:
	      delete myhandle;

	   if you do not want to keep the packet, then just leave everything as-is, and it will be free'd for you */
}

/* ========================================================================== */

int main(int argc, char *argv[]) {
	int i;

	try {

		/* setup libxbee */
		libxbee::XBee xbee("xbee1", "/dev/ttyUSB0", 57600);

		{
			/* make a local connection */
			myConnection local_con(xbee, "Local AT"); /* with a callback */

			struct xbee_conAddress addr;
			memset(&addr, 0, sizeof(addr));
			addr.addr64_enabled = 1;
			addr.addr64[0] = 0x00;
			addr.addr64[1] = 0x13;
			addr.addr64[2] = 0xA2;
			addr.addr64[3] = 0x00;
			addr.addr64[4] = 0x40;
			addr.addr64[5] = 0x3C;
			addr.addr64[6] = 0xB2;
			addr.addr64[7] = 0x6D;

			/* make a remote connection */
			myConnection remote_con(xbee, "Remote AT", &addr); /* with a callback */

			/* send data */
			local_con << "NI";
			remote_con << "NI";
			
			/* to send AT parameters:
			if for example you want to set DIO4 pin to high (parameter=5)
			xbee_conTx(remote_con.getHnd(), NULL, "D4%c", 0x05);
			*/
			
			
			/* wait a bit... */
			usleep(1000000);

			/* the connections are terminated when they go out of scope... */
		}

		usleep(1000000);

		/* the libxbee instance is shutdown when it goes out of scope... */
	} catch (xbee_err err) {
		std::cout << "Error " << err << "\n";
	}

	usleep(5000000);

	return 0;
}
