#ifndef __XBEE_XSYS_LOAD_H
#error This header should be included by xsys.h only
#endif /* __XBEE_XSYS_LOAD_H */
#ifndef __XBEE_XSYS_WIN32_H
#define __XBEE_XSYS_WIN32_H

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

#include <Windows.h>
#include <io.h>
#include <time.h>
#include <sys/timeb.h>
#include "xsys_win32_winpthreads.h"

#define sleep(a)                  Sleep((a)*1000)
#define usleep(a)                 Sleep((a)/1000)
#define strcasecmp(a,b)           _stricmp((a),(b))
#define strncasecmp(a,b,c)        _strnicmp((a),(b),(c))
#define snprintf(a,b,...)         _snprintf((a),(b),__VA_ARGS__)

/* TODO - THIS SHOULD BE INVESTIGATED FUTHER... */
#define va_copy(a,b)             ((a) = (b))


/* ######################################################################### */

typedef pthread_t         xsys_thread;
typedef pthread_key_t     xsys_thread_key;

typedef pthread_mutex_t   xsys_mutex;

typedef HANDLE            xsys_sem;
typedef size_t            xsys_size_t;
typedef size_t            xsys_ssize_t;

typedef HANDLE            xsys_serialDev;

#define EXPORT __declspec(dllexport)
#define INIT   
#define FINI   


/* ######################################################################### */
/* file I/O */

#define xsys_open(path, flags)                open((path),(flags))
#define xsys_close(fd)                        close((fd))
#define xsys_read(fd, buf, count)             read((fd),(buf),(count))
#define xsys_write(fd, buf, count)            write((fd),(buf),(count))

#define xsys_fopen(path, mode)                fopen((path), (mode))
#define xsys_fdopen(fd, mode)                 fdopen((fd), (mode))
#define xsys_fclose(stream)                   fclose((stream))
#define xsys_fread(ptr, size, nmemb, stream)  fread((ptr), (size), (nmemb), (stream))
#define xsys_fwrite(ptr, size, nmemb, stream) fwrite((ptr), (size), (nmemb), (stream))
#define xsys_fflush(stream)                   fflush((stream))
#define xsys_ferror(stream)                   ferror((stream))
#define xsys_feof(stream)                     feof((stream))

int xsys_select(FILE *stream, struct timeval *timeout);

/* ######################################################################### */
/* threads */

#define xsys_thread_create(thread, start_routine, arg) \
                                              pthread_create((pthread_t*)(thread), NULL, (start_routine), (arg))
#define xsys_thread_cancel(thread)            pthread_cancel((pthread_t)(thread))
#define xsys_thread_join(thread, retval)      pthread_join((pthread_t)(thread), (retval))
#define xsys_thread_self()                    pthread_self()
#define xsys_thread_detach(thread)            pthread_detach(thread)
#define xsys_thread_detach_self()             pthread_detach(pthread_self())
#define xsys_thread_iAm(thread)               pthread_equal(pthread_self(), (thread))
#define xsys_thread_lock()                    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL)
#define xsys_thread_unlock()                  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)

#define xsys_thread_key_init(key, destructor) pthread_key_create((key), (destructor))
#define xsys_thread_key_set(key, value)       pthread_setspecific((key), (value))
#define xsys_thread_key_get(key)              pthread_getspecific((key))

/* ######################################################################### */
/* mutexes */

#define xsys_mutex_init(mutex)                pthread_mutex_init((pthread_mutex_t*)(mutex), NULL)
#define xsys_mutex_destroy(mutex)             pthread_mutex_destroy((pthread_mutex_t*)(mutex))
#define xsys_mutex_lock(mutex)                pthread_mutex_lock((pthread_mutex_t*)(mutex))
#define xsys_mutex_trylock(mutex)             pthread_mutex_trylock((pthread_mutex_t*)(mutex))
#define xsys_mutex_unlock(mutex)              pthread_mutex_unlock((pthread_mutex_t*)(mutex))


/* ######################################################################### */
/* semaphores */

#define xsys_sem_init(sem)                    (((*(sem)) = CreateSemaphore(NULL,0,1024,NULL)) == NULL)
#define xsys_sem_destroy(sem)                 CloseHandle(*(sem))
#define xsys_sem_wait(sem)                    WaitForSingleObject(*(sem),INFINITE)
#define xsys_sem_trywait(sem)                 WaitForSingleObject(*(sem),0)
int xsys_sem_timedwait(xsys_sem *sem, struct timespec *timeout);
#define xsys_sem_post(sem)                    (!ReleaseSemaphore(*(sem),1,NULL))
int xsys_sem_getvalue(xsys_sem *sem, int *value);


#endif /* __XBEE_XSYS_WIN32_H */
