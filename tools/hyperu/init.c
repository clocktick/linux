#include "hyperu_user.h"
#include <stdio.h>

#define PRIORITY_LISTS 10

static struct hlist_head init_lists[PRIORITY_LISTS];
static struct hlist_head exit_lists[PRIORITY_LISTS];

int init_list_add(struct init_item *t, initfunc_t init,
			int priority, const char *name)
{
	t->init = init;
	t->fn_name = name;
	hlist_add_head(&t->n, &init_lists[priority]);

	return 0;
}

int exit_list_add(struct init_item *t, initfunc_t init,
			int priority, const char *name)
{
	t->init = init;
	t->fn_name = name;
	hlist_add_head(&t->n, &exit_lists[priority]);

	return 0;
}

int init_list__init(struct hyperu *hyperu, unsigned long flags)
{
	unsigned int i;
	int r = 0;
	struct init_item *t;

	for (i = 0; i < PRIORITY_LISTS; i++)
		hlist_for_each_entry(t, &init_lists[i], n) {
			r = t->init(hyperu, flags);
			if (r < 0) {
				fprintf(stderr, "Failed init: %s\n", t->fn_name);
				goto fail;
			}
		}

fail:
	return r;
}

int init_list__exit(struct hyperu *hyperu, unsigned long flags)
{
	int i;
	int r = 0;
	struct init_item *t;

	for (i = PRIORITY_LISTS - 1; i >= 0; i--)
		hlist_for_each_entry(t, &exit_lists[i], n) {
			r = t->init(hyperu, flags);
			if (r < 0) {
				fprintf(stderr, "%s failed.\n", t->fn_name);
				goto fail;
			}
		}
fail:
	return r;
}
