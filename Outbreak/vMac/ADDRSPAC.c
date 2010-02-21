/*
	ADDRSPAC.c

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

/*
	ADDRess SPACe

	Implements the address space of the Mac Plus.

	This code is descended from code in vMac by Philip Cummins, in
	turn descended from code in the Un*x Amiga Emulator by
	Bernd Schmidt.
*/

#ifndef AllFiles
#include "SYSDEPNS.h"
#include "ENDIANAC.h"
#include "MINEM68K.h"
#endif

#include "ADDRSPAC.h"

#include "Gateway.h"
#include "GrayBox.h"


/* top 8 bits out of 32 are ignored, so total size of address space is 2 ** 24 bytes */

#define TotAddrBytes (1UL << ln2TotAddrBytes)
#define kAddrMask (TotAddrBytes - 1)

/* map of address space */

#define kRAM_Base 0x00000000 /* when overlay off */
#define kRAM_Top  0x00400000

#define kROM_Base 0x00400000
#define kROM_Top  0x00500000

/* implementation of read/write for everything but RAM and ROM */

/* devide address space into banks, some of which are mapped to real memory */

GLOBALVAR ui3b *BankReadAddr[NumMemBanks];
GLOBALVAR ui3b *BankWritAddr[NumMemBanks];

#define kROM_BaseBank (kROM_Base >> ln2BytesPerMemBank)
#define kROM_TopBank (kROM_Top >> ln2BytesPerMemBank)

#define kRAM_BaseBank (kRAM_Base >> ln2BytesPerMemBank)
#define kRAM_TopBank (kRAM_Top >> ln2BytesPerMemBank)

#define ROMmem_mask (kROM_Size - 1)

LOCALPROC SetUpBankRange(ui5b StartBank, ui5b StopBank, ui3b * RealStart, CPTR VirtualStart, ui5b vMask, blnr Writeable)
{
	ui5b i;

	for (i = StartBank; i < StopBank; i++) {
		BankReadAddr[i] = RealStart + (((i << ln2BytesPerMemBank) - VirtualStart) & vMask);
		if (Writeable) {
			BankWritAddr[i] = BankReadAddr[i];
		}
	}
}

LOCALPROC SetPtrVecToNULL(ui3b **x, ui5b n)
{
	int i;

	for (i = 0; i < n; i++) {
		*x++ = nullpr;
	}
}

GLOBALPROC Memory_Reset(void)
{
	ui5b RAMmem_mask = kRAM_Size - 1;

	SetPtrVecToNULL(BankReadAddr, NumMemBanks);
	SetPtrVecToNULL(BankWritAddr, NumMemBanks);

	SetUpBankRange(kROM_BaseBank, kROM_TopBank, (ui3b *) ROM, kROM_Base, ROMmem_mask, falseblnr);

    SetUpBankRange(kRAM_BaseBank, kRAM_TopBank, (ui3b *)RAM, kRAM_Base, RAMmem_mask, trueblnr);
    
	SetUpBankRange((vGateway >> ln2BytesPerMemBank),
	               ((vGateway + 0x010000) >> ln2BytesPerMemBank),
	               (ui3b *)gateway, vGateway,
	               (0x010000 - 1),
	               falseblnr);
}

/*
	unlike in the real Mac Plus, Mini vMac
	will allow misaligned memory access,
	since it is easier to allow it than
	it is to correctly simulate a bus error
	and back out of the current instruction.
*/

LOCALFUNC ui3b *default_xlate(CPTR a)
{
	UnusedParam(a);
	return (ui3b *) RAM; /* So we don't crash. */
}

GLOBALFUNC ui3b *get_real_address(CPTR addr)
{
	ui3b *ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		return (ui3b *)((addr & MemBankAddrMask) + ba);
	} else {
		return default_xlate(addr);
	}
}
