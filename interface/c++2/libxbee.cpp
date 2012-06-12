#include <stdlib.h>
#include <stdarg.h>
#include <iostream>

#include "libxbee.h"

/* ========================================================================== */

std::list<libxbee::XBee*> libxbee::xbeeList;

std::list<std::string> libxbee::getModes(void) {
	xbee_err ret;
	char **modes;
	int i;
	std::list<std::string> modeList;
	
	if ((ret = xbee_modeGetList(&modes)) != XBEE_ENONE) throw(ret);
	for (i = 0; modes[i] != NULL; i++) {
		modeList.push_back(std::string(modes[i]));
	}
	free(modes);
	
	return modeList;
}

/* ========================================================================== */

libxbee::XBee::XBee(std::string mode) {
	xbee_err ret;
	
	if ((ret = xbee_setup(&xbee, mode.c_str())) != XBEE_ENONE) throw(ret);
	
	libxbee::xbeeList.push_back(this);
}
libxbee::XBee::XBee(std::string mode, std::string device, int baudrate) {
	xbee_err ret;
	
	if ((ret = xbee_setup(&xbee, mode.c_str(), device.c_str(), baudrate)) != XBEE_ENONE) throw(ret);
	
	libxbee::xbeeList.push_back(this);
}
libxbee::XBee::XBee(std::string mode, va_list ap) {
	xbee_err ret;
	
	if ((ret = xbee_vsetup(&xbee, mode.c_str(), ap)) != XBEE_ENONE) throw(ret);
	
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

std::list<std::string> libxbee::XBee::getConTypes(void) {
	xbee_err ret;
	char **types;
	int i;
	std::list<std::string> typeList;
	
	if ((ret = xbee_conGetTypes(xbee, &types)) != XBEE_ENONE) throw(ret);
	for (i = 0; types[i] != NULL; i++) {
		typeList.push_back(std::string(types[i]));
	}
	free(types);
	
	return typeList;
}

std::string libxbee::XBee::mode(void) {
	xbee_err ret;
	const char *mode;
	
	if ((ret = xbee_modeGet(getHnd(), &mode)) != XBEE_ENONE) throw(ret);
	
	return std::string(mode);
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

void libxbee::Con::xbee_conCallback(libxbee::Pkt **pkt) { }
void libxbee::Con::libxbee_callbackFunction(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	std::list<libxbee::XBee*>::iterator i;
	for (i = libxbee::xbeeList.begin(); i != libxbee::xbeeList.end(); i++) {
		if ((*i)->getHnd() == xbee) {
			libxbee::Con *c;
			if ((c = (*i)->conLocate(con)) == NULL) break;
			
			libxbee::Pkt *pktClass = new libxbee::Pkt(*pkt);
			
			c->xbee_conCallback(&pktClass);
			
			/* if they took the packet, then don't free/delete it */
			if (pktClass != NULL) {
				delete pktClass;
			}
			
			/* either way, libxbee doesn't need to free it, it was free'd just now or its the user's responsibility... */
			*pkt = NULL;
			return;
		}
	}
	std::cerr << "  1#[" << __FILE__ << ":" << __LINE__ << "] " << __FUNCTION__ << "(): A connection called back to the C++ interface, but it wasnt found...\n";
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

void libxbee::Con::purge(void) {
	xbee_err ret;
	
	if ((ret = xbee_conPurge(con)) != XBEE_ENONE) throw(ret);
}

void libxbee::Con::sleep(void) {
	setSleep(CON_SLEEP);
}
void libxbee::Con::snooze(void) {
	setSleep(CON_SNOOZE);
	std::cout << "Sent for snooze\n";
}
void libxbee::Con::wake(void) {
	setSleep(CON_AWAKE);
}
void libxbee::Con::setSleep(enum xbee_conSleepStates state) {
	xbee_err ret;
	
	if ((ret = xbee_conSleepSet(con, state)) != XBEE_ENONE) throw(ret);
}
enum xbee_conSleepStates libxbee::Con::getSleep(void) {
	xbee_err ret;
	enum xbee_conSleepStates state;
	
	if ((ret = xbee_conSleepGet(con, &state)) != XBEE_ENONE) throw(ret);
	
	return state;
}

void libxbee::Con::getSettings(struct xbee_conSettings *settings) {
	xbee_err ret;
	if (settings == NULL) throw(XBEE_EINVAL);
	
	if ((ret = xbee_conSettings(con, NULL, settings)) != XBEE_ENONE) throw(ret);
}
void libxbee::Con::setSettings(struct xbee_conSettings *settings) {
	xbee_err ret;
	if (settings == NULL) throw(XBEE_EINVAL);
	
	if ((ret = xbee_conSettings(con, settings, NULL)) != XBEE_ENONE) throw(ret);
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

void *libxbee::Pkt::getData(const char *key) {
	return getData(key, 0, 0);
}
void *libxbee::Pkt::getData(const char *key, int id) {
	return getData(key, id, 0);
}
void *libxbee::Pkt::getData(const char *key, int id, int index) {
	xbee_err ret;
	void *p;
	
	if ((ret = xbee_pktDataGet(pkt, key, id, index, &p)) != XBEE_ENONE) throw(ret);
	return p;
}

int libxbee::Pkt::getAnalog(int channel) {
	return getAnalog(channel, 0);
}
int libxbee::Pkt::getAnalog(int channel, int index) {
	xbee_err ret;
	int value;
	
	if ((ret = xbee_pktAnalogGet(pkt, channel, index, &value)) != XBEE_ENONE) throw(ret);
	return value;
}

bool libxbee::Pkt::getDigital(int channel) {
	return getDigital(channel, 0);
}
bool libxbee::Pkt::getDigital(int channel, int index) {
	xbee_err ret;
	int value;
	
	if ((ret = xbee_pktDigitalGet(pkt, channel, index, &value)) != XBEE_ENONE) throw(ret);
	return (bool)!!value;
}
