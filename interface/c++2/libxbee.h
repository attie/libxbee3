#ifndef __XBEE_CPP_H
#define __XBEE_CPP_H

#include <string>
#include <list>
#include <xbee.h>

namespace libxbee {
	class XBee;
	class Con;
	class ConCallback;
	class Pkt;
	
	extern std::list<XBee*> xbeeList;
	
	class XBee {
		public:
			explicit XBee(std::string mode);
			explicit XBee(std::string mode, std::string device, int baudrate);
			~XBee(void);
			
		private:
			struct xbee *xbee;
			std::list<Con*> conList;
			
		public:
			struct xbee *getHnd(void);
			void conRegister(Con *con);
			void conUnregister(Con *con);
			Con *conLocate(struct xbee_con *con);
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
			virtual void xbee_conCallback(struct xbee_pkt **pkt, void **data);
			
		public:
			struct xbee_con *getHnd(void);
			unsigned char Tx(std::string data);
			unsigned char Tx(const unsigned char *buf, int len);
			void Rx(Pkt &pkt, int *remainingPackets = NULL);
	};
	
	class ConCallback: public Con {
		private:
			XBee &parent;
			virtual void xbee_conCallback(struct xbee_pkt **pkt, void **data) = 0;
			
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
			
		private:
			struct xbee_pkt *pkt;
	};
};

#endif /* __XBEE_CPP_H */
