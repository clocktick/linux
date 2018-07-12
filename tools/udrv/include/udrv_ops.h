#ifndef UDRV_OPS_H__
#define UDRV_OPS_H__

struct udrv_ops {
	int (*log)(const char *fmt, ...);
	void (*exit)(int);

	long (*syscall0)(long nr);
	long (*syscall1)(long nr, long a0);
	long (*syscall2)(long nr, long a0, long a1);
	long (*syscall3)(long nr, long a0, long a1, long a2);
	long (*syscall4)(long nr, long a0, long a1, long a2, long a3);
	long (*syscall5)(long nr, long a0, long a1, long a2, long a3, long a4);
	long (*syscall6)(long nr, long a0, long a1, long a2, long a3, long a4, long a5);
};

#endif
