#ifndef __XBEE_XSYS_LOAD_C
#error This source should be included by xsys.c only
#endif /* __XBEE_XSYS_LOAD_C */

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
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "internal.h"

#pragma comment (lib, "uuid.lib")

int xsys_select(FILE *stream, struct timeval *timeout) {
	fd_set fds;
	int fd;

	fd = fileno(stream);
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	return select(fd + 1, &fds, NULL, NULL, timeout);
}

/* ######################################################################### */

int xsys_sem_timedwait(xsys_sem *sem, struct timespec *timeout) {
	DWORD dwMiliseconds;
	if (timeout) {
		dwMiliseconds  = timeout->tv_sec * 1000;
		dwMiliseconds += timeout->tv_nsec / 1000000;
	} else {
		dwMiliseconds = 0;
	}
	return WaitForSingleObject(sem, dwMiliseconds);
}

#define SEMAQUERYINFOCLASS	0
#define WINAPI __stdcall
typedef long NTSTATUS;
struct seminfo {
	UINT Count; // current semaphore count
	UINT Limit; // max semaphore count
};
NTSTATUS ((WINAPI *NtQuerySemaphore)(HANDLE Handle, UINT InfoClass, struct seminfo *SemaInfo, UINT InfoSize, PUINT RetLen)) = NULL;
HMODULE g_ntdll = NULL;
int xsys_sem_getvalue(xsys_sem *sem, int *value) {
	struct seminfo info;
	UINT retLen;
	
	if (!NtQuerySemaphore) {
		if (!g_ntdll) {
			g_ntdll = LoadLibrary("ntdll.dll");
			if (!g_ntdll) return -1;
		}
		if (!NtQuerySemaphore) {
			NtQuerySemaphore = GetProcAddress(g_ntdll, "NtQuerySemaphore");
			if (!NtQuerySemaphore) return -1;
		}
	}
	
	if (NtQuerySemaphore(sem, SEMAQUERYINFOCLASS, &info, sizeof(info), &retLen) < 0) return -1;
	if (retLen != sizeof(info)) return -1;
	
	*value = info.Count;
	
	return 0;
}

/* ######################################################################### */
/* thanks to Carl Staelin for this:
     http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows */
LARGE_INTEGER getFILETIMEoffset() {
	SYSTEMTIME s;
	FILETIME f;
	LARGE_INTEGER t;

	s.wYear = 1970;
	s.wMonth = 1;
	s.wDay = 1;
	s.wHour = 0;
	s.wMinute = 0;
	s.wSecond = 0;
	s.wMilliseconds = 0;
	SystemTimeToFileTime(&s, &f);
	t.QuadPart = f.dwHighDateTime;
	t.QuadPart <<= 32;
	t.QuadPart |= f.dwLowDateTime;
	return (t);
}
int clock_gettime(int X, struct timeval *tv) {
	LARGE_INTEGER t;
	FILETIME f;
	double microseconds;
	static LARGE_INTEGER offset;
	static double frequencyToMicroseconds;
	static int initialized = 0;
	static BOOL usePerformanceCounter = 0;

	if (!initialized) {
		LARGE_INTEGER performanceFrequency;
		initialized = 1;
		usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
		if (usePerformanceCounter) {
			QueryPerformanceCounter(&offset);
			frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
		} else {
			offset = getFILETIMEoffset();
			frequencyToMicroseconds = 10.;
		}
	}
	if (usePerformanceCounter) {
		QueryPerformanceCounter(&t);
	} else {
		GetSystemTimeAsFileTime(&f);
		t.QuadPart = f.dwHighDateTime;
		t.QuadPart <<= 32;
		t.QuadPart |= f.dwLowDateTime;
	}

	t.QuadPart -= offset.QuadPart;
	microseconds = (double)t.QuadPart / frequencyToMicroseconds;
	t.QuadPart = microseconds;
	tv->tv_sec = t.QuadPart / 1000000;
	tv->tv_usec = t.QuadPart % 1000000;

	return (0);
}

/* ######################################################################### */

xbee_err xbee_serialSetup(struct xbee_serialInfo *info) {
  return XBEE_ENOTIMPLEMENTED;
}
