/*
  libxbee - a C library to aid the use of Digi's XBee wireless modules
            running in API mode (AP=2).

  Copyright (C) 2009  Attie Grande (attie@attie.co.uk)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../internal.h"
#include "../xbee_int.h"
#include "../log.h"
#include "common.h"

xbee_err xbee_serialSetup(struct xbee_serialInfo *info) {
  struct termios tc;
  speed_t chosenbaud;

	if (!info) return XBEE_EMISSINGPARAM;
	
	switch (info->baudrate) {
		case 1200:   chosenbaud = B1200;   break;
		case 2400:   chosenbaud = B2400;   break;
		case 4800:   chosenbaud = B4800;   break;
		case 9600:   chosenbaud = B9600;   break;
		case 19200:  chosenbaud = B19200;  break;
		case 38400:  chosenbaud = B38400;  break;
		case 57600:  chosenbaud = B57600;  break;
		case 115200: chosenbaud = B115200; break;
		default:
			return XBEE_EINVALBAUDRATE;
	}
	
	if ((info->fd = xsys_open(info->device, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK)) == -1) return XBEE_EIO;
	/* purge buffer */
	{
		char buf[1024];
		int n;
		do {
			n = read(info->fd, buf, sizeof(buf));
		} while (n > 0);
	}
	fcntl(info->fd, F_SETFL, 0); /* disable blocking */
	
	if ((info->f = xsys_fdopen(info->fd, "r+")) == NULL) return XBEE_EIO;
	
	xsys_fflush(info->f);
	xsys_disableBuffer(info->f);
	
	if (tcgetattr(info->fd, &tc)) {
		perror("tcgetattr()");
		return XBEE_ESETUP;
	}
	
  /* input flags */
  tc.c_iflag &= ~ IGNBRK;           /* enable ignoring break */
  tc.c_iflag &= ~(IGNPAR | PARMRK); /* disable parity checks */
  tc.c_iflag &= ~ INPCK;            /* disable parity checking */
  tc.c_iflag &= ~ ISTRIP;           /* disable stripping 8th bit */
  tc.c_iflag &= ~(INLCR | ICRNL);   /* disable translating NL <-> CR */
  tc.c_iflag &= ~ IGNCR;            /* disable ignoring CR */
  tc.c_iflag &= ~(IXON | IXOFF);    /* disable XON/XOFF flow control */
  /* output flags */
  tc.c_oflag &= ~ OPOST;            /* disable output processing */
  tc.c_oflag &= ~(ONLCR | OCRNL);   /* disable translating NL <-> CR */
  tc.c_oflag &= ~ OFILL;            /* disable fill characters */
  /* control flags */ 
  tc.c_cflag |=   CLOCAL;           /* prevent changing ownership */
  tc.c_cflag |=   CREAD;            /* enable reciever */
  tc.c_cflag &= ~ PARENB;           /* disable parity */
  tc.c_cflag &= ~ CSTOPB;           /* disable 2 stop bits */
  tc.c_cflag &= ~ CSIZE;            /* remove size flag... */
  tc.c_cflag |=   CS8;              /* ...enable 8 bit characters */
  tc.c_cflag |=   HUPCL;            /* enable lower control lines on close - hang up */
#ifdef XBEE_NO_RTSCTS
  tc.c_cflag &= ~ CRTSCTS;          /* disable hardware CTS/RTS flow control */
#else
  tc.c_cflag |=   CRTSCTS;          /* enable hardware CTS/RTS flow control */
#endif
  /* local flags */
  tc.c_lflag &= ~ ISIG;             /* disable generating signals */
  tc.c_lflag &= ~ ICANON;           /* disable canonical mode - line by line */
  tc.c_lflag &= ~ ECHO;             /* disable echoing characters */
  tc.c_lflag &= ~ ECHONL;           /* ??? */
  tc.c_lflag &= ~ NOFLSH;           /* disable flushing on SIGINT */
  tc.c_lflag &= ~ IEXTEN;           /* disable input processing */

  /* control characters */
  memset(tc.c_cc,0,sizeof(tc.c_cc));
	
	/* set i/o baud rate */
  if (cfsetspeed(&tc, chosenbaud)) {
		perror("cfsetspeed()");
		return XBEE_ESETUP;
	}
	
  if (tcsetattr(info->fd, TCSAFLUSH, &tc)) {
		perror("tcsetattr()");
		return XBEE_ESETUP;
	}
	
	/* enable input & output transmission */
  if (tcflow(info->fd, TCOON | TCION)) {
		perror("tcflow()");
		return XBEE_ESETUP;
	}
	
	return XBEE_ENONE;
}

/* ######################################################################### */

#define XBEE_MAX_BUFFERLEN 256

static xbee_err xbee_ioRead(FILE *f, int len, unsigned char *dest, int escaped) {
	int pos;
	int ret;
	
	if (!f || len == 0 || !dest) return XBEE_EMISSINGPARAM;
	
	pos = 0;
	do {
		if ((ret = xsys_select(f, NULL)) == -1) {
			perror("xbee_select()");
			if (errno == EINTR) return XBEE_ESELECTINTERRUPTED;
			return XBEE_ESELECT;
		}
		ret = xsys_fread(&(dest[pos]), 1, len - pos, f);
		if (ret == 0) {
			if (xsys_ferror(f)) {
				perror("fread()");
				if (xsys_feof(f)) return XBEE_EEOF;
				return XBEE_EIO;
			}
			continue;
		}
		
		/* process the escape characters out */
		if (escaped) {
			int i, p, d;
			p = pos + (ret - 1);
			
			if (pos > 0) {
				i = pos - 1;
			} else {
				i = pos;
			}
			
			/* yes this bit is complex... */
			d = 0;
			for (; i < p; i++) {
				if (d > 0) {
					dest[i] = dest[i + d];
				}
				if (dest[i] == 0x7D) {
					dest[i] = dest[i + d + 1] ^ 0x20;
					d++;
				}
			}
			
			ret -= d;
		}
		
		pos += ret;
	} while (pos < len);
	
	return XBEE_ENONE;
}

xbee_err xbee_xbeeRxIo(struct xbee *xbee, struct xbee_buf **buf) {
	struct xbee_buf *iBuf;
	void *p;
	
	struct xbee_serialInfo *data;
	
	unsigned char c;
	unsigned char chksum;
	int t;
	xbee_err ret;
	
	if (!xbee || !buf) return XBEE_EMISSINGPARAM;
	if (!xbee->mode || !xbee->modeData) return XBEE_EINVAL;
	
	data = xbee->modeData;
	
	if ((iBuf = malloc(sizeof(*iBuf) + XBEE_MAX_BUFFERLEN)) == NULL) return XBEE_ENOMEM;
	
	while (1) {
		/* get the start delimiter (0x7F) */
		do {
			if ((ret = xbee_ioRead(data->f, 1, &c, 0)) != XBEE_ENONE) return ret;
		} while (c != 0x7E);
		
		/* get the length (2 bytes) */
		if ((ret = xbee_ioRead(data->f, 2, iBuf->data, 1)) != XBEE_ENONE) return ret;
		t = ((iBuf->data[0] << 8) & 0xFF00) | (iBuf->data[1] & 0xFF);
		if (t > XBEE_MAX_BUFFERLEN) {
			xbee_log(1, "OVERSIZED PACKET... data loss has occured (packet length: %d)", t);
			continue;
		}
		iBuf->len = t;
		
		/* get the data! */
		if ((ret = xbee_ioRead(data->f, iBuf->len, iBuf->data, 1)) != XBEE_ENONE) return ret;
		
		/* get the checksum */
		if ((ret = xbee_ioRead(data->f, 1, &chksum, 1)) != XBEE_ENONE) return ret;
		
		/* check the checksum */
		for (t = 0; t < iBuf->len; t++) {
			chksum += iBuf->data[t];
		}
		if ((chksum & 0xFF) != 0xFF) {
			xbee_log(1, "INVALID CHECKSUM... data loss has occured (packet length: %d)", iBuf->len);
			for (t = 0; t < iBuf->len; t++) {
				xbee_log(10, "  %3d: 0x%02X  %c", t, iBuf->data[t], ((iBuf->data[t] >= ' ' && iBuf->data[t] <= '~') ? iBuf->data[t] : '.'));
			}
			continue;
		}
		break;
	}
	
	/* resize the memory, and ignore failure */
	if ((p = realloc(iBuf, sizeof(*iBuf) + iBuf->len)) != NULL) iBuf = p;
	iBuf->data[iBuf->len] = '\0'; /* null terminate the data */
	
	*buf = iBuf;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

xbee_err xbee_xbeeTxIo(struct xbee *xbee, struct xbee_buf *buf) {
	return XBEE_ENOTIMPLEMENTED;
}
