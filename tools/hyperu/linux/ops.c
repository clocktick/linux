#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "hyperu_user.h"

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

static int linux_init(struct hyperu *hyperu)
{
	hyperu->ops->syscall0 = &syscall0_ret_errno;
	hyperu->ops->syscall1 = &syscall1_ret_errno;
	hyperu->ops->syscall2 = &syscall2_ret_errno;
	hyperu->ops->syscall3 = &syscall3_ret_errno;
	hyperu->ops->syscall4 = &syscall4_ret_errno;
	hyperu->ops->syscall5 = &syscall5_ret_errno;
	hyperu->ops->syscall6 = &syscall6_ret_errno;
	fprintf(stderr, "init:linux OK\n");
	return 0;
}

static int linux_exit(struct hyperu *hyperu)
{
	return 0;
}

core_init(linux_init);
core_exit(linux_exit);
