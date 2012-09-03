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
#include "prepare.h"

#pragma comment (lib, "uuid.lib")

/* ######################################################################### */

HMODULE module = NULL;
int attach_counter = 0;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH) {
		/* ensure that libxbee has been shut down nicely */
		attach_counter--;
		if (!attach_counter) {
			xbee_fini();
		}
	} else if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH) {
		attach_counter++;
		if (!module) {
			/* keep a handle on the module */
			module = (HMODULE)hModule;
			xbee_init();
		}
	}
	return TRUE;
}

HRESULT __stdcall DllCanUnloadNow(void) {
	return !attach_counter;
}

/* ######################################################################### */

/* when opening COM1-COM9, you can specify 'COMx'
   when opening ports out of this range, you must specify '\\\\.\\COMx' */
int xsys_serialSetup(struct xbee_serialInfo *info) {
	DCB tc;
	DWORD ev_mask;
	COMMTIMEOUTS timeouts;

	if (!info) return XBEE_EMISSINGPARAM;
	if (!info->device) return XBEE_EINVAL;

	info->dev = CreateFile(TEXT(info->device),
	                       GENERIC_READ | GENERIC_WRITE,
	                       0,    /* exclusive access */
	                       NULL, /* default security attributes */
	                       OPEN_EXISTING,
	                       0,
	                       NULL);

	if (info->dev == INVALID_HANDLE_VALUE) return XBEE_EIO;

	GetCommState(info->dev, &tc);
	tc.BaudRate          = info->baudrate;
	tc.fBinary           = TRUE;
	tc.fParity           = FALSE;
	#ifdef XBEE_NO_RTSCTS
	tc.fOutxCtsFlow      = FALSE;
	tc.fRtsControl       = RTS_CONTROL_DISABLE;
	#else
	tc.fOutxCtsFlow      = TRUE;
	tc.fRtsControl       = RTS_CONTROL_ENABLE;
	#endif
	tc.fOutxDsrFlow      = FALSE;
	tc.fDtrControl       = DTR_CONTROL_DISABLE;
	tc.fDsrSensitivity   = FALSE;
	tc.fTXContinueOnXoff = FALSE;
	tc.fOutX             = FALSE;
	tc.fInX              = FALSE;
	tc.fErrorChar        = FALSE;
	tc.fNull             = FALSE;
	tc.fAbortOnError     = FALSE;
	tc.ByteSize          = 8;
	tc.Parity            = NOPARITY;
	tc.StopBits          = ONESTOPBIT;
	SetCommState(info->dev, &tc);

	/* setup some timeouts to keep things moving (damn windows) */
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutMultiplier = 5;
	timeouts.ReadTotalTimeoutConstant = 25;
	timeouts.WriteTotalTimeoutMultiplier = 5;
	timeouts.WriteTotalTimeoutConstant = 25;
	SetCommTimeouts(info->dev, &timeouts);

	return XBEE_ENONE;
}

int xsys_serialShutdown(struct xbee_serialInfo *info) {
	if (!info) return XBEE_EMISSINGPARAM;
	if (info->dev == INVALID_HANDLE_VALUE) return XBEE_EINVAL;
	CloseHandle(info->dev);
	info->dev = INVALID_HANDLE_VALUE;
	return XBEE_ENONE;
}

int xsys_serialRead(struct xbee_serialInfo *info, int len, unsigned char *dest) {
	int pos;
	int ret = 1;
	int err;
	
	if (!info) return XBEE_EMISSINGPARAM;
	if (info->dev == INVALID_HANDLE_VALUE) return XBEE_EINVAL;
	
	for (pos = 0; pos < len; pos += ret) {
		if (ReadFile(info->dev, &(dest[pos]), len - pos, &ret, NULL)) {
			if (ret == 0) usleep(2000);
			continue;
		}
		
		err = GetLastError();
		if (err == ERROR_ACCESS_DENIED || err == ERROR_OPERATION_ABORTED) {
			/* this seems to happen when an FTDI is unplugged */
			return XBEE_EEOF;
		}
		//printf("err: %d\n", err);
#ifndef _WIN32
#warning TODO - decide if err is due to device being removed (e.g: FTDI) and return XBEE_EOF
#endif
		return XBEE_EIO;
	}
	
	return XBEE_ENONE;
}

int xsys_serialWrite(struct xbee_serialInfo *info, int len, unsigned char *src) {
	int pos;
	int ret;
	int err;
	
	if (!info) return XBEE_EMISSINGPARAM;
	if (info->dev == INVALID_HANDLE_VALUE) return XBEE_EINVAL;
	
	for (pos = 0; pos < len; pos += ret) {
		if (WriteFile(info->dev, &(src[pos]), len - pos, &ret, NULL)) continue;
		return XBEE_EIO;
	}
	
	return XBEE_ENONE;
}

/* ######################################################################### */

int xsys_sem_timedwait(xsys_sem *sem, struct timespec *timeout) {
	DWORD dwMiliseconds;
	if (timeout) {
		struct timespec now;
		
		/* calculate epoch miliseconds */
		dwMiliseconds  = timeout->tv_sec * 1000;
		dwMiliseconds += timeout->tv_nsec / 1000000;
		
		/* offset that from 'now' */
		clock_gettime(CLOCK_REALTIME, &now);
		dwMiliseconds -= now.tv_sec * 1000;
		dwMiliseconds -= now.tv_nsec / 1000000;
		
		/* dwMiliseconds should now hold the number of ms from NOW */
	} else {
		dwMiliseconds = 0;
	}
	return WaitForSingleObject(*sem, dwMiliseconds);
}

#define SEMAQUERYINFOCLASS	0
#define WINAPI __stdcall
typedef long NTSTATUS;
struct seminfo {
	UINT Count; // current semaphore count
	UINT Limit; // max semaphore count
};
typedef NTSTATUS ((WINAPI *_NtQuerySemaphore)(HANDLE Handle, UINT InfoClass, struct seminfo *SemaInfo, UINT InfoSize, PUINT RetLen));
_NtQuerySemaphore NtQuerySemaphore = NULL;
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
			NtQuerySemaphore = (_NtQuerySemaphore)GetProcAddress(g_ntdll, "NtQuerySemaphore");
			if (!NtQuerySemaphore) return -1;
		}
	}
	
	if (NtQuerySemaphore(*sem, SEMAQUERYINFOCLASS, &info, sizeof(info), &retLen) < 0) return -1;
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

	memset(tv, 0, sizeof(*tv));
	
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
