#ifndef __XBEE_CPP_H
#define __XBEE_CPP_H

#include <string>
#include <list>
#include <xbee.h>

namespace libxbee {
	class XBee;
	class Con;
	class ConCallback;
	
	extern std::list<XBee*> xbeeList;

	class XBee {
		public:
			explicit XBee(std::string mode);
			explicit XBee(std::string mode, std::string device, int baudrate);
			~XBee(void);
			
		private:
			struct xbee *xbee;
			std::list<Con*> conList;
			std::list<ConCallback*> conCallbackList;
			
		public:
			struct xbee *getHnd(void);
			void conRegister(Con *con);
			void conUnregister(Con *con);
			Con *conLocate(struct xbee_con *con);

			void conRegister(ConCallback *con);
			void conUnregister(ConCallback *con);
			ConCallback *conCallbackLocate(struct xbee_con *con);
	};

	class Con {
		public:
			explicit Con(XBee &parent, std::string type);
			~Con(void);
			
		private:
			XBee &parent;
			struct xbee *xbee;
			struct xbee_con *con;
			virtual void xbee_conCallback(struct xbee_pkt **pkt, void **data);
			
		public:
			struct xbee_con *getHnd(void);
			unsigned char Tx(std::string data);
	};

	class ConCallback: public Con {
		private:
			XBee &parent;
			static void libxbee_callbackFunction(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data);
			virtual void xbee_conCallback(struct xbee_pkt **pkt, void **data) = 0;
			
		public:
			explicit ConCallback(XBee &parent, std::string type);
			~ConCallback(void);
	};

};

#endif /* __XBEE_CPP_H */
