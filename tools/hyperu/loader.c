#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <execinfo.h>

#include <gelf.h>
#include <libelf.h>

#include "hyperu_user.h"

#define PAGE_SIZE	4096
#define ALIGN_UP(x,a)	(((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x,a) ALIGN_UP((x) - ((a) - 1), (a))

static Elf *open_kelf(char *fname, int *fd)
{
	Elf *kelf;
	GElf_Ehdr eh;

	if (!fname || !fd)
		return NULL;

	*fd = open(fname, O_RDONLY);
	if (*fd < 0) {
		fprintf(stderr, "failed to open ELF image %s: %s\n", fname, strerror(errno));
		return NULL;
	}
	elf_version(EV_CURRENT);
	kelf = elf_begin(*fd, ELF_C_READ, NULL);
	if (!kelf) {
		close(*fd);
		*fd = -1;
		fprintf(stderr, "failed to open ELF image %s: %s\n", fname, elf_errmsg(0));
		return NULL;
	}
	if (ELFCLASS64 != gelf_getclass(kelf)) {
		fprintf(stderr, "only support 64 bits ELF yet\n");
		goto err;
	}
	if (!gelf_getehdr(kelf, &eh)) {
		fprintf(stderr, "failed to read Ehdr: %s\n", elf_errmsg(0));
		goto err;
	}
	switch (eh.e_machine) {
	case EM_X86_64:
		break;
	default:
		fprintf(stderr, "unsupported machine type %x\n", eh.e_machine);
		goto err;
	}
	if (ET_EXEC != eh.e_type) {
		fprintf(stderr, "unsupported ELF type %x\n", eh.e_type);
		goto err;
	}
	return kelf;
err:
	elf_end(kelf);
	close(*fd);
	*fd = -1;
	return NULL;
}

static int elf_map_seg(Elf *kelf, int fd, size_t s, unsigned long flags)
{
	GElf_Phdr *phdr, mem;
	int prot;
	size_t bytes;
	off_t off;
	void *addr;

	phdr = gelf_getphdr (kelf, s, &mem);
	if (!phdr) {
		fprintf(stderr, "%s:%s\n", __func__, elf_errmsg(0));
		return -1;
	}
	if (PT_LOAD != phdr->p_type)
		return 0;
	phdr = gelf_getphdr (kelf, s, &mem);
	if (INIT_F_VERBOSE & flags)
		fprintf (stderr,
		      "%ld. type:%x offset:%016lx vaddr:%016lx paddr:%016lx\n"
		      "   filesz:%016lx memsz:%016lx flags:%08x, align:%08lx\n",
		      s,
		      phdr->p_type,
		      phdr->p_offset,
		      phdr->p_vaddr,
		      phdr->p_paddr,
		      phdr->p_filesz,
		      phdr->p_memsz,
		      phdr->p_flags,
		      phdr->p_align);

	bytes = ALIGN_UP(phdr->p_memsz, PAGE_SIZE);
	off = ALIGN_DOWN(phdr->p_offset, PAGE_SIZE);

	prot = 0;
	if (PF_X & phdr->p_flags)
		prot |= PROT_EXEC;
	if (PF_R & phdr->p_flags)
		prot |= PROT_READ;
//	if (PF_W & phdr->p_flags)
		prot |= PROT_WRITE;

	flags = MAP_FIXED;
	if (prot & PROT_WRITE)
		flags |= MAP_PRIVATE;
	else
		flags |= MAP_SHARED;
	addr = mmap((void*)phdr->p_vaddr, bytes, prot, flags, fd, off);
	if (MAP_FAILED == addr) {
		fprintf(stderr, "mmap():%s\n", strerror(errno));
		return -1;
	}
	if (addr != (void*)phdr->p_vaddr) {
		fprintf(stderr, "loaded address is %p, expected address is %p\n",
				addr, (void*)phdr->p_vaddr);
		return -1;
	}

	return 0;
}

int elf_symbol_address(Elf *kelf, const char *name, unsigned long *val)
{
	int n;
	Elf_Scn *scn;
	GElf_Ehdr ehdr_mem, *ehdr;
	GElf_Shdr shdr_mem, *shdr;
	Elf_Data *data;

	ehdr = gelf_getehdr (kelf, &ehdr_mem);
	if (!ehdr)
		return -1;

	n = 0;
	for (scn = elf_getscn(kelf, 0); scn; scn = elf_getscn(kelf, ++n)) {
		int class;
		size_t nsym, c;
		char *sym_name;

		shdr = gelf_getshdr (scn, &shdr_mem);
		if (!shdr)
			return -1;
		data = elf_getdata (scn, NULL);
		if (!data)
			return -1;
		if (SHT_SYMTAB != shdr->sh_type)
			continue;

		class = gelf_getclass (kelf);
		nsym =  data->d_size;
		nsym /= (class == ELFCLASS32) ? sizeof (Elf32_Sym) : sizeof (Elf64_Sym);
		for (c = 0; c < nsym ; c++) {
			GElf_Sym sym_mem;
			GElf_Sym *sym = gelf_getsym (data, c, &sym_mem);

			sym_name = elf_strptr(kelf, shdr->sh_link, sym->st_name);
			if (!sym_name)
				continue;
			if (!strcmp(name, sym_name)) {
				*val = sym->st_value;
				return 0;
			}
		}
	}
	return -1;
}

/*
static int fixup_rodata_mapping(Elf *kelf)
{
	unsigned long __start_rodata, __end_rodata;
	int rc;

	rc = elf_symbol_address(kelf, "__start_rodata", &__start_rodata);
	if (rc < 0) {
		fprintf(stderr, "failed to get address of '__start_rodata'\n");
		return rc;
	}

	rc = elf_symbol_address(kelf, "__end_rodata", &__end_rodata);
	if (rc < 0) {
		fprintf(stderr, "failed to get address of '__end_rodata'\n");
		return rc;
	}
	fprintf(stderr, "rodata: [%016lx - %016lx]\n", __start_rodata, __end_rodata);

	__start_rodata = ALIGN_DOWN(__start_rodata, PAGE_SIZE);
	__end_rodata = ALIGN_UP(__end_rodata, PAGE_SIZE);
	rc = mremap((void*)__start_rodata,
			__end_rodata - __start_rodata,
			__end_rodata - __start_rodata,
			MREMAP_FIXED,
			PROT_READ|PROT_WRITE|PROT_EXEC);
	if (rc < 0) {
		fprintf(stderr, "fixup rodata section failure: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}
*/

static int load_elf(Elf *kelf, int fd, unsigned long flags)
{
	size_t segs, s;
	int rc;

	rc = elf_getphdrnum(kelf, &segs);
	if (rc < 0) {
		fprintf(stderr, "%s:%s\n", __func__, elf_errmsg(0));
		return rc;
	}
	if (INIT_F_VERBOSE & flags)
		fprintf(stderr, "kernel ELF address layout:\n");
	for (s = 0; s < segs; s++) {
		rc = elf_map_seg(kelf, fd, s, flags);
		if (rc < 0)
			return rc;
	}

#if 0
	rc = fixup_rodata_mapping(kelf);
	if (rc < 0)
		return rc;
#endif

	return 0;
}

static void close_kelf(Elf *kelf, int fd)
{
	if (kelf)
		elf_end(kelf);
	close(fd);
}

static int hyperu_bootstrap(struct hyperu *hyperu, unsigned long flags)
{
	GElf_Ehdr eh;
	void (*entry)(struct hyperu_ops *, char*);

	if (!gelf_getehdr(hyperu->elf, &eh)) {
		fprintf(stderr, "%s:failed to read Ehdr: %s\n", __func__, elf_errmsg(0));
		return -1;
	}

	if (INIT_F_VERBOSE & flags)
		fprintf(stderr, "=====\nBooting kernel\n=====\n");

	entry = (void*)eh.e_entry;
	entry(hyperu->ops, hyperu->cmdline);
	return 0;
}

#define CMDLINE_SIZE	4096
static void construct_cmdline(int argc, char *argv[], char *cmdline, int rest)
{
	int off, n, a;

	n = off = 0;
	rest = CMDLINE_SIZE;
	for (a=0; a<argc && rest>0; a++) {
		n = snprintf(cmdline+off, rest-off, "%s ", argv[a]);
		off += n;
		rest -= n;
	}
	if (off > 0 && cmdline[off-1] == ' ')
		cmdline[off-1] = 0;
	else
		cmdline[off] = 0;
}

#if 0
static void dump_my_stack(int sig)
{
	void *array[20];
	size_t size;

	fprintf(stderr, "SIGSEGV:\n");
	size = backtrace(array, 20);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}
#endif

static int hyperu_init(struct hyperu *hyperu, char *kernel, char *cmdline, unsigned long flags)
{
	int rc, fd;
	Elf *elf;

	elf = open_kelf(kernel, &fd);
	if (!elf)
		return -1;
	rc = load_elf(elf, fd, flags);
	if (rc < 0)
		return -1;

	hyperu->kernel = kernel;
	hyperu->cmdline = cmdline;
	hyperu->ops = hyperu_get_ops();
	hyperu->elf = elf;
	hyperu->fd = fd;
	rc = init_list__init(hyperu, flags);
	if (rc < 0)
		return rc;
	rc = hyperu_init_arch(hyperu, flags);
	if (rc < 0)
		return rc;

	return rc;
}

static int hyperu_exit(struct hyperu *hyperu, unsigned long flags)
{
	int rc;

	rc = init_list__exit(hyperu, flags);
	close_kelf(hyperu->elf, hyperu->fd);
	return rc;
}

int main(int argc, char *argv[])
{
	int rc;
	char *fname;
	char cmdline[CMDLINE_SIZE];
	struct hyperu hyperu;
	unsigned long flags;

#if 0
	signal(SIGSEGV, dump_my_stack);
#endif

	if (1 == argc) {
		fname = "../../build/vmlinux";
		argc = 0;
	} else {
		fname = argv[1];
		argc -= 2;
		argv += 2;
	}
	flags = INIT_F_VERBOSE;
	construct_cmdline(argc, argv, cmdline, CMDLINE_SIZE);

	rc = hyperu_init(&hyperu, fname, cmdline, flags);
	if (rc < 0)
		return -1;
	rc = hyperu_bootstrap(&hyperu, flags);
	if (rc < 0)
		goto err;
	hyperu_exit(&hyperu, flags);

	return 0;

err:
	hyperu_exit(&hyperu, flags);
	return -1;
}
