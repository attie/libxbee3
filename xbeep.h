#ifndef __XBEE_CPP_H
#define __XBEE_CPP_H

#include <string>
#include <list>
#include <stdarg.h>

#include <xbee.h>

namespace libxbee {
	class XBee;
	class Con;
	class ConCallback;
	class Pkt;
	
	extern std::list<XBee*> xbeeList;
	
	std::list<std::string> getModes(void);
	
	class EXPORT XBee {
		public:
			EXPORT explicit XBee(std::string mode);
			EXPORT explicit XBee(std::string mode, std::string device, int baudrate);
			EXPORT explicit XBee(std::string mode, va_list ap);
			EXPORT ~XBee(void);
			
		private:
			struct xbee *xbee;
			std::list<Con*> conList;
			
		public:
			EXPORT struct xbee *getHnd(void);
			EXPORT void conRegister(Con *con);
			EXPORT void conUnregister(Con *con);
			EXPORT Con *conLocate(struct xbee_con *con);
			EXPORT std::list<std::string> getConTypes(void);
			
			EXPORT std::string mode(void);
			
			EXPORT void setLogTarget(FILE *f);
			EXPORT void setLogLevel(int level);
			EXPORT int getLogLevel(void);
	};
	
	class EXPORT Con {
		public:
			EXPORT explicit Con(XBee &parent, std::string type, struct xbee_conAddress *address = NULL);
			EXPORT ~Con(void);
			
			EXPORT unsigned char operator<< (std::string data);
			EXPORT void operator>> (Pkt &pkt);
			
		private:
			friend class ConCallback;
			
			XBee &parent;
			struct xbee *xbee;
			struct xbee_con *con;
			static void libxbee_callbackFunction(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data);
			virtual void xbee_conCallback(Pkt **pkt);
			
		public:
			EXPORT struct xbee_con *getHnd(void);
			EXPORT unsigned char Tx(std::string data);
			EXPORT unsigned char Tx(const unsigned char *buf, int len);
			EXPORT void Rx(Pkt &pkt, int *remainingPackets = NULL);
			
			EXPORT void purge(void);
			
			EXPORT void sleep(void);
			EXPORT void snooze(void);
			EXPORT void wake(void);
			EXPORT void setSleep(enum xbee_conSleepStates state);
			EXPORT enum xbee_conSleepStates getSleep(void);
			
			EXPORT void getSettings(struct xbee_conSettings *settings);
			EXPORT void setSettings(struct xbee_conSettings *settings);
	};
	
	class EXPORT ConCallback: public Con {
		private:
			XBee &parent;
			virtual void xbee_conCallback(Pkt **pkt) = 0;
			
		public:
			EXPORT explicit ConCallback(XBee &parent, std::string type, struct xbee_conAddress *address = NULL);
	};
	
	class EXPORT Pkt {
		public:
			EXPORT explicit Pkt(struct xbee_pkt *pkt = NULL);
			EXPORT ~Pkt(void);
			
			EXPORT unsigned char operator[] (int index);
			EXPORT void operator<< (Con &con);
			
		private:
			struct xbee_pkt *pkt;
			
		public:
			EXPORT struct xbee_pkt *getHnd(void);
			EXPORT void setHnd(struct xbee_pkt *pkt);
			EXPORT int size(void);
			
			/* use these two with care... */
			EXPORT void *getData(const char *key);
			EXPORT void *getData(const char *key, int id);
			EXPORT void *getData(const char *key, int id, int index);
			
			EXPORT int getAnalog(int channel);
			EXPORT int getAnalog(int channel, int index);
			EXPORT bool getDigital(int channel);
			EXPORT bool getDigital(int channel, int index);
	};
};

#endif /* __XBEE_CPP_H */
