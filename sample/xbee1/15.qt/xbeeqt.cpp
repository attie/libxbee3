#include "xbeeqt.h"

libxbee::ConQt::ConQt(libxbee::XBee &parent, std::string type, struct xbee_conAddress *address): QObject(0), ConCallback(parent, type, address) {
	/* nothing */
}
void libxbee::ConQt::xbee_conCallback(libxbee::Pkt **pkt) {
	struct xbee_pkt *raw_pkt = (*pkt)->getHnd();
	*pkt = NULL;
	libxbee::Pkt *new_pkt = new libxbee::Pkt(raw_pkt);

	emit Rx(new_pkt);
}
