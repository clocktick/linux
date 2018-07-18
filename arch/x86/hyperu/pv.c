// SPDX-License-Identifier: GPL-2.0
/*
 * Core of paravirt_ops implementation.
 *
 * This file contains the hyperu_paravirt_ops structure itself, and the
 * implementations for:
 * - privileged instructions
 * - interrupt flags
 * - segment operations
 * - booting and setup
 *
 * Li Yu <raise.sail@gmail.com> 2018
 */

#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/preempt.h>
#include <linux/hardirq.h>
#include <linux/percpu.h>
#include <linux/delay.h>
#include <linux/start_kernel.h>
#include <linux/sched.h>
#include <linux/kprobes.h>
#include <linux/bootmem.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <linux/page-flags.h>
#include <linux/highmem.h>
#include <linux/console.h>
#include <linux/pci.h>
#include <linux/gfp.h>
#include <linux/memblock.h>
#include <linux/edd.h>
#include <linux/frame.h>

#include <asm/paravirt.h>
#include <asm/apic.h>
#include <asm/page.h>
#include <asm/fixmap.h>
#include <asm/processor.h>
#include <asm/proto.h>
#include <asm/msr-index.h>
#include <asm/traps.h>
#include <asm/setup.h>
#include <asm/desc.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/reboot.h>
#include <asm/stackprotector.h>
#include <asm/hypervisor.h>
#include <asm/mach_traps.h>
#include <asm/mwait.h>
#include <asm/pci_x86.h>
#include <asm/cpu.h>
#include <asm/e820/api.h>

#ifdef CONFIG_ACPI
#include <linux/acpi.h>
#include <asm/acpi.h>
#include <acpi/pdc_intel.h>
#include <acpi/processor.h>
#endif

#include "../kernel/cpu/cpu.h" /* get_cpu_cap() */

#include <asm/hyperu.h>
#include "pv.h"

static void __init hyperu_banner(void)
{
	pr_info("Booting paravirtualized kernel on %s\n", pv_info.name);
	printk(KERN_INFO "HyperU version: %d.%d\n", 1, 0);
}

static void hyperu_cpuid(unsigned int *ax, unsigned int *bx,
		      unsigned int *cx, unsigned int *dx)
{
	unsigned maskebx = ~0;

	asm("cpuid"
		: "=a" (*ax),
		  "=b" (*bx),
		  "=c" (*cx),
		  "=d" (*dx)
		: "0" (*ax), "2" (*cx));

	*bx &= maskebx;
}

static void __init hyperu_init_capabilities(void)
{
#if 0
	setup_clear_cpu_cap(X86_FEATURE_DCA);
	setup_clear_cpu_cap(X86_FEATURE_APERFMPERF);
	setup_clear_cpu_cap(X86_FEATURE_MTRR);
	setup_clear_cpu_cap(X86_FEATURE_ACC);
	setup_clear_cpu_cap(X86_FEATURE_X2APIC);
	setup_clear_cpu_cap(X86_FEATURE_SME);
	setup_clear_cpu_cap(X86_FEATURE_PCID);

	setup_force_cpu_cap(X86_FEATURE_MWAIT);
#endif
	/* xsetbv() faults in fpu__init_cpu_xstate() */
	setup_clear_cpu_cap(X86_FEATURE_XSAVE);
	setup_clear_cpu_cap(X86_FEATURE_OSXSAVE);
}

static void hyperu_set_debugreg(int reg, unsigned long val)
{
}

static unsigned long hyperu_get_debugreg(int reg)
{
	return 0UL;
}

static void hyperu_end_context_switch(struct task_struct *next)
{
}

static unsigned long hyperu_store_tr(void)
{
	return 0;
}

static void hyperu_alloc_ldt(struct desc_struct *ldt, unsigned entries)
{
}

static void hyperu_free_ldt(struct desc_struct *ldt, unsigned entries)
{
}

static void hyperu_set_ldt(const void *addr, unsigned entries)
{
}

static void hyperu_load_gdt(const struct desc_ptr *dtr)
{
}

static void hyperu_load_tls(struct thread_struct *t, unsigned int cpu)
{
}

#ifdef CONFIG_X86_64
static void hyperu_load_gs_index(unsigned int idx)
{
}
#endif

static void hyperu_write_ldt_entry(struct desc_struct *dt, int entrynum,
				const void *ptr)
{
}


static void hyperu_write_idt_entry(gate_desc *dt, int entrynum, const gate_desc *g)
{
	//static DEFINE_PER_CPU(struct desc_ptr, idt_desc);
}

/* Load a new IDT into HyperU.  In principle this can be per-CPU, so we
   hold a spinlock to protect the static traps[] array (static because
   it avoids allocation, and saves stack space). */
static void hyperu_load_idt(const struct desc_ptr *desc)
{
}

/* Write a GDT descriptor entry.  Ignore LDT descriptors, since
   they're handled differently. */
static void hyperu_write_gdt_entry(struct desc_struct *dt, int entry,
				const void *desc, int type)
{
}

static void hyperu_load_sp0(unsigned long sp0)
{
	this_cpu_write(cpu_tss_rw.x86_tss.sp0, sp0);
}

static void hyperu_io_delay(void)
{
}


static unsigned long hyperu_read_cr0(void)
{
	//static DEFINE_PER_CPU(unsigned long, hyperu_cr0_value);
	return -1UL;
}

static void hyperu_write_cr0(unsigned long cr0)
{
}

static void hyperu_write_cr4(unsigned long cr4)
{
}

#ifdef CONFIG_X86_64
static inline unsigned long hyperu_read_cr8(void)
{
	return -1UL;
}
static inline void hyperu_write_cr8(unsigned long val)
{
	BUG_ON(val);
}
#endif

void hyperu_set_iopl_mask(unsigned mask)
{
	int iopl;

	iopl = (mask == 0) ? 1 : (mask >> 12) & 3;
	hyperu_set_iopl(iopl);
}

static u64 hyperu_read_msr_safe(unsigned int msr, int *err)
{
	u64 data;

	*err = hyperu_rdmsr(msr, &data);
	return data;
}

static int hyperu_write_msr_safe(unsigned int msr, unsigned low, unsigned high)
{
	return hyperu_wrmsr(msr, low, high);
}

static u64 hyperu_read_msr(unsigned int msr)
{
	int err;

	return hyperu_read_msr_safe(msr, &err);
}

static void hyperu_write_msr(unsigned int msr, unsigned low, unsigned high)
{
	hyperu_write_msr_safe(msr, low, high);
}

static const struct pv_info hyperu_info __initconst = {
	.shared_kernel_pmd = 0,
	.name = "HyperU",
};

static u64 hyperu_read_pmc(int counter)
{
	return 0;
}

static void hyperu_iret(void )
{
}

static void hyperu_sysret64(void)
{
}

static const struct pv_cpu_ops hyperu_cpu_ops __initconst = {
	.cpuid = hyperu_cpuid,

	.set_debugreg = hyperu_set_debugreg,
	.get_debugreg = hyperu_get_debugreg,

	.read_cr0 = hyperu_read_cr0,
	.write_cr0 = hyperu_write_cr0,

	.write_cr4 = hyperu_write_cr4,

#ifdef CONFIG_X86_64
	.read_cr8 = hyperu_read_cr8,
	.write_cr8 = hyperu_write_cr8,
#endif

	.wbinvd = native_wbinvd,

	.read_msr = hyperu_read_msr,
	.write_msr = hyperu_write_msr,

	.read_msr_safe = hyperu_read_msr_safe,
	.write_msr_safe = hyperu_write_msr_safe,

	.read_pmc = hyperu_read_pmc,

	.iret = hyperu_iret,
#ifdef CONFIG_X86_64
	.usergs_sysret64 = hyperu_sysret64,
#endif

	.load_tr_desc = paravirt_nop,
	.set_ldt = hyperu_set_ldt,
	.load_gdt = hyperu_load_gdt,
	.load_idt = hyperu_load_idt,
	.load_tls = hyperu_load_tls,
#ifdef CONFIG_X86_64
	.load_gs_index = hyperu_load_gs_index,
#endif

	.alloc_ldt = hyperu_alloc_ldt,
	.free_ldt = hyperu_free_ldt,

	.store_tr = hyperu_store_tr,

	.write_ldt_entry = hyperu_write_ldt_entry,
	.write_gdt_entry = hyperu_write_gdt_entry,
	.write_idt_entry = hyperu_write_idt_entry,
	.load_sp0 = hyperu_load_sp0,

	.set_iopl_mask = hyperu_set_iopl_mask,
	.io_delay = hyperu_io_delay,

	/* HyperU takes care of %gs when switching to usermode for us */
	.swapgs = paravirt_nop,

	.start_context_switch = paravirt_start_context_switch,
	.end_context_switch = hyperu_end_context_switch,
};

static void hyperu_restart(char *msg)
{
	//hyperu_reboot(SHUTDOWN_reboot);
}

static void hyperu_machine_halt(void)
{
	//hyperu_reboot(SHUTDOWN_poweroff);
}

static void hyperu_machine_power_off(void)
{
	if (pm_power_off)
		pm_power_off();
	//hyperu_reboot(SHUTDOWN_poweroff);
}

static void hyperu_crash_shutdown(struct pt_regs *regs)
{
	//hyperu_reboot(SHUTDOWN_crash);
}

static void hyperu_emergency_restart(void)
{
}

static const struct machine_ops hyperu_machine_ops __initconst = {
	.restart = hyperu_restart,
	.halt = hyperu_machine_halt,
	.power_off = hyperu_machine_power_off,
	.shutdown = hyperu_machine_halt,
	.crash_shutdown = hyperu_crash_shutdown,
	.emergency_restart = hyperu_emergency_restart,
};

static unsigned char hyperu_get_nmi_reason(void)
{
	unsigned char reason = 0;
	return reason;
}

static void __init hyperu_set_legacy_features(void)
{
	x86_platform.legacy.rtc = 1;
}

static void __init setup_features(void)
{
}

char * __init hyperu_memory_setup(void)
{
	return e820__memory_setup_default();
}

void __init hyperu_probe_roms(void)
{
	//video roms
	//system roms
}

void __init hyperu_arch_setup(void)
{
}

void __init hyperu_init_apic(void)
{
}

void __init hyperu_smp_init(void)
{
}

int __init pci_hyperu_init(void)
{
	return 0;
}

void __init hyperu_setup(void)
{
	int rc;

	setup_features(); // to probe hypervisor version and features

	pv_info = hyperu_info;
	pv_init_ops.patch = paravirt_patch_default;
	pv_cpu_ops = hyperu_cpu_ops;

	x86_platform.get_nmi_reason = hyperu_get_nmi_reason;

	x86_init.resources.probe_roms = hyperu_probe_roms;
	x86_init.resources.memory_setup = hyperu_memory_setup;
	x86_init.irqs.intr_mode_init	= x86_init_noop;
	x86_init.oem.arch_setup = hyperu_arch_setup;
	x86_init.oem.banner = hyperu_banner;

	/*
	 * Set up some pagetable state before starting to set any ptes.
	 */

	hyperu_init_mmu_ops();

	/* Prevent unwanted bits from being set in PTEs. */
	__supported_pte_mask &= ~_PAGE_GLOBAL;

	/*
	 * Prevent page tables from being allocated in highmem, even
	 * if CONFIG_HIGHPTE is enabled.
	 */
	__userpte_alloc_gfp &= ~__GFP_HIGHMEM;

	/* Work out if we support NX */
	get_cpu_cap(&boot_cpu_data);
	x86_configure_nx();

	hyperu_init_irq_ops();

	/*
	per_cpu(hyperu_vcpu_id, 0) = 0;
	*/

	idt_setup_early_handler();

	hyperu_init_capabilities();

#ifdef CONFIG_X86_LOCAL_APIC
	/*
	 * set up the basic apic ops.
	 */
	hyperu_init_apic();
#endif

	machine_ops = hyperu_machine_ops;
	hyperu_smp_init();

#ifdef CONFIG_ACPI_NUMA
	/*
	 * The pages we from HyperU are not related to machine pages, so
	 * any NUMA information the kernel tries to get from ACPI will
	 * be meaningless.  Prevent it from trying.
	 */
	acpi_numa = -1;
#endif

	local_irq_disable();
	early_boot_irqs_disabled = true;

	pv_info.kernel_rpl = 0;
	/* set the limit of our address space */
	hyperu_reserve_top();

	rc = hyperu_set_iopl(3);
	if (rc != 0)
		hyperu_raw_printk("hyperu_set_iopl() failed %d\n", rc);

#ifdef CONFIG_X86_32
	/* set up basic CPUID stuff */
	cpu_detect(&new_cpu_data);
	set_cpu_cap(&new_cpu_data, X86_FEATURE_FPU);
	new_cpu_data.x86_capability[CPUID_1_EDX] = cpuid_edx(1);
#endif

	/* Poke various useful things into boot_params */
	boot_params.hdr.type_of_loader = (9 << 4) | 0;
	boot_params.hdr.hardware_subarch = X86_SUBARCH_HYPERU;

	{
		x86_platform.set_legacy_features =
				hyperu_set_legacy_features;

		x86_init.pci.arch_init = pci_hyperu_init;
		/* Make sure ACS will be enabled */
		pci_request_acs();
		//hyperu_acpi_sleep_register();

		/* Avoid searching for BIOS MP tables */
		x86_init.mpparse.find_smp_config = x86_init_noop;
		x86_init.mpparse.get_smp_config = x86_init_uint_noop;
	}

	add_preferred_console("hyperu", 0, NULL);

#ifdef CONFIG_PCI
	/* PCI BIOS service won't work from a PV guest. */
	pci_probe &= ~PCI_PROBE_BIOS;
#endif

	/* We need this for printk timestamps */
	//hyperu_setup_runstate_info(0);
}

static uint32_t __init hyperu_platform_pv(void)
{
	return 0;
}

static void hyperu_pin_vcpu(int cpu)
{
}

const __initconst struct hypervisor_x86 x86_hyper_hyperu_pv = {
	.name                   = "hyperu PV",
	.detect                 = hyperu_platform_pv,
	.type			= X86_HYPER_USER,
	.runtime.pin_vcpu       = hyperu_pin_vcpu,
};
