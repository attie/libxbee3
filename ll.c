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

#include "xbee.h"
#include "ll.h"

/* DO NOT RE-ORDER! */
struct ll_info {
	struct ll_info *next;
	struct ll_info *prev;
	int is_head;
	struct ll_head *head;
	void *item;
};

/* this file is scary, sorry it isn't commented... i nearly broke myself writing it
   maybe oneday soon i'll be brave and put some commends down */

xbee_err ll_init(struct ll_head *list) {
	if (!list) return XBEE_EINVAL;
	list->is_head = 1;
	list->head = NULL;
	list->tail = NULL;
	list->self = list;
	if (xsys_mutex_init(&list->mutex)) return XBEE_EMUTEX;
	return 0;
}

void ll_destroy(struct ll_head *list, void (*freeCallback)(void *)) {
	void *p;
	while ((p = ll_ext_tail(list)) != NULL) {
		if (freeCallback) freeCallback(p);
	}
	xsys_mutex_destroy(&list->mutex);
}

/* ################################################################ */

void *ll_alloc(void) {
	struct ll_head *h;
	if ((h = calloc(1, sizeof(struct ll_head))) == NULL) return NULL;
	if (ll_init(h) != 0) {
		free(h);
		h = NULL;
	}
	return h;
}

void ll_free(void *list, void (*freeCallback)(void *)) {
	ll_destroy(list, freeCallback);
	free(list);
}

/* ################################################################ */

xbee_err _ll_add_head(void *list, void *item, int needMutex) {
	struct ll_head *h;
	struct ll_info *i, *p;
	xbee_err ret;
	ret = 0;
	if (!list || !item) return XBEE_EINVAL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return XBEE_EINVAL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	p = h->head;
	if (!(h->head = calloc(1, sizeof(struct ll_info)))) {
		h->head = p;
		ret = -2;
		goto out;
	}
	h->head->head = h;
	h->head->prev = NULL;
	if (p) {
		h->head->next = p;
		p->prev = h->head;
	} else {
		h->head->next = NULL;
		h->tail = h->head;
	}
	h->head->item = item;
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

xbee_err _ll_add_tail(void *list, void *item, int needMutex) {
	struct ll_head *h;
	struct ll_info *i, *p;
	xbee_err ret;
	ret = 0;
	if (!list || !item) return XBEE_EINVAL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return XBEE_EINVAL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	p = h->tail;
	if (!(h->tail = calloc(1, sizeof(struct ll_info)))) {
		h->tail = p;
		ret = -2;
		goto out;
	}
	h->tail->head = h;
	h->tail->next = NULL;
	if (p) {
		h->tail->prev = p;
		p->next = h->tail;
	} else {
		h->tail->prev = NULL;
		h->head = h->tail;
	}
	h->tail->item = item;
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

/* NULL ref will add to tail */
xbee_err _ll_add_after(void *list, void *ref, void *item, int needMutex) {
	struct ll_head *h;
	struct ll_info *i, *t;
	xbee_err ret;
	ret = 0;
	if (!list || !item) return XBEE_EINVAL;
	if (!ref) return ll_add_tail(list, item);
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return XBEE_EINVAL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	i = h->head;
	while (i) {
		if (i->item == ref) break;
		i = i->next;
	}
	if (!i) {
		ret = -2;
		goto out;
	}
	if (!(t = calloc(1, sizeof(struct ll_info)))) {
		ret = -3;
		goto out;
	}
	t->head = i->head;
	if (!i->next) {
		h->tail = t;
		t->next = NULL;
	} else {
		i->next->prev = t;
		t->next = i->next;
	}
	i->next = t;
	t->prev = i;
	t->item = item;
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

/* NULL ref will add to head */
xbee_err _ll_add_before(void *list, void *ref, void *item, int needMutex) {
	struct ll_head *h;
	struct ll_info *i, *t;
	xbee_err ret;
	ret = 0;
	if (!list || !item) return XBEE_EINVAL;
	if (!ref) return ll_add_tail(list, item);
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return XBEE_EINVAL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	i = h->head;
	while (i) {
		if (i->item == ref) break;
		i = i->next;
	}
	if (!i) {
		ret = -2;
		goto out;
	}
	if (!(t = calloc(1, sizeof(struct ll_info)))) {
		ret = -3;
		goto out;
	}
	t->head = i->head;
	if (!i->prev) {
		h->head = t;
		t->prev = NULL;
	} else {
		i->prev->next = t;
		t->prev = i->prev;
	}
	i->prev = t;
	t->next = i;
	t->item = item;
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

/* ################################################################ */

void *ll_get_head(void *list) {
	struct ll_head *h;
	struct ll_info *i;
	void *ret;
	ret = NULL;
	if (!list) return NULL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (!h->head) return NULL;
	return h->head->item;
}

void *ll_get_tail(void *list) {
	struct ll_head *h;
	struct ll_info *i;
	void *ret;
	ret = NULL;
	if (!list) return NULL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (!h->tail) return NULL;
	return h->tail->item;
}

/* returns struct ll_info* or NULL - don't touch the pointer if you don't know what you're doing ;) */
void *_ll_get_item(void *list, void *item, int needMutex) {
	struct ll_head *h;
	struct ll_info *i;
	if (!list || !item) return NULL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	i = h->head;
	while (i) {
		if (i->item == item) break;
		i = i->next;
	}
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return i;
}

void *_ll_get_next(void *list, void *ref, int needMutex) {
	struct ll_head *h;
	struct ll_info *i;
	void *ret;
	ret = NULL;
	if (!list) return NULL;
	if (!ref) return ll_get_head(list);
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	i = h->head;
	if (!(i = _ll_get_item(list, ref, 0))) goto out;
	i = i->next;
	if (i) ret = i->item;
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

void *_ll_get_prev(void *list, void *ref, int needMutex) {
	struct ll_head *h;
	struct ll_info *i;
	void *ret;
	ret = NULL;
	if (!list) return NULL;
	if (!ref) return ll_get_tail(list);
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	i = h->head;
	if (!(i = _ll_get_item(list, ref, 0))) goto out;
	i = i->prev;
	if (i) ret = i->item;
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return i->item;
}

void *_ll_get_index(void *list, unsigned int index, int needMutex) {
	struct ll_head *h;
	struct ll_info *i;
	void *ret;
	if (!list) return NULL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	i = h->head;
	for (ret = NULL; (ret = _ll_get_next(list, ret, 0)) != NULL && index; index--);
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

/* ################################################################ */

void *_ll_ext_head(void *list, int needMutex) {
	struct ll_head *h;
	struct ll_info *i, *p;
	void *ret;
	ret = NULL;
	if (!list) return NULL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	p = h->head;
	if (!p) goto out;
	ret = p->item;
	h->head = p->next;
	if (h->head) h->head->prev = NULL;
	if (h->tail == p) h->tail = NULL;
	free(p);
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

void *_ll_ext_tail(void *list, int needMutex) {
	struct ll_head *h;
	struct ll_info *i, *p;
	void *ret;
	ret = NULL;
	if (!list) return NULL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return NULL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	p = h->tail;
	if (!p) goto out;
	ret = p->item;
	h->tail = p->prev;
	if (h->tail) h->tail->next = NULL;
	if (h->head == p) h->head = NULL;
	free(p);
out:
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

xbee_err _ll_ext_item(void *list, void *item, int needMutex) {
	struct ll_head *h;
	struct ll_info *i, *p;
	xbee_err ret;
	ret = 0;
	if (!list || !item) return XBEE_EINVAL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return XBEE_EINVAL;
	if (needMutex) xsys_mutex_lock(&h->mutex);
	p = h->head;
	while (p) {
		if (p->is_head) {
			ret = -2;
			break;
		}
		if (p->item == item) {
			if (p->next) {
				p->next->prev = p->prev;
			} else {
				h->tail = p->prev;
			}
			if (p->prev) {
				p->prev->next = p->next;
			} else {
				h->head = p->next;
			}
			free(p);
			break;
		}
		p = p->next;
	}
	if (needMutex) xsys_mutex_unlock(&h->mutex);
	return ret;
}

void *ll_ext_index(void *list, int index) {
	void *ret;
	if (!(ret = ll_get_index(list, index))) return NULL;
	if (ll_ext_item(list, ret)) return NULL;
	return ret;
}

/* ################################################################ */

xbee_err ll_count_items(void *list) {
	struct ll_head *h;
	struct ll_info *i, *p;
	xbee_err ret;
	ret = -1;
	if (!list) return XBEE_EINVAL;
	i = list;
	h = i->head;
	if (!(h && h->is_head && h->self == h)) return XBEE_EINVAL;
	xsys_mutex_lock(&h->mutex);
	for (p = h->head, ret = 0; p; p = p->next, ret++);
	xsys_mutex_unlock(&h->mutex);
	return ret;
}

/* ################################################################ */

xbee_err ll_combine(void *head, void *tail) {
	struct ll_head *hH, *hT;
	struct ll_info *iH, *iT;
	void *v;
	xbee_err ret;
	ret = 0;
	if (!head || !tail) return XBEE_EINVAL;
	iH = head;
	hH = iH->head;
	if (!(hH && hH->is_head && hH->self == hH)) return XBEE_EINVAL;
	xsys_mutex_lock(&hH->mutex);
	
	iT = tail;
	hT = iT->head;
	if (!(hT && hT->is_head && hT->self == hT)) { ret = XBEE_EINVAL; goto out; }
	xsys_mutex_lock(&hT->mutex);
	
	while ((v = _ll_ext_head(tail, 0)) != NULL) {
		_ll_add_tail(head, v, 0);
	}
	xsys_mutex_unlock(&hH->mutex);
out:
	xsys_mutex_unlock(&hT->mutex);
	return ret;
}
