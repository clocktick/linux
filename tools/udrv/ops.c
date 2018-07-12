#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <gelf.h>
#include <libelf.h>

#include "udrv_ops.h"

static long syscall0_ret_errno(long nr)
{
	int rc;

	rc = syscall(nr);
	if (!rc)
		return 0;
	return -errno;
}

static long syscall1_ret_errno(long nr, long a0)
{
	int rc;

	rc = syscall(nr, a0);
	if (!rc)
		return 0;
	return -errno;
}

static long syscall2_ret_errno(long nr, long a0, long a1)
{
	int rc;

	rc = syscall(nr, a0, a1);
	if (!rc)
		return 0;
	return -errno;
}

static long syscall3_ret_errno(long nr, long a0, long a1, long a2)
{
	int rc;

	rc = syscall(nr, a0, a1, a2);
	if (!rc)
		return 0;
	return -errno;
}

static long syscall4_ret_errno(long nr, long a0, long a1, long a2, long a3)
{
	int rc;

	rc = syscall(nr, a0, a1, a2, a3);
	if (!rc)
		return 0;
	return -errno;
}

static long syscall5_ret_errno(long nr, long a0, long a1, long a2, long a3, long a4)
{
	int rc;

	rc = syscall(nr, a0, a1, a2, a3, a4);
	if (!rc)
		return 0;
	return -errno;
}

static long syscall6_ret_errno(long nr, long a0, long a1, long a2, long a3, long a4, long a5)
{
	int rc;

	rc = syscall(nr, a0, a1, a2, a3, a4, a5);
	if (!rc)
		return 0;
	return -errno;
}

static struct udrv_ops udrv_ops = {
	.log = &printf,
	.exit = &exit,

	.syscall0 = &syscall0_ret_errno,
	.syscall1 = &syscall1_ret_errno,
	.syscall2 = &syscall2_ret_errno,
	.syscall3 = &syscall3_ret_errno,
	.syscall4 = &syscall4_ret_errno,
	.syscall5 = &syscall5_ret_errno,
	.syscall6 = &syscall6_ret_errno,
};

struct udrv_ops *udrv_get_ops(void)
{
	return (struct udrv_ops *)&udrv_ops;
}
