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
#include "../ll.h"
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
			return XBEE_EINVAL;
	}
	
	if ((info->fd = xsys_open(info->device, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK)) == -1) return XBEE_EIO;
	
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
#ifdef linux
/* not for FreeBSD */
  tc.c_oflag &= ~ OFILL;            /* disable fill characters */
#endif /* linux */
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
#ifdef linux
/* for Linux */
  if (tcflow(info->fd, TCOON | TCION)) {
#else
/* for FreeBSD */
  if (tcflow(info->fd, TCOON)) {
#endif
		perror("tcflow()");
		return XBEE_ESETUP;
	}
	
	/* purge buffer */
	{
		char buf[1024];
		int n;
		do {
			usleep(5000); /* 5ms */
			n = read(info->fd, buf, sizeof(buf));
		} while (n > 0);
	}
	fcntl(info->fd, F_SETFL, 0); /* disable blocking */
	
#ifndef linux
/* for FreeBSD */
	usleep(250000); /* it seems that the serial port takes a while to get going... */
#endif
	
	return XBEE_ENONE;
}

/* ######################################################################### */

#define XBEE_MAX_BUFFERLEN 256

static xbee_err xbee_ioRead(FILE *f, int len, unsigned char *dest, int escaped) {
	int pos;
	int ret;
	int nextIsEscaped;
	
	if (!f || !dest) return XBEE_EMISSINGPARAM;
	if (len == 0) return XBEE_EINVAL;
	
	pos = 0;
	nextIsEscaped = 0;
	do {
		if ((ret = xsys_select(f, NULL)) == -1) {
			perror("xbee_select()");
			if (errno == EINTR) return XBEE_ESELECTINTERRUPTED;
			return XBEE_ESELECT;
		}
		ret = xsys_fread(&(dest[pos]), 1, len - pos, f);
		if (ret == 0) {
			if (xsys_feof(f)) return XBEE_EEOF;
			if (xsys_ferror(f)) {
				perror("fread()");
				return XBEE_EIO;
			}
			continue;
		}
		if (nextIsEscaped) {
			dest[pos] ^= 0x20;
			nextIsEscaped = 0;
		}
		
		/* process the escape characters out */
		if (escaped) {
			int i, p, d;
			p = pos + (ret - 1);
			
			if (pos > 0) {
				i = pos - 1;
			} else {
				i = 0;
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
			if (dest[i] == 0x7D) {
				nextIsEscaped = 1;
				d++;
			}
			
			ret -= d;
		}
		
		pos += ret;
	} while (pos < len);
	
	return XBEE_ENONE;
}

xbee_err xbee_xbeeRxIo(struct xbee *xbee, void *arg, struct xbee_buf **buf) {
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
	ll_add_tail(needsFree, iBuf);
	
	while (1) {
		/* get the start delimiter (0x7E) */
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
	ll_lock(needsFree);
	if ((p = realloc(iBuf, sizeof(*iBuf) + iBuf->len)) != NULL) {
		_ll_ext_item(needsFree, iBuf, 0);
		_ll_add_tail(needsFree, p, 0);
		iBuf = p;
	}
	ll_unlock(needsFree);

	iBuf->data[iBuf->len] = '\0'; /* null terminate the data */
	
	*buf = iBuf;
	
	return XBEE_ENONE;
}

/* ######################################################################### */

/* firstEscaped = -1 - NONE
                   0 - first byte
                   1 - second byte... */
static xbee_err xbee_ioWrite(FILE *f, int len, unsigned char *src, int firstEscaped) {
	int pos;
	int ret;
	int esc;
	int wlen;
	
	if (!f || !src) return XBEE_EMISSINGPARAM;
	if (len == 0) return XBEE_EINVAL;
	
	for (pos = 0; pos < len; pos += ret) {
		ret = 0;
	
		if (firstEscaped == -1) {
			wlen = len - pos;
		} else {
			/* handle bytes that need escaping */
			
			if (pos >= firstEscaped) {
				/* first munch all bytes that satisfy the criteria (need escaping, and are past the firstEscaped point) */
				while (src[pos] == 0x7E ||
				       src[pos] == 0x7D ||
				       src[pos] == 0x11 ||
				       src[pos] == 0x13) {
					unsigned char c[2];
					c[0] = 0x7D;
					c[1] = src[pos] ^ 0x20;
					if (xsys_fwrite(c, 2, 1, f) != 1) {
						if (xsys_ferror(f)) {
							perror("fwrite()");
							return XBEE_EIO;
						}
						usleep(5000);
					}
					pos++;
				}
			}
			/* find the next byte that needs escaping */
			for (esc = pos; esc < len; esc++) {
				if (esc < firstEscaped) continue; /* skip the first x bytes */
				if (src[esc] == 0x7E ||
				    src[esc] == 0x7D ||
				    src[esc] == 0x11 ||
				    src[esc] == 0x13) break;
			}
			wlen = esc - pos;
		}
		if (!wlen) continue;
	
		if ((ret = xsys_fwrite(&(src[pos]), 1, wlen, f)) > 0) continue;
		
		if (xsys_ferror(f)) {
			perror("fwrite()");
			return XBEE_EIO;
		}
		usleep(5000);
	}
	
	return XBEE_ENONE;
}

xbee_err xbee_xbeeTxIo(struct xbee *xbee, void *arg, struct xbee_buf *buf) {
	struct xbee_serialInfo *data;
	size_t txSize;
	size_t memSize;
	struct xbee_buf *iBuf;
	unsigned char chksum;
	int pos;

	if (!xbee || !buf) return XBEE_EMISSINGPARAM;
	if (!xbee->mode || !xbee->modeData) return XBEE_EINVAL;
	
	data = xbee->modeData;
	
	/* Delimiter + Length + Payload + Checksum */
	txSize = 4 + buf->len;
	memSize = txSize + sizeof(*iBuf);
	
	iBuf = data->txBuf;
	if (!iBuf || data->txBufSize < memSize) {
		if ((iBuf = malloc(memSize)) == NULL) return XBEE_ENOMEM;
		data->txBuf = iBuf;
		data->txBufSize = memSize;
	}
	
	iBuf->len = txSize;
	iBuf->data[0] = 0x7E;
	iBuf->data[1] = ((buf->len) >> 8) & 0xFF;
	iBuf->data[2] = ((buf->len)     ) & 0xFF;
	
	chksum = 0;
	for (pos = 0; pos < buf->len; pos++) {
		chksum += buf->data[pos];
		iBuf->data[3 + pos] = buf->data[pos];
	}
	iBuf->data[3 + pos] = 0xFF - chksum;
	
	return xbee_ioWrite(data->f, iBuf->len, iBuf->data, 1);
}
