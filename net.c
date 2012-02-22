/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode (AP=2).

	Copyright (C) 2009	Attie Grande (attie@attie.co.uk)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.	If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "internal.h"
#include "xbee_int.h"
#include "net.h"
#include "ll.h"

EXPORT xbee_err xbee_netStart(struct xbee *xbee, int port, int(*clientFilter)(struct xbee *xbee, char *remoteHost)) {
	xbee_err ret;
	int fd;
	int i;
  struct sockaddr_in addrinfo;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (xbee->netInfo != NULL) return XBEE_EINVAL;
	if (port <= 0 || port >= 65535) return XBEE_EINVAL;
	
	if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) return XBEE_EIO;
	
	i = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) == -1) {
		xsys_close(fd);
		return XBEE_EIO;
	}
	
	memset(&addrinfo, 0, sizeof(addrinfo));
	addrinfo.sin_family = AF_INET;
	addrinfo.sin_port = htons(port);
	addrinfo.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(fd, (const struct sockaddr*)&addrinfo, sizeof(addrinfo)) == -1) {
		xsys_close(fd);
		return XBEE_EIO;
	}
	
	if ((ret = xbee_netvStart(xbee, fd, clientFilter)) != XBEE_ENONE) {
		xsys_close(fd);
	}
	
	return ret;
}

EXPORT xbee_err xbee_netvStart(struct xbee *xbee, int fd, int(*clientFilter)(struct xbee *xbee, char *remoteHost)) {
	struct xbee_netInfo *info;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (fd < 0) return XBEE_EINVAL;
	
	if (listen(fd, 512) == -1) return XBEE_EIO;
	
	if ((info = malloc(sizeof(*info))) == NULL) return XBEE_ENOMEM;
	memset(info, 0, sizeof(*info));
	
	info->fd = fd;
	info->clientFilter = clientFilter;
	info->clientList = ll_alloc();
	
	xbee->netInfo = info;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_netKillClient(struct xbee_netClientInfo *client) {
	return XBEE_ENOTIMPLEMENTED;
}

EXPORT xbee_err xbee_netStop(struct xbee *xbee) {
	struct xbee_netInfo *info;
	
	if (!xbee) return XBEE_EMISSINGPARAM;
	if (!xbee->netInfo) return XBEE_EINVAL;
	
	info = xbee->netInfo;
	xbee->netInfo = NULL;
	
	ll_free(info->clientList, (void(*)(void*))xbee_netKillClient);
	shutdown(info->fd, SHUT_RDWR);
	xsys_close(info->fd);
	
	free(info);

	return XBEE_ENONE;
}
