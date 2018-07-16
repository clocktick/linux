#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "hyperu_ops.h"

static void hyperu_log(const char *fmt, va_list ap)
{
	char buf[1024];

	vsnprintf(buf, 1024, fmt, ap);
	fprintf(stderr, "%s", buf);
	fflush(stderr);
}

static void hyperu_log_n(char *buf, int n)
{
	if (n > 0)
		fwrite(buf, n, 1, stderr);
	else if (!n) {
		fprintf(stderr, "%s", buf);
		fflush(stderr);
	}
}

static struct hyperu_ops hyperu_ops = {
	.log = &hyperu_log,
	.log_n = &hyperu_log_n,
	.exit = &exit,
};

struct hyperu_ops *hyperu_get_ops(void)
{
	return (struct hyperu_ops *)&hyperu_ops;
}
