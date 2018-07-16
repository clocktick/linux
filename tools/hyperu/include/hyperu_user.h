#ifndef HYPERU_USER_H__
#define HYPERU_USER_H__

#include "hyperu_ops.h"
#include "util-init.h"

extern struct hyperu_ops *hyperu_get_ops(void);

struct hyperu {
	struct hyperu_ops *ops;
};

#endif
