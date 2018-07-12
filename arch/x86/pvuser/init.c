#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/percpu.h>
#include <linux/start_kernel.h>
#include <linux/io.h>
#include <linux/memblock.h>
#include <linux/mem_encrypt.h>

#include <asm/processor.h>
#include <asm/proto.h>
#include <asm/smp.h>
#include <asm/setup.h>
#include <asm/desc.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/sections.h>
#include <asm/kdebug.h>
#include <asm/e820/api.h>
#include <asm/bios_ebda.h>
#include <asm/bootparam_utils.h>
#include <asm/microcode.h>
#include <asm/kasan.h>
#include <asm/unistd.h>
#include <asm/prctl.h>

#include <uapi/asm/udrv_kern.h>

void __init pvuser_start_kernel(struct udrv_ops *ops, char *cmdline)
{
	long rc;

	if (cmdline) {
		strncpy(boot_command_line, cmdline, COMMAND_LINE_SIZE);
		boot_command_line[COMMAND_LINE_SIZE-1] = 0;
	} else {
		boot_command_line[0] = 0;
	}
	ops->log("%s:ops=%p cmdline='%s'\n",
		 __func__, ops, boot_command_line);

	rc = ops->syscall2(__NR_arch_prctl,
			(long)ARCH_SET_GS,
			(long)&per_cpu(irq_stack_union, 0));
	ops->log("x86/prctl(ARCH_SET_GS):%ld\n", rc);
	if (rc < 0)
		ops->exit(-1);

	memset(&boot_params, 0, sizeof boot_params);
	sanitize_boot_params(&boot_params);

	start_kernel();
	ops->exit(0);
}
