#include <linux/sched/mm.h>
#include <linux/highmem.h>
#include <linux/debugfs.h>
#include <linux/bug.h>
#include <linux/vmalloc.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/gfp.h>
#include <linux/memblock.h>
#include <linux/seq_file.h>
#include <linux/crash_dump.h>
#ifdef CONFIG_KEXEC_CORE
#include <linux/kexec.h>
#endif

#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/fixmap.h>
#include <asm/mmu_context.h>
#include <asm/setup.h>
#include <asm/paravirt.h>
#include <asm/e820/api.h>
#include <asm/linkage.h>
#include <asm/page.h>
#include <asm/init.h>
#include <asm/pat.h>
#include <asm/smp.h>

#include "pv.h"

static void hyperu_set_pmd_hyper(pmd_t *ptr, pmd_t val)
{
}

static inline void __hyperu_set_pte(pte_t *ptep, pte_t pteval)
{
}

static void hyperu_set_pte_at(struct mm_struct *mm, unsigned long addr,
		    pte_t *ptep, pte_t pteval)
{
}

__visible pteval_t hyperu_pte_val(pte_t pte)
{
	return (pteval_t)0;
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_pte_val);

__visible pgdval_t hyperu_pgd_val(pgd_t pgd)
{
	return (pgdval_t)0;
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_pgd_val);

__visible pte_t hyperu_make_pte(pteval_t pte)
{
	return native_make_pte(pte);
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_make_pte);

__visible pgd_t hyperu_make_pgd(pgdval_t pgd)
{
	return native_make_pgd(pgd);
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_make_pgd);

__visible pmdval_t hyperu_pmd_val(pmd_t pmd)
{
	return (pmdval_t)0;
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_pmd_val);

static void hyperu_set_pud(pud_t *ptr, pud_t val)
{
}

#ifdef CONFIG_X86_PAE
static void hyperu_set_pte_atomic(pte_t *ptep, pte_t pte)
{
}

static void hyperu_pte_clear(struct mm_struct *mm, unsigned long addr, pte_t *ptep)
{
}

static void hyperu_pmd_clear(pmd_t *pmdp)
{
	trace_hyperu_mmu_pmd_clear(pmdp);
	set_pmd(pmdp, __pmd(0));
}
#endif	/* CONFIG_X86_PAE */

__visible pmd_t hyperu_make_pmd(pmdval_t pmd)
{
	return native_make_pmd(pmd);
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_make_pmd);

#ifdef CONFIG_X86_64
__visible pudval_t hyperu_pud_val(pud_t pud)
{
	return 0;
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_pud_val);

__visible pud_t hyperu_make_pud(pudval_t pud)
{
	return native_make_pud(pud);
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_make_pud);

static void __init hyperu_set_p4d_hyper(p4d_t *ptr, p4d_t val)
{
}

#if CONFIG_PGTABLE_LEVELS >= 5
__visible p4dval_t hyperu_p4d_val(p4d_t p4d)
{
	return 0;
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_p4d_val);

__visible p4d_t hyperu_make_p4d(p4dval_t p4d)
{
	return 0;
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_make_p4d);
#endif  /* CONFIG_PGTABLE_LEVELS >= 5 */
#endif	/* CONFIG_X86_64 */

static void hyperu_activate_mm(struct mm_struct *prev, struct mm_struct *next)
{
	spin_lock(&next->page_table_lock);
	spin_unlock(&next->page_table_lock);
}

static void hyperu_dup_mmap(struct mm_struct *oldmm, struct mm_struct *mm)
{
	spin_lock(&mm->page_table_lock);
	spin_unlock(&mm->page_table_lock);
}

static void hyperu_exit_mmap(struct mm_struct *mm)
{
}

static void __init hyperu_pagetable_init(void)
{
	paging_init();
}

static void hyperu_write_cr2(unsigned long cr2)
{
}

static unsigned long hyperu_read_cr2(void)
{
	return 0;
}

unsigned long hyperu_read_cr2_direct(void)
{
	return 0;
}

static noinline void hyperu_flush_tlb(void)
{
}

static void hyperu_flush_tlb_one_user(unsigned long addr)
{
}

static void hyperu_flush_tlb_others(const struct cpumask *cpus,
				 const struct flush_tlb_info *info)
{
}

static unsigned long hyperu_read_cr3(void)
{
	return 0;
}

static void __init hyperu_write_cr3_init(unsigned long cr3)
{
}

static int hyperu_pgd_alloc(struct mm_struct *mm)
{
	return 0;
}

static void hyperu_pgd_free(struct mm_struct *mm, pgd_t *pgd)
{
}

__visible pte_t hyperu_make_pte_init(pteval_t pte)
{
	return native_make_pte(pte);
}
PV_CALLEE_SAVE_REGS_THUNK(hyperu_make_pte_init);

static void __init hyperu_set_pte_init(pte_t *ptep, pte_t pte)
{
	native_set_pte(ptep, pte);
}

/* Early in boot, while setting up the initial pagetable, assume
   everything is pinned. */
static void __init hyperu_alloc_pte_init(struct mm_struct *mm, unsigned long pfn)
{
}

/* Used for pmd and pud */
static void __init hyperu_alloc_pmd_init(struct mm_struct *mm, unsigned long pfn)
{
}

/* Early release_pte assumes that all pts are pinned, since there's
   only init_mm and anything attached to that is pinned. */
static void __init hyperu_release_pte_init(unsigned long pfn)
{
}

static void __init hyperu_release_pmd_init(unsigned long pfn)
{
}

void __init hyperu_reserve_top(void)
{
}

static void hyperu_set_fixmap(unsigned idx, phys_addr_t phys, pgprot_t prot)
{
}

static void hyperu_leave_lazy_mmu(void)
{
}

static const struct pv_mmu_ops hyperu_mmu_ops __initconst = {
	.read_cr2 = hyperu_read_cr2,
	.write_cr2 = hyperu_write_cr2,

	.read_cr3 = hyperu_read_cr3,
	.write_cr3 = hyperu_write_cr3_init,

	.flush_tlb_user = hyperu_flush_tlb,
	.flush_tlb_kernel = hyperu_flush_tlb,
	.flush_tlb_one_user = hyperu_flush_tlb_one_user,
	.flush_tlb_others = hyperu_flush_tlb_others,

	.pgd_alloc = hyperu_pgd_alloc,
	.pgd_free = hyperu_pgd_free,

	.alloc_pte = hyperu_alloc_pte_init,
	.release_pte = hyperu_release_pte_init,
	.alloc_pmd = hyperu_alloc_pmd_init,
	.release_pmd = hyperu_release_pmd_init,

	.set_pte = hyperu_set_pte_init,
	.set_pte_at = hyperu_set_pte_at,
	.set_pmd = hyperu_set_pmd_hyper,

	.ptep_modify_prot_start = __ptep_modify_prot_start,
	.ptep_modify_prot_commit = __ptep_modify_prot_commit,

	.pte_val = PV_CALLEE_SAVE(hyperu_pte_val),
	.pgd_val = PV_CALLEE_SAVE(hyperu_pgd_val),

	.make_pte = PV_CALLEE_SAVE(hyperu_make_pte_init),
	.make_pgd = PV_CALLEE_SAVE(hyperu_make_pgd),

#ifdef CONFIG_X86_PAE
	.set_pte_atomic = hyperu_set_pte_atomic,
	.pte_clear = hyperu_pte_clear,
	.pmd_clear = hyperu_pmd_clear,
#endif	/* CONFIG_X86_PAE */
	.set_pud = hyperu_set_pud,

	.make_pmd = PV_CALLEE_SAVE(hyperu_make_pmd),
	.pmd_val = PV_CALLEE_SAVE(hyperu_pmd_val),

#ifdef CONFIG_X86_64
	.pud_val = PV_CALLEE_SAVE(hyperu_pud_val),
	.make_pud = PV_CALLEE_SAVE(hyperu_make_pud),
	.set_p4d = hyperu_set_p4d_hyper,

	.alloc_pud = hyperu_alloc_pmd_init,
	.release_pud = hyperu_release_pmd_init,

#if CONFIG_PGTABLE_LEVELS >= 5
	.p4d_val = PV_CALLEE_SAVE(hyperu_p4d_val),
	.make_p4d = PV_CALLEE_SAVE(hyperu_make_p4d),
#endif
#endif	/* CONFIG_X86_64 */

	.activate_mm = hyperu_activate_mm,
	.dup_mmap = hyperu_dup_mmap,
	.exit_mmap = hyperu_exit_mmap,

	.lazy_mode = {
		.enter = paravirt_enter_lazy_mmu,
		.leave = hyperu_leave_lazy_mmu,
		.flush = paravirt_flush_lazy_mmu,
	},

	.set_fixmap = hyperu_set_fixmap,
};

static void __init hyperu_after_bootmem(void)
{
}

void __init hyperu_init_mmu_ops(void)
{
	x86_init.paging.pagetable_init = hyperu_pagetable_init;
	x86_init.hyper.init_after_bootmem = hyperu_after_bootmem;

	pv_mmu_ops = hyperu_mmu_ops;
}
