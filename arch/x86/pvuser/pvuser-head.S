/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/init.h>
#include <linux/linkage.h>

#include <asm/boot.h>
#include <asm/asm.h>
#include <asm/msr.h>
#include <asm/page_types.h>
#include <asm/percpu.h>
#include <asm/unwind_hints.h>

ENTRY(startup_pvuser_64)
	UNWIND_HINT_EMPTY
	/* save arguments  */
	mov %rdi, %r12
	mov %rsi, %r13
	cld
	/* Clear .bss */
	xor %rax,%rax
	mov $__bss_start, %_ASM_DI
	mov $__bss_stop, %_ASM_CX
	sub %_ASM_DI, %_ASM_CX
	shr $__ASM_SEL(2, 3), %_ASM_CX
	rep __ASM_SIZE(stos)

	mov $init_thread_union+THREAD_SIZE, %_ASM_SP

	mov %r12, %rdi
	mov %r13, %rsi
	jmp pvuser_start_kernel
END(startup_pvuser_64)
