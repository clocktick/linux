#ifndef UDRV_OPS_H__
#define UDRV_OPS_H__

struct udrv_ops {
	int (*log)(const char *fmt, ...);
	void (*exit)(int);
};

#endif
