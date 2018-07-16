#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <asm/msr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hyperu_user.h"

static int *cpu_msr_fd;
static int max_cpu;

struct ioc_regs8 {
	__u32 eax;
	__u32 ecx;
	__u32 edx;
	__u32 ebx;

	__u32 esp;
	__u32 ebp;
	__u32 esi;
	__u32 edi;
};

int rdmsr(long cpu, long msr, long *dataptr)
{
	struct ioc_regs8 r8;
	int rc;

	if (max_cpu <= 0)
		return -ENODEV;
	if (cpu >= max_cpu || !dataptr)
		return -EINVAL;

	memset(&r8, 0, sizeof(r8));
	r8.edi = msr;

	rc = ioctl(cpu_msr_fd[cpu], X86_IOC_RDMSR_REGS, &r8);
	if (rc < 0) {
		fprintf(stderr, "RDMSR %lx on cpu%ld: %s\n", msr, cpu, strerror(errno));
		return -errno;
	}
	*dataptr = ((long)r8.edx << 32) | r8.eax;
	return 0;
}

int wrmsr(long cpu, long msr, long data)
{
	struct ioc_regs8 r8;
	int rc;

	if (max_cpu <= 0)
		return -ENODEV;
	if (cpu >= max_cpu)
		return -EINVAL;

	memset(&r8, 0, sizeof(r8));
	r8.edi = msr;
	r8.eax = data & -1U;
	r8.edx = data >> 32;

	rc = ioctl(cpu_msr_fd[cpu], X86_IOC_WRMSR_REGS, &r8);
	if (rc < 0)
		return -errno;
	return 0;
}

static int msr_init(struct hyperu *hyperu)
{
	int cpu;

	max_cpu = sysconf(_SC_NPROCESSORS_CONF);
	if (max_cpu < 0) {
		fprintf(stderr, "sysconf(_SC_NPROCESSORS_CONF) return %d\n", max_cpu);
		return -1;
	}

	cpu_msr_fd = malloc(max_cpu * sizeof(int));
	if (!cpu_msr_fd) {
		fprintf(stderr, "%s: no memory\n", __func__);
		return -1;
	}

	for (cpu=0; cpu<max_cpu; cpu++) {
		char devname[64];

		snprintf(devname, 64, "/dev/cpu/%d/msr", cpu);
		cpu_msr_fd[cpu] = open(devname, O_RDWR);
		if (cpu_msr_fd[cpu] < 0) {
			fprintf(stderr, "failed to open device %s:%s\n", devname, strerror(errno));
			goto err;
		}
	}

	hyperu->ops->x86.read_msr = &rdmsr;
	hyperu->ops->x86.write_msr = &wrmsr;
	fprintf(stderr, "init:msr OK\n");

	return 0;
err:
	cpu--;
	while (cpu>=0) {
		close(cpu_msr_fd[cpu]);
	}
	free(cpu_msr_fd);
	return -1;
}

static int msr_exit(struct hyperu *hyperu)
{
	int cpu, nr_cpu;

	nr_cpu = max_cpu;
	max_cpu = -1;

	for (cpu=0; cpu<nr_cpu; cpu++) {
		close(cpu_msr_fd[cpu]);
	}
	free(cpu_msr_fd);
	return 0;
}

arch_init(msr_init);
arch_exit(msr_exit);
