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
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with libxbee. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <list>
#include <iomanip>

#include <xbeep.h>

class remotenode {
	public:
		explicit remotenode(std::string name, uint16_t addr16, uint64_t addr64):
			name(name),
			addr16(addr16),
			addr64(addr64) { };
		explicit remotenode(std::vector<unsigned char> nd_payload);

		std::string getName(void)     const { return name;                        }
		uint16_t    getAddr16(void)   const { return addr16;                      }
		uint64_t    getAddr64(void)   const { return addr64;                      }
		uint32_t    getAddr64Hi(void) const { return (addr64 >> 32) & 0xFFFFFFFF; }
		uint32_t    getAddr64Lo(void) const { return addr64 & 0xFFFFFFFF;         }

	private:
		std::string name;
		uint16_t addr16;
		uint64_t addr64;
};

remotenode::remotenode(std::vector<unsigned char> nd_payload) {
	int i = 0;

	this->addr16  = (nd_payload[i + 0] << 8) & 0xFF00;
	this->addr16 |= (nd_payload[i + 1] << 0) & 0x00FF;
	i += 2;

	this->addr64 = nd_payload[i + 0] & 0xFF;
	this->addr64 = ((this->addr64 << 8) & ~0xFF) | (nd_payload[i + 1] & 0xFF);
	this->addr64 = ((this->addr64 << 8) & ~0xFF) | (nd_payload[i + 2] & 0xFF);
	this->addr64 = ((this->addr64 << 8) & ~0xFF) | (nd_payload[i + 3] & 0xFF);
	this->addr64 = ((this->addr64 << 8) & ~0xFF) | (nd_payload[i + 4] & 0xFF);
	this->addr64 = ((this->addr64 << 8) & ~0xFF) | (nd_payload[i + 5] & 0xFF);
	this->addr64 = ((this->addr64 << 8) & ~0xFF) | (nd_payload[i + 6] & 0xFF);
	this->addr64 = ((this->addr64 << 8) & ~0xFF) | (nd_payload[i + 7] & 0xFF);
	i += 8;

	i += 1;

	this->name = "";
	for (; i < nd_payload.size(); i++) {
		this->name += char(nd_payload[i]);
	}
}

/* ========================================================================== */

class atcon: public libxbee::ConCallback {
	public:
		explicit atcon(libxbee::XBee &parent, std::string type, struct xbee_conAddress *address = NULL):
			libxbee::ConCallback(parent, type, address),
			node_detect_complete(true) { };

		void xbee_conCallback(libxbee::Pkt **pkt);

		void start_node_detect(void);
		bool node_detect_complete;
		std::list<remotenode> node_list;
};

void atcon::xbee_conCallback(libxbee::Pkt **pkt) {
	if (!this->node_detect_complete) {
		if ((*pkt)->getATCommand() != "ND") {
			/* print an error if we thought we were in the middle of an 'ND' command... */
			std::cout << "Early exit of Node Detect...\n";
			this->node_detect_complete = true;
		}
	}

	if (this->node_detect_complete) {
		/* don't print anything if we're not interested */
		return;
	}

	std::vector<unsigned char> data = (*pkt)->getVector();

	if (data.size() == 0) {
		/* an AT response, with zero data length indicates that the scan is complete */
		std::cout << "Scan Complete!\n";
		this->node_detect_complete = true;
		return;
	}

	if (data.size() < 11) {
		/* ensure that we have enough data */
		std::cout << "Received small packet...\n";
		return;
	}

	/* push a new remotenode object into our list */
	this->node_list.push_back(remotenode(data));
}

void atcon::start_node_detect(void) {
	/* clear the list of it's previous results */
	this->node_list.clear();

	/* mark the in-progress flag */
	this->node_detect_complete = false;

	/* and execute the 'ND' command */
	try {
		*this << "ND";
	} catch (xbee_err err) {
		/* we expect the ND command to return -17 / XBEE_ETIMEOUT because it's a long running operation */
		if (err != -17) throw;
	}
}

/* ========================================================================== */

int main(int argc, char *argv[]) {
	int i;

	try {
		/* setup libxbee */
		libxbee::XBee xbee("xbee1", "/dev/ttyUSB0", 57600);
		
		/* make a connection */
		atcon con(xbee, "Local AT"); /* AT connection with a callback handler */

		/* start node detect */
		con.start_node_detect();

		/* lazy-wait for it to finish */
		while (!con.node_detect_complete) {
			usleep(100000);
		}

		/* print out a list of nodes */
		for (std::list<remotenode>::iterator n = con.node_list.begin(); n != con.node_list.end(); n++) {
			/* it's just easier to print nice-looking output using printf()... */
			printf("Node: %-20s  0x%04X  0x%08X 0x%08X\n",
			       n->getName().c_str(), n->getAddr16(), n->getAddr64Hi(), n->getAddr64Lo());
		}

	} catch (xbee_err err) {
		std::cout << "Error " << err << "\n";
	}
	
	return 0;
}
