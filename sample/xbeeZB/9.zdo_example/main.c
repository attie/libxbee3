/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <xbee.h>

#include "zdo.h"

void modemCB(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	if (!pkt || !*pkt || (*pkt)->dataLen != 1) return;
	printf("Modem Status: 0x%02X\n", (*pkt)->data[0]);
	switch ((*pkt)->data[0]) {
		case 0x00: printf("\tHardware Reset\n");                                     break;
		case 0x01: printf("\tWatchdog timer reset\n");                               break;
		case 0x02: printf("\tJoined network\n");                                     break;
		case 0x03: printf("\tDisassociated\n");                                      break;
		case 0x06: printf("\tCoordinator started\n");                                break;
		case 0x07: printf("\tNetwork security key was updated\n");                   break;
		case 0x0D: printf("\tVoltage supply limit exceeded\n");                      break;
		case 0x11: printf("\tModem configuration changed while join in progress\n"); break;
		default:
			if ((*pkt)->data[0] > 0x80) {
				printf("\tStack Error\n");
			} else {
				printf("\t** unknown **\n");
			}
	}
}

void zdoCB(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data) {
	int i;
	unsigned char *sStart;
	int entriesRemain;
	int dataRemain;
	
	if ((*pkt)->dataLen < 2) return;
	if ((*pkt)->data[1] != 0x00) return;
	if ((*pkt)->dataLen < 5) return;
	
	entriesRemain = (*pkt)->data[4] & 0xFF;
	sStart = & (*pkt)->data[5];
	dataRemain = (*pkt)->dataLen - 5;
	while (dataRemain >= 22 && entriesRemain > 0) {
		printf(">--- Node\n");
		printf("   - PAN Address: 0x%02X%02X%02X%02X 0x%02X%02X%02X%02X\n",
		       sStart[7], sStart[6], sStart[5], sStart[4], 
		       sStart[3], sStart[2], sStart[1], sStart[0]);
		printf("   - 64-bit Addr: 0x%02X%02X%02X%02X 0x%02X%02X%02X%02X\n",
		       sStart[15], sStart[14], sStart[13], sStart[12], 
		       sStart[11], sStart[10], sStart[ 9], sStart[ 8]);
		printf("   - 16-bit Addr: 0x%02X%02X\n",
		       sStart[17], sStart[16]);
		switch ((sStart[18] >> 0) & 0x03) {
			case 0x00: printf("   - Coordinator\n"); break;
			case 0x01: printf("   - Router\n"); break;
			case 0x02: printf("   - End Device\n"); break;
			default:   printf("   - Unknown device type\n"); break;
		}
		switch ((sStart[18] >> 2) & 0x03) {
			case 0x00: printf("   - Rx ON whilst idle\n"); break;
			case 0x01: printf("   - Rx OFF whilst idle\n"); break;
			default:   printf("   - Unknown idle Rx state\n"); break;
		}
		switch ((sStart[18] >> 4) & 0x07) {
			case 0x00: printf("   - Is my parent\n"); break;
			case 0x01: printf("   - Is my child\n"); break;
			case 0x02: printf("   - Is my sibling\n"); break;
			case 0x03: printf("   - No relation\n"); break;
			default:   printf("   - Unknown relationship\n"); break;
		}
		switch ((sStart[19] >> 0) & 0x03) {
			case 0x00: printf("   - Is NOT accepting join requests\n"); break;
			case 0x01: printf("   - Is not accepting join requests\n"); break;
			default:   printf("   - Unknown join request handling\n"); break;
		}
		printf("   - Node depth: %d (hops to coordinator)\n", sStart[20]);
		printf("   - LQI: %d\n", sStart[21]);
		entriesRemain--;
		sStart += 22;
		dataRemain -= 22;
	}
	return;
}

int main(void) {
	void *d;
	struct xbee *xbee;
	struct xbee_con *conModem;
	struct xbee_con *conReq;
	struct xbee_conAddress address;
	int i;
	xbee_err ret;

	if ((ret = xbee_setup(&xbee, "xbeeZB", "/dev/ttyUSB1", 57600)) != XBEE_ENONE) {
		printf("ret: %d (%s)\n", ret, xbee_errorToStr(ret));
		return ret;
	}

	/* ################################ */
	if ((ret = xbee_conNew(xbee, &conModem, "Modem Status", NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conNew() returned: %d (%s)", ret, xbee_errorToStr(ret));
		return ret;
	}
	if ((ret = xbee_conCallbackSet(conModem, modemCB, NULL)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conCallbackSet() returned: %d", ret);
		return ret;
	}
	
	
	/* ################################ */
	address.addr64_enabled = 1;
	address.addr64[0] = 0x00;
	address.addr64[1] = 0x13;
	address.addr64[2] = 0xA2;
	address.addr64[3] = 0x00;
	address.addr64[4] = 0x40;
	address.addr64[5] = 0x89;
	address.addr64[6] = 0x16;
	address.addr64[7] = 0xDC;
	if ((ret = xbee_zdo_conNew(xbee, &conReq, &address, 0x31, zdoCB)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_zdo_conNew() returned: %d", ret);
		return ret;
	}
	
	sleep(1);
	
	printf("\n");
	for (i = 0; i <= 0xFF; i++) {
		xbee_conTx(conReq, NULL, "%c%c", i, i);
		printf("\r0x%02X...", i);
		fflush(stdout);
	}
	printf("\n");
	
	sleep(5);

	if ((ret = xbee_zdo_conEnd(conReq)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_zdo_conNew() returned: %d", ret);
		return ret;
	}
	if ((ret = xbee_conEnd(conModem)) != XBEE_ENONE) {
		xbee_log(xbee, -1, "xbee_conEnd() returned: %d", ret);
		return ret;
	}
	

	xbee_shutdown(xbee);

	return 0;
}
