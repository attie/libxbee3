#ifndef __XBEE_QT_H
#define __XBEE_QT_H

#ifndef __cplusplus
#warning This header file is intended for use with C++
#else

#include <QObject>
#include <xbeep.h>

namespace libxbee {
	class ConQt: public QObject, public libxbee::ConCallback {
		Q_OBJECT

		public:
			explicit ConQt(XBee &parent, std::string type, struct xbee_conAddress *address = NULL);

		signals:
			void Rx(libxbee::Pkt *pkt); /* <-- you should connect to this one */

		private:
			virtual void xbee_conCallback(Pkt **pkt);
	};
};

#endif /* __cplusplus */

#endif /* __XBEE_QT_H */
