#include <sys/io.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hyperu_user.h"

static int set_iopl(int level)
{
	int rc;

	rc = iopl(level);
	if (rc < 0) {
		fprintf(stderr, "iopl(%d):%s\n", level, strerror(errno));
	}
	return rc;
}

static int io_init(struct hyperu *hyperu, unsigned long flags)
{
	hyperu->ops->x86.set_iopl = &set_iopl;
	if (INIT_F_VERBOSE & flags)
		fprintf(stderr, "init:io OK\n");
	return 0;
}

static int io_exit(struct hyperu *hyperu, unsigned long flags)
{
	return 0;
}

arch_init(io_init);
arch_exit(io_exit);
