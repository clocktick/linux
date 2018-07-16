#ifndef ASM_X86_HYPERU_H_
#define ASM_X86_HYPERU_H_

#include <linux/types.h>
#include <stdarg.h>

extern int hyperu_rdmsr(unsigned int msr, u64 *val);
extern int hyperu_wrmsr(unsigned int msr, u32 low, u32 high);
extern int hyperu_set_iopl(int level);

extern void hyperu_raw_log(const char *fmt, va_list ap);
extern void hyperu_raw_printk(const char *fmt, ...);
extern void hyperu_early_console_init(int keep);

#endif
