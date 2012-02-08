#ifndef __XBEE_XSYS_LOAD_H
#error This header should be included by xsys.h only
#endif /* __XBEE_XSYS_LOAD_H */
#ifndef __XBEE_XSYS_LINUX_H
#define __XBEE_XSYS_LINUX_H

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

#include <unistd.h>
#include <sys/time.h>

#include <fcntl.h>
#define __USE_GNU
#include <pthread.h>
#undef __USE_GNU
#include <semaphore.h>


/* ######################################################################### */

typedef pthread_t         xsys_thread;

typedef pthread_mutex_t   xsys_mutex;

typedef sem_t             xsys_sem;
typedef size_t            xsys_size_t;
typedef ssize_t           xsys_ssize_t;

#define EXPORT __attribute__((visibility("default")))
#define INIT   __attribute__((constructor))
#define FINI   __attribute__((destructor))


/* ######################################################################### */
/* file I/O */

#define xsys_open(path, flags)                open((path),(flags))
int xsys_lockf(int fd);
#define xsys_close(fd)                        close((fd))
#define xsys_read(fd, buf, count)             read((fd),(buf),(count))
#define xsys_write(fd, buf, count)            write((fd),(buf),(count))

#define xsys_fopen(path, mode)                fopen((path),(mode))
#define xsys_fdopen(fd, mode)                 fdopen((fd),(mode))
#define xsys_fclose(stream)                   fclose((stream))
#define xsys_fread(ptr, size, nmemb, stream)  fread((ptr),(size),(nmemb),(stream))
#define xsys_fwrite(ptr, size, nmemb, stream) fwrite((ptr),(size),(nmemb),(stream))
#define xsys_fflush(stream)                   fflush((stream))
#define xsys_ferror(stream)                   ferror((stream))
#define xsys_feof(stream)                     feof((stream))

int xsys_select(FILE *stream, struct timeval *timeout);

#define xsys_disableBuffer(stream)            setvbuf((stream), NULL, _IONBF, BUFSIZ)


/* ######################################################################### */
/* threads */

#define xsys_thread_create(thread, start_routine, arg) \
                                              pthread_create((pthread_t*)(thread), NULL, (start_routine), (arg))
#define xsys_thread_cancel(thread)            pthread_cancel((pthread_t)(thread))
#define xsys_thread_join(thread, retval)      pthread_join((pthread_t)(thread), (retval))
#define xsys_thread_detach_self()             pthread_detach(pthread_self())
#define xsys_thread_iAm(thread)               pthread_equal(pthread_self(), (thread))


/* ######################################################################### */
/* mutexes */

#define xsys_mutex_init(mutex)                pthread_mutex_init((pthread_mutex_t*)(mutex), NULL)
#define xsys_mutex_destroy(mutex)             pthread_mutex_destroy((pthread_mutex_t*)(mutex))
#define xsys_mutex_lock(mutex)                pthread_mutex_lock((pthread_mutex_t*)(mutex))
#define xsys_mutex_trylock(mutex)             pthread_mutex_trylock((pthread_mutex_t*)(mutex))
#define xsys_mutex_unlock(mutex)              pthread_mutex_unlock((pthread_mutex_t*)(mutex))


/* ######################################################################### */
/* semaphores */

#define xsys_sem_init(sem)                    sem_init((sem_t*)(sem), 0, 0)
#define xsys_sem_destroy(sem)                 sem_destroy((sem_t*)(sem))
#define xsys_sem_wait(sem)                    sem_wait((sem_t*)(sem))
#define xsys_sem_timedwait(sem, timeout)      sem_timedwait((sem_t*)(sem), (timeout))
#define xsys_sem_post(sem)                    sem_post((sem_t*)(sem))
#define xsys_sem_getvalue(sem, value)         sem_getvalue((sem), (value))


#endif /* __XBEE_XSYS_LINUX_H */
