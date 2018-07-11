#include "include/uapi/udrv_ops.h"

void pvuser_start_kernel(struct udrv_ops *ops, unsigned long flags)
{
	ops->log("%s:%s\n", __func__, "hello-world");
	ops->exit(0);
}
