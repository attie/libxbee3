#include <iostream>

#include "libxbee.h"

/* ========================================================================== */

std::list<libxbee::XBee*> libxbee::xbeeList;

libxbee::XBee::XBee(std::string mode) {
	xbee_err ret;
	
	if ((ret = xbee_setup(&xbee, mode.c_str())) != XBEE_ENONE) throw(ret);
}
libxbee::XBee::XBee(std::string mode, std::string device, int baudrate) {
	xbee_err ret;
	
	if ((ret = xbee_setup(&xbee, mode.c_str(), device.c_str(), baudrate)) != XBEE_ENONE) throw(ret);
	
	libxbee::xbeeList.push_back(this);
}

libxbee::XBee::~XBee(void) {
	xbee_shutdown(xbee);
	libxbee::xbeeList.remove(this);
}

struct xbee *libxbee::XBee::getHnd(void) {
	return xbee;
}
void libxbee::XBee::conRegister(Con *con) {
	xbee_err ret;
	if ((ret = xbee_conValidate(con->getHnd())) != XBEE_ENONE) throw(ret);
	conList.push_back(con);
	conList.unique();
}
void libxbee::XBee::conUnregister(Con *con) {
	conList.remove(con);
}
libxbee::Con *libxbee::XBee::conLocate(struct xbee_con *con) {
	std::list<libxbee::Con*>::iterator i;
	for (i = conList.begin(); i != conList.end(); i++) {
		if ((*i)->getHnd() == con) return (*i);
	}
	return NULL;
}

/* ========================================================================== */

libxbee::Con::Con(libxbee::XBee &parent, std::string type, struct xbee_conAddress *address) : parent(parent) {
	xbee_err ret;
	
	if ((xbee = parent.getHnd()) == NULL) throw(XBEE_EINVAL);
	
	if ((ret = xbee_conNew(xbee, &con, type.c_str(), address)) != XBEE_ENONE) throw(ret);
	if ((ret = xbee_conDataSet(con, (void*)this, NULL)) != XBEE_ENONE) {
		xbee_conEnd(con);
		throw(ret);
	}
	try {
		parent.conRegister(this);
	} catch (xbee_err ret) {
		xbee_conEnd(con);
		throw(ret);
	}
}
libxbee::Con::~Con(void) {
	parent.conUnregister(this);
	xbee_conEnd(con);
}

unsigned char libxbee::Con::operator<< (std::string data) {
	return Tx(data);
}
void libxbee::Con::operator>> (Pkt &pkt) {
	return Rx(pkt);
}

struct xbee_con *libxbee::Con::getHnd(void) {
	return con;
}

unsigned char libxbee::Con::Tx(std::string data) {
	unsigned char retVal;
	xbee_err ret;
	
	if ((ret = xbee_connTx(con, &retVal, (const unsigned char*)data.c_str(), data.size())) != XBEE_ENONE) throw(ret);
	
	return retVal;
}
unsigned char libxbee::Con::Tx(const unsigned char *buf, int len) {
	unsigned char retVal;
	xbee_err ret;
	
	if ((ret = xbee_connTx(con, &retVal, buf, len)) != XBEE_ENONE) throw(ret);
	
	return retVal;
}

void libxbee::Con::Rx(Pkt &pkt, int *remainingPackets) {
	struct xbee_pkt *raw_pkt;
	xbee_err ret;
	
	if ((ret = xbee_conRx(con, &raw_pkt, remainingPackets)) != XBEE_ENONE) throw(ret);
	
	pkt.setHnd(raw_pkt);
}

void libxbee::Con::xbee_conCallback(struct xbee_pkt **pkt, void **data) { }
void libxbee::Con::libxbee_callbackFunction(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	std::list<libxbee::XBee*>::iterator i;
	for (i = libxbee::xbeeList.begin(); i != libxbee::xbeeList.end(); i++) {
		if ((*i)->getHnd() == xbee) {
			libxbee::Con *c;
			if ((c = (*i)->conLocate(con)) == NULL) break;
			c->xbee_conCallback(pkt, data);
			return;
		}
	}
	std::cerr << "  1#[" << __FILE__ << ":" << __LINE__ << "] " << __FUNCTION__ << "(): A connection called back to the C++ interface, but it wasnt found...\n";
}

/* ========================================================================== */

libxbee::ConCallback::ConCallback(XBee &parent, std::string type, struct xbee_conAddress *address) : parent(parent), Con(parent, type, address) {
	xbee_err ret;
	
	if ((ret = xbee_conCallbackSet(this->getHnd(), libxbee::Con::libxbee_callbackFunction, NULL)) != XBEE_ENONE) throw(ret);
}

/* ========================================================================== */

libxbee::Pkt::Pkt(struct xbee_pkt *pkt) {
	this->pkt = pkt;
}
libxbee::Pkt::~Pkt(void) {
	if (pkt != NULL) xbee_pktFree(pkt);
}

unsigned char libxbee::Pkt::operator[] (int index) {
	if (pkt == NULL) throw(XBEE_EINVAL);
	if (index > size()) throw(XBEE_ERANGE);
	return pkt->data[index];
}
void libxbee::Pkt::operator<< (Con &con) {
	con.Rx(*this);
}

int libxbee::Pkt::size(void) {
	if (pkt == NULL) throw(XBEE_EINVAL);
	return pkt->dataLen;
}

struct xbee_pkt *libxbee::Pkt::getHnd(void) {
	return pkt;
}
void libxbee::Pkt::setHnd(struct xbee_pkt *pkt) {
	if (this->pkt) xbee_pktFree(this->pkt);
	this->pkt = pkt;
}
