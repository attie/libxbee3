#ifndef __XBEE_LL_H
#define __XBEE_LL_H

/*
	libxbee - a C library to aid the use of Digi's XBee wireless modules
	          running in API mode.

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

xbee_err ll_lock(void *list);
xbee_err ll_unlock(void *list);




#define ll_add_head(list, item)         _ll_add_head((list),(item),1)
#define ll_add_tail(list, item)         _ll_add_tail((list),(item),1)
#define ll_add_after(list, ref, item)   _ll_add_after((list),(ref),(item),1)
#define ll_add_before(list, ref, item)  _ll_add_before((list),(ref),(item),1)

xbee_err _ll_add_head(void *list, void *item, int needMutex);
xbee_err _ll_add_tail(void *list, void *item, int needMutex);
xbee_err _ll_add_after(void *list, void *ref, void *item, int needMutex);
xbee_err _ll_add_before(void *list, void *ref, void *item, int needMutex);


#define ll_get_head(list, item)         _ll_get_head((list),(item),1)
#define ll_get_tail(list, item)         _ll_get_tail((list),(item),1)
#define ll_get_item(list, item)         _ll_get_item((list),(item),1)
#define ll_get_next(list, ref, item)    _ll_get_next((list),(ref),(item),1)
#define ll_get_prev(list, ref, item)    _ll_get_prev((list),(ref),(item),1)
#define ll_get_index(list, index, item) _ll_get_index((list),(index),(item),1)

#define ll_ext_head(list, item)         _ll_ext_head((list),(item),1)
#define ll_ext_tail(list, item)         _ll_ext_tail((list),(item),1)
#define ll_ext_item(list, item)         _ll_ext_item((list),(item),1)
#define ll_ext_index(list, index, item) _ll_ext_index((list),(index),(item),1)

xbee_err _ll_get_head(void *list, void **retItem, int needMutex);
xbee_err _ll_get_tail(void *list, void **retItem, int needMutex);
xbee_err _ll_get_item(void *list, void *item, int needMutex);
xbee_err _ll_get_next(void *list, void *ref, void **retItem, int needMutex);
xbee_err _ll_get_prev(void *list, void *ref, void **retItem, int needMutex);
xbee_err _ll_get_index(void *list, unsigned int index, void **retItem, int needMutex);

xbee_err _ll_ext_head(void *list, void **retItem, int needMutex);
xbee_err _ll_ext_tail(void *list, void **retItem, int needMutex);
xbee_err _ll_ext_item(void *list, void *item, int needMutex);
xbee_err _ll_ext_index(void *list, unsigned int index, void **retItem, int needMutex);


#define ll_modify_items(list, oldItem, newItem) \
                                        _ll_modify_item((list), (oldItem), (newItem), 1)
#define ll_count_items(list, retCount)  _ll_count_items((list), (retCount), 1)

xbee_err _ll_modify_item(void *list, void *oldItem, void *newItem, int needMutex);
xbee_err _ll_count_items(void *list, unsigned int *retCount, int needMutex);
xbee_err ll_combine(void *head, void *tail);

#endif /* __XBEE_LL_H */
