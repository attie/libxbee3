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
	
	class XBee {
		public:
			explicit XBee(std::string mode);
			explicit XBee(std::string mode, std::string device, int baudrate);
			explicit XBee(std::string mode, va_list ap);
			~XBee(void);
			
		private:
			struct xbee *xbee;
			std::list<Con*> conList;
			
		public:
			struct xbee *getHnd(void);
			void conRegister(Con *con);
			void conUnregister(Con *con);
			Con *conLocate(struct xbee_con *con);
			std::list<std::string> getConTypes(void);
			
			std::string mode(void);
			
			void setLogTarget(FILE *f);
			void setLogLevel(int level);
			int getLogLevel(void);
	};
	
	class Con {
		public:
			explicit Con(XBee &parent, std::string type, struct xbee_conAddress *address = NULL);
			~Con(void);
			
			unsigned char operator<< (std::string data);
			void operator>> (Pkt &pkt);
			
		private:
			friend class ConCallback;
			
			XBee &parent;
			struct xbee *xbee;
			struct xbee_con *con;
			static void libxbee_callbackFunction(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data);
			virtual void xbee_conCallback(Pkt **pkt);
			
		public:
			struct xbee_con *getHnd(void);
			unsigned char Tx(std::string data);
			unsigned char Tx(const unsigned char *buf, int len);
			void Rx(Pkt &pkt, int *remainingPackets = NULL);
			
			void purge(void);
			
			void sleep(void);
			void snooze(void);
			void wake(void);
			void setSleep(enum xbee_conSleepStates state);
			enum xbee_conSleepStates getSleep(void);
			
			void getSettings(struct xbee_conSettings *settings);
			void setSettings(struct xbee_conSettings *settings);
	};
	
	class ConCallback: public Con {
		private:
			XBee &parent;
			virtual void xbee_conCallback(Pkt **pkt) = 0;
			
		public:
			explicit ConCallback(XBee &parent, std::string type, struct xbee_conAddress *address = NULL);
	};
	
	class Pkt {
		public:
			explicit Pkt(struct xbee_pkt *pkt = NULL);
			~Pkt(void);
			
			unsigned char operator[] (int index);
			void operator<< (Con &con);
			
			struct xbee_pkt *getHnd(void);
			void setHnd(struct xbee_pkt *pkt);
			int size(void);
			
			/* use these two with care... */
			void *getData(const char *key);
			void *getData(const char *key, int id);
			void *getData(const char *key, int id, int index);
			
			int getAnalog(int channel);
			int getAnalog(int channel, int index);
			bool getDigital(int channel);
			bool getDigital(int channel, int index);
			
		private:
			struct xbee_pkt *pkt;
	};
};

#endif /* __XBEE_CPP_H */
