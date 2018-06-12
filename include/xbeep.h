#ifndef _XBEEP_H
#define _XBEEP_H

/*
	libxbee - a C/C++ library to aid the use of Digi's XBee wireless modules
	          running in API mode.

	Copyright (C) 2009 onwards  Attie Grande (attie@attie.co.uk)

	libxbee is free software: you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libxbee is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __cplusplus
#warning This header file is intended for use with C++
#else

#include <string>
#include <list>
#include <vector>
#include <stdarg.h>

#include <xbee.h>

#ifndef EXPORT
#define EXPORT
#define XBEE_EXPORT_DEFINED
#endif

namespace libxbee {
	class XBee;
	class Con;
	class ConCallback;
	class Pkt;
	
	extern std::list<XBee*> xbeeList;
	
	std::list<std::string> getModes(void);

	class xbee_etx {
		public:
			EXPORT explicit xbee_etx(xbee_err ret, unsigned char retVal): ret(ret), retVal(retVal) { };
			const xbee_err ret;
			const unsigned char retVal;
	};
	
	class EXPORT XBee {
		public:
			EXPORT explicit XBee(const std::string &mode);
			EXPORT explicit XBee(const std::string &mode, const std::string &device, int baudrate);
			EXPORT explicit XBee(const std::string &mode, va_list ap);
			EXPORT ~XBee(void);
			
		private:
			struct xbee *xbee;
			std::list<Con*> conList;
			
		public:
			EXPORT struct xbee *getHnd(void);
			EXPORT const struct xbee *getHnd(void) const;
			EXPORT void conRegister(Con *con);
			EXPORT void conUnregister(Con *con);
			EXPORT const Con *conLocate(const struct xbee_con *con) const;
			EXPORT Con *conLocate(const struct xbee_con *con);
			EXPORT std::list<std::string> getConTypes(void) const;
			
			EXPORT std::string mode(void) const;
			
			EXPORT void setLogTarget(FILE *f) const;
			EXPORT void setLogLevel(int level) const;
			EXPORT int getLogLevel(void) const;
	};
	
	class EXPORT Con {
		public:
			EXPORT explicit Con(XBee &parent, const std::string &type, struct xbee_conAddress *address = NULL);
			EXPORT ~Con(void);
			
			EXPORT unsigned char operator<< (const std::string &data) const;
			EXPORT unsigned char operator<< (const std::vector<unsigned char> &data) const;
			EXPORT unsigned char operator<< (const std::vector<char> &data) const;
			EXPORT void operator>> (Pkt &pkt) const;
			EXPORT void operator>> (std::string &data) const;
			EXPORT void operator>> (std::vector<unsigned char> &data) const;
			EXPORT void operator>> (std::vector<char> &data) const;
			
		private:
			friend class XBee;
			friend class ConCallback;
			
			XBee &parent;
			struct xbee *xbee;
			struct xbee_con *con;
			virtual void xbee_conCallback(Pkt **pkt);
			
		public:
			EXPORT const struct xbee_con *getHnd(void) const;
			EXPORT struct xbee_con *getHnd(void);
			EXPORT unsigned char Tx(const std::string &data) const;
			EXPORT unsigned char Tx(const std::vector<unsigned char> &data) const;
			EXPORT unsigned char Tx(const std::vector<char> &data) const;
			EXPORT unsigned char Tx(const unsigned char *buf, int len) const;
			EXPORT unsigned char Tx(unsigned char *frameId, const std::string &data) const;
			EXPORT unsigned char Tx(unsigned char *frameId, const std::vector<unsigned char> &data) const;
			EXPORT unsigned char Tx(unsigned char *frameId, const std::vector<char> &data) const;
			EXPORT unsigned char Tx(unsigned char *frameId, const unsigned char *buf, int len) const;
			EXPORT void Rx(Pkt &pkt, int *remainingPackets = NULL) const;
			EXPORT int RxAvailable(void) const;
			
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
			static void libxbee_callbackFunction(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data);
			
		public:
			EXPORT explicit ConCallback(XBee &parent, std::string type, struct xbee_conAddress *address = NULL);
	};
	
	class EXPORT Pkt {
		public:
			EXPORT explicit Pkt(struct xbee_pkt *pkt = NULL);
			EXPORT ~Pkt(void);
			
			EXPORT unsigned char operator[] (int index);
			EXPORT void operator<< (const Con &con);
			EXPORT void operator>> (std::string &data) const;
			EXPORT void operator>> (std::vector<unsigned char> &data) const;
			EXPORT void operator>> (std::vector<char> &data) const;
			
		private:
			struct xbee_pkt *pkt;
			
		public:
			EXPORT const struct xbee_pkt *getHnd(void) const;
			EXPORT struct xbee_pkt *getHnd(void);
			EXPORT void setHnd(struct xbee_pkt *pkt);
			
			/* when calling this function, YOU become responsible for freeing the previously held packet */
			EXPORT struct xbee_pkt *dropHnd(void);
			
			EXPORT int size(void) const;
			
			EXPORT std::string getData(void) const;
			EXPORT std::vector<unsigned char> getVector(void) const;
			EXPORT std::vector<char> getVector2(void) const;

			/* use these three with care... */
			EXPORT void *getData(const char *key) const ;
			EXPORT void *getData(const char *key, int id) const;
			EXPORT void *getData(const char *key, int id, int index) const;
			
			EXPORT std::string getATCommand(void) const;
			
			EXPORT int getAnalog(int channel) const;
			EXPORT int getAnalog(int channel, int index) const;
			EXPORT bool getDigital(int channel) const;
			EXPORT bool getDigital(int channel, int index) const;

			EXPORT unsigned char getRssi(void) const;
	};
};

#ifdef XBEE_EXPORT_DEFINED
#undef EXPORT
#undef XBEE_EXPORT_DEFINED
#endif

#endif /* __cplusplus */

#endif /* _XBEEP_H */
