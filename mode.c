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

#include "internal.h"
#include "xbee_int.h"
#include "mode.h"

const struct xbee_mode * const modeList[] = { MODELIST };

/* ######################################################################### */

xbee_err xbee_modeRetrieve(char *name, const struct xbee_mode **retMode) {
	const struct xbee_mode *mode;
	int i;
	if (!name || !retMode) return XBEE_EMISSINGPARAM;
	
	for (i = 0; modeList[i]; i++) {
		if (!modeList[i]->name) continue;
		if (strcasecmp(modeList[i]->name, name)) continue;
		mode = modeList[i];
		
		/* check compulsory functionality */
		if (!mode->init) return XBEE_EINVAL;
		if (!mode->rx_io) return XBEE_EINVAL;
		if (!mode->tx_io) return XBEE_EINVAL;
		
		*retMode = mode;
		return XBEE_ENONE;
	}
	
	return XBEE_EFAILED;
}

/* ######################################################################### */

/* pull the given mode information into the given xbee instance */
xbee_err xbee_modeImport(struct xbee_modeConType **retConTypes, const struct xbee_mode *mode) {
	int i, n;
	struct xbee_modeConType *conTypes;
	
	if (!retConTypes || !mode) return XBEE_EMISSINGPARAM;
	if (*retConTypes) return XBEE_EINVAL;
	
	for (n = 0; mode->conTypes[n].name; n++);
	
	if ((conTypes = malloc(sizeof(*conTypes) * (n + 1))) == NULL) return XBEE_ENOMEM;
	memset(&conTypes[n], 0, sizeof(*conTypes));
	
	for (i = 0; i < n; i++) {
		/* keep the pointers (they are const after all) */
		conTypes[i].name = mode->conTypes[i].name;
		conTypes[i].rxHandler = mode->conTypes[i].rxHandler;
		conTypes[i].txHandler = mode->conTypes[i].txHandler;
	}
	
	*retConTypes = conTypes;
	
	return XBEE_ENOTIMPLEMENTED;
}

xbee_err xbee_modeAddConType(struct xbee_modeConType **extConTypes, const char *name, const struct xbee_modeDataHandlerRx *rxHandler, const struct xbee_modeDataHandlerTx *txHandler) {
	int n;
	struct xbee_modeConType *conTypes;
	
	if (!extConTypes || !name || (!rxHandler && !txHandler)) return XBEE_EMISSINGPARAM;
	if (!*extConTypes) return XBEE_EINVAL;
	
	for (n = 0; (*extConTypes)[n].name; n++);
	
	if ((conTypes = realloc(*extConTypes, sizeof(*conTypes) * (n + 2))) == NULL) return XBEE_ENOMEM;
	memset(&conTypes[n + 1], 0, sizeof(*conTypes));
	*extConTypes = conTypes;
	
	conTypes[n].rxHandler = rxHandler;
	conTypes[n].txHandler = txHandler;
	conTypes[n].name = name; /* add the name last so that it doesn't get activated before it is completely ready */
	
	return XBEE_ENONE;
}

xbee_err xbee_modeCleanup(struct xbee_modeConType *conTypes) {
	if (!conTypes) return XBEE_EMISSINGPARAM;
	free(conTypes);
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_modeLocateConType(struct xbee_modeConType *conTypes, char *name, unsigned char *rxId, unsigned char *txId, struct xbee_modeConType **retType) {
	int i;
	
	if (!retType) return XBEE_EMISSINGPARAM;
	if (!name && !rxId && !txId) return XBEE_EMISSINGPARAM;
	
	for (i = 0; conTypes[i].name; i++) {
		if (name) {
			if (strcasecmp(conTypes[i].name, name)) continue;
		}
		if (rxId) {
			if (!conTypes[i].rxHandler) continue;
			if (conTypes[i].rxHandler->identifier != *rxId) continue;
			if (!conTypes[i].rxHandler->func) continue;
		}
		if (txId) {
			if (!conTypes[i].txHandler) continue;
			if (conTypes[i].txHandler->identifier != *txId) continue;
			if (!conTypes[i].txHandler->func) continue;
		}
		
		*retType = &conTypes[i];
		return XBEE_ENONE;
	}
	
	return XBEE_EFAILED;
}

/* ######################################################################### */

EXPORT xbee_err xbee_modeGetList(char ***retList) {
	int i, o;
	size_t memSize;
	char **mList;
	char *mName;
	if (!retList) return XBEE_EMISSINGPARAM;
	
	memSize = 0;
	for (i = 0, o = 0; modeList[i]; i++) {
		if (!modeList[i]->name) continue;
		memSize += sizeof(char *);
		memSize += sizeof(char) * (strlen(modeList[i]->name) + 1);
		o++;
	}
	memSize += sizeof(char *);
	
	if ((mList = malloc(memSize)) == NULL) {
		return XBEE_ENOMEM;
	}
	
	mName = (char *)&(mList[o+1]);
	for (i = 0, o = 0; modeList[i]; i++) {
		if (!modeList[i]->name) continue;
		mList[o] = mName;
		strcpy(mName, modeList[i]->name);
		mName += strlen(mName) + 1;
		o++;
	}
	mList[o] = NULL;
	
	*retList = mList;
	
	return XBEE_ENONE;
}

EXPORT xbee_err xbee_modeGet(struct xbee *xbee, const char **mode) {
	if (!xbee || !mode) return XBEE_EMISSINGPARAM;
#ifndef XBEE_DISABLE_STRICT_OBJECTS
	if (xbee_validate(xbee) != XBEE_ENONE) return XBEE_EINVAL;
#endif /* XBEE_DISABLE_STRICT_OBJECTS */
	*mode = xbee->mode->name;
	return XBEE_ENONE;
}
