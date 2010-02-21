/*
	ADDRSPAC.h

	Copyright (C) 2004 Bernd Schmidt, Philip Cummins, Paul Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

#ifdef ADDRSPAC_H
#error "header already included"
#else
#define ADDRSPAC_H
#endif

EXPORTPROC Memory_Reset(void);

/*
	mapping of address space to real memory
*/

EXPORTVAR(ui3b *, BankReadAddr[NumMemBanks])
EXPORTVAR(ui3b *, BankWritAddr[NumMemBanks]) /* if BankWritAddr[i] != NULL then BankWritAddr[i] == BankReadAddr[i] */

EXPORTFUNC ui3b *get_real_address(CPTR addr);

/*
	memory access routines that can use when have address
	that is known to be in RAM (and that is in the first
	copy of the ram, not the duplicates, i.e. < kRAM_Size).
*/

#define get_ram_byte(addr) do_get_mem_byte((addr)+(ui3p)RAM)
#define get_ram_word(addr) do_get_mem_word((addr)+(ui3p)RAM)
#define get_ram_long(addr) do_get_mem_long((addr)+(ui3p)RAM)

#define put_ram_byte(addr, b) do_put_mem_byte((addr)+(ui3p)RAM, (b))
#define put_ram_word(addr, w) do_put_mem_word((addr)+(ui3p)RAM, (w))
#define put_ram_long(addr, l) do_put_mem_long((addr)+(ui3p)RAM, (l))

#define get_ram_address(addr) ((addr)+(ui3p)RAM)

#define kRAM_Size 0x00400000
#define kROM_Size 0x020000
