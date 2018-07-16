#ifndef _ARCH_X86_PVUSER_PV_H_
#define _ARCH_X86_PVUSER_PV_H_

extern void __init hyperu_init_irq_ops(void);
extern void __init hyperu_init_mmu_ops(void);
extern void __init hyperu_reserve_top(void);

extern void __init hyperu_setup(void);

#endif
