#ifndef __XBEE_LL_H
#define __XBEE_LL_H

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

#include <pthread.h>

typedef pthread_mutex_t   xsys_mutex;
#define xsys_mutex_init(mutex)                pthread_mutex_init((pthread_mutex_t*)(mutex), NULL)
#define xsys_mutex_destroy(mutex)             pthread_mutex_destroy((pthread_mutex_t*)(mutex))
#define xsys_mutex_lock(mutex)                pthread_mutex_lock((pthread_mutex_t*)(mutex))
#define xsys_mutex_trylock(mutex)             pthread_mutex_trylock((pthread_mutex_t*)(mutex))
#define xsys_mutex_unlock(mutex)              pthread_mutex_unlock((pthread_mutex_t*)(mutex))

/* DO NOT RE-ORDER! */
struct ll_head {
	struct ll_info *head;
	struct ll_info *tail;
	int is_head;
	struct ll_head *self;
	xsys_mutex mutex;
};

xbee_err ll_init(struct ll_head *list);
void ll_destroy(struct ll_head *list, void (*freeCallback)(void*));

void *ll_alloc(void);
void ll_free(void *list, void (*freeCallback)(void *));

#define ll_add_head(list, item)        _ll_add_head((list),(item),1)
xbee_err _ll_add_head(void *list, void *item, int needMutex);
/* - */
#define ll_add_tail(list, item)        _ll_add_tail((list),(item),1)
xbee_err _ll_add_tail(void *list, void *item, int needMutex);
/* - */
#define ll_add_after(list, ref, item)  _ll_add_after((list),(ref),(item),1)
xbee_err _ll_add_after(void *list, void *ref, void *item, int needMutex);
/* - */
#define ll_add_before(list, ref, item) _ll_add_before((list),(ref),(item),1)
xbee_err _ll_add_before(void *list, void *ref, void *item, int needMutex);

void *ll_get_head(void *list);
void *ll_get_tail(void *list);

/* returns struct ll_info* or NULL - don't touch the pointer ;) */
#define ll_get_item(list, item) _ll_get_item((list),(item),1)
void *_ll_get_item(void *list, void *item, int needMutex);

#define ll_get_next(list, ref) _ll_get_next((list),(ref),1)
void *_ll_get_next(void *list, void *ref, int needMutex);
/* - */
#define ll_get_prev(list, ref) _ll_get_prev((list),(ref),1)
void *_ll_get_prev(void *list, void *ref, int needMutex);
/* - */
#define ll_get_index(list, index) _ll_get_index((list),(index),1)
void *_ll_get_index(void *list, unsigned int index, int needMutex);

#define ll_ext_head(list)       _ll_ext_head((list),1)
void *_ll_ext_head(void *list, int needMutex);
/* - */
#define ll_ext_tail(list)       _ll_ext_tail((list),1)
void *_ll_ext_tail(void *list, int needMutex);
/* - */
#define ll_ext_item(list, item) _ll_ext_item((list),(item),1)
xbee_err _ll_ext_item(void *list, void *item, int needMutex);

xbee_err ll_modify_item(void *list, void *oldItem, void *newItem);
xbee_err ll_count_items(void *list);
xbee_err ll_combine(void *head, void *tail);

#endif /* __XBEE_LL_H */
