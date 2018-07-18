#ifndef HYPERU_USER_H__
#define HYPERU_USER_H__

#include "hyperu_ops.h"
#include "util-init.h"
#include <libelf.h>

struct hyperu {
	struct hyperu_ops *ops;

	char *kernel;
	char *cmdline;
	int fd;
	Elf *elf;
};

extern struct hyperu_ops *hyperu_get_ops(void);
extern int hyperu_init_arch(struct hyperu *hyperu, unsigned long flags);
extern int elf_symbol_address(Elf *kelf, const char *name, unsigned long *val);

#endif
