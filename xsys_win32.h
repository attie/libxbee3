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

#pragma comment (lib, "uuid.lib")

#define usleep(a)                 Sleep((a)/1000)

/* ######################################################################### */

typedef HANDLE            xsys_thread;
typedef DWORD             xsys_thread_key;

typedef HANDLE            xsys_mutex;

typedef HANDLE            xsys_sem;
typedef size_t            xsys_size_t;
typedef size_t            xsys_ssize_t;

typedef HANDLE            xsys_file;

#define EXPORT __declspec(dllexport)
#define INIT   
#define FINI   


/* ######################################################################### */
/* file I/O */

#define xsys_open(path, flags)                open((path),(flags))
int xsys_lockf(int fd);
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

#define xbee_thread_create(thread, start_routine, arg) \
                                              (((thread) = CreateThread(NULL,0,(void *)(start_routine),(void *)(arg),0,NULL)) == NULL)
#define xsys_thread_cancel(thread)            TerminateThread((thread),0)
#define xsys_thread_join(thread, retval)      (WaitForSingleObject((thread),INFINITE) && GetExitCodeThread((thread), (retval)))
#define xsys_thread_self()                    (0)
#define xsys_thread_detach(thread)            
#define xsys_thread_detach_self()             
#define xsys_thread_iAm(thread)               
#define xsys_thread_lock()                    
#define xsys_thread_unlock()                  

#define xsys_thread_key_init(key, destructor) ((*(key)) = TlsAlloc())
#define xsys_thread_key_set(key, value)       TlsSetValue((key), (value))
#define xsys_thread_key_get(key)              TlsGetValue((key))

/* ######################################################################### */
/* mutexes */

#define xsys_mutex_init(mutex)                (((*(mutex)) = CreateEvent(NULL,FALSE,TRUE,NULL)) == NULL)
#define xsys_mutex_destroy(mutex)             CloseHandle((mutex))
#define xsys_mutex_lock(mutex)                WaitForSingleObject((mutex),INFINITE)
#define xsys_mutex_trylock(mutex)             WaitForSingleObject((mutex),0)
#define xsys_mutex_unlock(mutex)              SetEvent((mutex))


/* ######################################################################### */
/* semaphores */

#define xsys_sem_init(sem)                    (((*(sem)) = CreateEvent(NULL,FALSE,FALSE,NULL)) == NULL)
#define xsys_sem_destroy(sem)                 CloseHandle((sem))
#define xsys_sem_wait(sem)                    WaitForSingleObject((sem),INFINITE)
#define xsys_sem_trywait(sem)                 WaitForSingleObject((sem),0)
int xsys_sem_timedwait(xsys_sem *sem, struct timespec *timeout);
#define xsys_sem_post(sem)                    SetEvent((sem))
#define xsys_sem_getvalue(sem, value)         


#endif /* __XBEE_XSYS_WIN32_H */
