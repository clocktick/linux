#ifndef hyperu__UTIL_INIT_H
#define hyperu__UTIL_INIT_H

#include "util-kernel.h"
#include "list.h"

struct hyperu;

struct init_item {
	struct hlist_node n;
	const char *fn_name;
	int (*init)(struct hyperu *);
};

int init_list__init(struct hyperu *hyperu);
int init_list__exit(struct hyperu *hyperu);

int init_list_add(struct init_item *t, int (*init)(struct hyperu *),
			int priority, const char *name);
int exit_list_add(struct init_item *t, int (*init)(struct hyperu *),
			int priority, const char *name);

#define __init_list_add(cb, l)						\
static void __attribute__ ((constructor)) __init__##cb(void)		\
{									\
	static char name[] = #cb;					\
	static struct init_item t;					\
	init_list_add(&t, cb, l, name);					\
}

#define __exit_list_add(cb, l)						\
static void __attribute__ ((constructor)) __init__##cb(void)		\
{									\
	static char name[] = #cb;					\
	static struct init_item t;					\
	exit_list_add(&t, cb, l, name);					\
}

#define core_init(cb) __init_list_add(cb, 0)
#define arch_init(cb) __init_list_add(cb, 1)

#define core_exit(cb) __exit_list_add(cb, 0)
#define arch_exit(cb) __exit_list_add(cb, 1)

#endif
