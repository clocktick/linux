#ifndef HYPERU_OPS_H__
#define HYPERU_OPS_H__

#include <stdarg.h>

struct hyperu_ops {
	void (*log)(const char *fmt, va_list arg);
	void (*log_n)(char *buf, int n);
	void (*exit)(int);

	long (*syscall0)(long nr);
	long (*syscall1)(long nr, long a0);
	long (*syscall2)(long nr, long a0, long a1);
	long (*syscall3)(long nr, long a0, long a1, long a2);
	long (*syscall4)(long nr, long a0, long a1, long a2, long a3);
	long (*syscall5)(long nr, long a0, long a1, long a2, long a3, long a4);
	long (*syscall6)(long nr, long a0, long a1, long a2, long a3, long a4, long a5);

	struct {
		int (*read_msr)(long cpu, long msr, long *dataptr);
		int (*write_msr)(long cpu, long msr, long dataval);
		int (*set_iopl)(int level);
	} x86;
};

#endif
