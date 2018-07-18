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
#include <linux/console.h>

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

#include <uapi/asm/hyperu_kern.h>

#include "pv.h"

static struct hyperu_ops *h_ops;

void hyperu_raw_log_n(char *buf, int len)
{
	h_ops->log_n(buf, len);
}

void hyperu_raw_log(const char *fmt, va_list ap)
{
	h_ops->log(fmt, ap);
}

void hyperu_raw_printk(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	hyperu_raw_log(fmt, ap);
	va_end(ap);
}

#ifdef CONFIG_EARLY_PRINTK
static void hyperu_write_console(struct console *console, const char *string,
				  unsigned len)
{
	hyperu_raw_log_n((char*)string, len);
}

static struct console early_hyperu_console = {
	.name		= "hyperu",
	.write		= hyperu_write_console,
	.flags		= CON_PRINTBUFFER | CON_BOOT | CON_ANYTIME,
	.index		= -1,
};

void __init hyperu_early_console_init(int keep)
{
	register_console(&early_hyperu_console);
}

#endif	/* CONFIG_EARLY_PRINTK */

int hyperu_set_iopl(int level)
{
	return h_ops->x86.set_iopl(level);
}

int hyperu_rdmsr(unsigned int msr, u64 *data)
{
	return h_ops->x86.read_msr(smp_processor_id(), msr, (long*)data);
}

int hyperu_wrmsr(unsigned int msr, u32 low, u32 high)
{
	long data;

	data = low | ((long)high<<32);
	return h_ops->x86.write_msr(smp_processor_id(), msr, data);
}

void __init hyperu_start_kernel(struct hyperu_ops *ops, char *cmdline)
{
	long rc;

	h_ops = ops;

	if (cmdline) {
		strncpy(boot_command_line, cmdline, COMMAND_LINE_SIZE);
		boot_command_line[COMMAND_LINE_SIZE-1] = 0;
	} else {
		boot_command_line[0] = 0;
	}
	rc = ops->syscall2(__NR_arch_prctl,
			(long)ARCH_SET_GS,
			(long)&per_cpu(irq_stack_union, 0));
	if (rc < 0) {
		ops->log_n("arch_prctl(ARCH_SET_GS) failed\n", 0);
		ops->exit(-1);
	}
	hyperu_raw_printk("kernel cmdline='%s'\n", boot_command_line);
	sanitize_boot_params(&boot_params);

	hyperu_setup();
	start_kernel();
	ops->exit(0);
}
