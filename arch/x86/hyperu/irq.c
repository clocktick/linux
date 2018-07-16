// SPDX-License-Identifier: GPL-2.0
#include <linux/hardirq.h>

#include <asm/x86_init.h>

asmlinkage __visible unsigned long hyperu_save_fl(void)
{
	return 0UL;
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_save_fl);

__visible void hyperu_restore_fl(unsigned long flags)
{
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_restore_fl);

asmlinkage __visible void hyperu_irq_disable(void)
{
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_irq_disable);

asmlinkage __visible void hyperu_irq_enable(void)
{
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_irq_enable);

static void hyperu_safe_halt(void)
{
	BUG();
}

static void hyperu_halt(void)
{
	hyperu_safe_halt();
}

static const struct pv_irq_ops hyperu_irq_ops __initconst = {
	.save_fl = PV_CALLEE_SAVE(hyperu_save_fl),
	.restore_fl = PV_CALLEE_SAVE(hyperu_restore_fl),
	.irq_disable = PV_CALLEE_SAVE(hyperu_irq_disable),
	.irq_enable = PV_CALLEE_SAVE(hyperu_irq_enable),

	.safe_halt = hyperu_safe_halt,
	.halt = hyperu_halt,
};

void __init hyperu_init_IRQ(void)
{
}

void __init hyperu_init_irq_ops(void)
{
	pv_irq_ops = hyperu_irq_ops;
	x86_init.irqs.intr_init = hyperu_init_IRQ;
}
