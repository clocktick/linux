#include <asm/e820.h>
#include <asm/bootparam.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hyperu_user.h"

static const char *e820_type2name[] = {
	[E820_RAM] = "RAM",
	[E820_RESERVED] = "RESERVED",
	[E820_ACPI] = "ACPI",
	[E820_NVS] = "NVS",
	[E820_UNUSABLE] = "UNUSABLE",
	[E820_PMEM] = "PMEM",
};

int hyperu_init_arch(struct hyperu *hyperu, unsigned long flags)
{
	int rc, i;
	struct boot_params *bp;

	rc = elf_symbol_address(hyperu->elf, "boot_params", (unsigned long*)&bp);
	if (rc < 0)
		return -1;
	memset(bp, 0, sizeof(struct boot_params));

	if (INIT_F_VERBOSE & flags)
		fprintf(stderr, "E820 mapping@%p:\n", &bp->e820_table[0]);
	i = 0;
#define TE(a,s,t) \
	({\
	 	if (INIT_F_VERBOSE & flags) \
		 	fprintf(stderr, "%d. [%016x - %016x] %s\n", \
				i, a, s, e820_type2name[t]); \
		(struct boot_e820_entry){a, s, t};\
	})
	bp->e820_table[i++] = TE(0000000000, 0x00010000, E820_RAM);
	bp->e820_table[i++] = TE(0x00010000, 0x00100000, E820_RESERVED);
	bp->e820_table[i++] = TE(0x00100000, 0x40110000, E820_RAM);
	bp->e820_entries = i;
#undef TE
	return 0;
}
