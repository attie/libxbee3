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

const struct xbee_mode * const modeList[] = {
	NULL,
};

/* ######################################################################### */

xbee_err xbee_modeRetrieve(char *name, const struct xbee_mode **retMode) {
	int i;
	if (!name || !retMode) return XBEE_EMISSINGPARAM;
	
	for (i = 0; modeList[i]; i++) {
		if (!modeList[i]->name) continue;
		if (strcasecmp(modeList[i]->name, name)) continue;
		
		*retMode = modeList[i];
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
