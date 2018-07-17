#include <asm/bootparam.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hyperu_user.h"

int arch_init_hyperu(struct hyperu *hyperu)
{
	int rc, i;
	struct boot_params *bp;

	rc = elf_symbol_address(hyperu->elf, "boot_params", (unsigned long*)&bp);
	if (rc < 0)
		return -1;
	memset(bp, 0, sizeof(struct boot_params));

	i = 0;
#define TE(a,s,t) (struct e820entry){a, s, t}
	bp->e820_map[i++] = TE(0,0xc0000000,E820_RAM);
	bp->e820_entries = i;
#undef TE
	return 0;
}
