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
#include "MYOSGLUE.h"
#include "ENDIANAC.h"
#include "MINEM68K.h"
#endif

#include "ADDRSPAC.h"

IMPORTFUNC ui5b SCSI_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);
IMPORTFUNC ui5b SCC_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);
IMPORTFUNC ui5b IWM_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);
IMPORTFUNC ui5b VIA_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);
IMPORTFUNC ui5b Sony_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);

/* top 8 bits out of 32 are ignored, so total size of address space is 2 ** 24 bytes */

#define TotAddrBytes (1UL << ln2TotAddrBytes)
#define kAddrMask (TotAddrBytes - 1)

/* map of address space */

LOCALVAR ui3b vOverlay;

#define kROM_Overlay_Base 0x00000000 /* when overlay on */
#define kROM_Overlay_Top  0x00100000

#define kRAM_Base 0x00000000 /* when overlay off */
#define kRAM_Top  0x00400000

#define kROM_Base 0x00400000
#define kROM_Top  0x00500000

#define kSCSI_Block_Base 0x00580000
#define kSCSI_Block_Top  0x00600000

#define kRAM_Overlay_Base 0x00600000 /* when overlay on */
#define kRAM_Overlay_Top  0x00700000

#define kSCCRd_Block_Base 0x00800000
#define kSCCRd_Block_Top  0x00A00000

#define kSCCWr_Block_Base 0x00A00000
#define kSCCWr_Block_Top  0x00C00000

#define kIWM_Block_Base 0x00C00000
#define kIWM_Block_Top  0x00E00000

#define kVIA_Block_Base 0x00E80000
#define kVIA_Block_Top  0x00F00000

#define kDSK_Block_Base 0x00F40000
#define kDSK_Block_Top  0x00F40020

#define kAutoVector_Base 0x00FFFFF0
#define kAutoVector_Top  0x01000000

/* implementation of read/write for everything but RAM and ROM */

#define kSCCRdBase 0x9FFFF8
#define kSCCWrBase 0xBFFFF9

#define kSCC_Mask 0x07

#define kVIA_Mask 0x001FFF
#define kVIA_Base 0xEFE1FE

#define kIWM_Mask 0x001FFF // Allocated Memory Bandwidth for IWM
#define kIWM_Base 0xDFE1FF // IWM Memory Base

GLOBALFUNC ui5b MM_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr)
{
	CPTR mAddressBus = addr & kAddrMask;

	if (mAddressBus < kIWM_Block_Base) {
		if (mAddressBus < kSCCRd_Block_Base) {
#if CurEmu >= kEmuPlus1M
			if ((mAddressBus >= kSCSI_Block_Base) && (mAddressBus < kSCSI_Block_Top)) {
				Data = SCSI_Access(Data, WriteMem, ByteSize, mAddressBus - kSCSI_Block_Base);
			}
#endif
		} else {
			if (mAddressBus >= kSCCWr_Block_Base) {
				if (WriteMem) {
					/* if ((mAddressBus >= kSCCWr_Block_Base) && (mAddressBus < kSCCWr_Block_Top)) */  // Write Only Address
					{
						Data = SCC_Access(Data, WriteMem, ByteSize, (mAddressBus - kSCCWrBase) & kSCC_Mask);
					}
				}
			} else {
				if (! WriteMem) {
					/* if ((mAddressBus >= kSCCRd_Block_Base) && (mAddressBus < kSCCRd_Block_Top)) */ // Read Only Address
					{
						Data = SCC_Access(Data, WriteMem, ByteSize, (mAddressBus - kSCCRdBase) & kSCC_Mask);
					}
				}
			}
		}
	} else {
		if (mAddressBus < kVIA_Block_Base) {
			if (/* (mAddressBus >= kIWM_Block_Base) && */(mAddressBus < kIWM_Block_Top)) {
				Data = IWM_Access(Data, WriteMem, ByteSize, (mAddressBus - kIWM_Base) & kIWM_Mask);
			}
		} else {
			if (/* (mAddressBus >= kVIA_Block_Base) && */(mAddressBus < kVIA_Block_Top)) {
				Data = VIA_Access(Data, WriteMem, ByteSize, (mAddressBus - kVIA_Base) & kVIA_Mask);
			} else {
				if ((mAddressBus >= kDSK_Block_Base) && (mAddressBus < kDSK_Block_Top)) {
					Data = Sony_Access(Data, WriteMem, ByteSize, mAddressBus - kDSK_Block_Base);
				} else
				if ((mAddressBus >= kAutoVector_Base) && (mAddressBus < kAutoVector_Top)) {
					/* SetAutoVector(); */
					/* Exception(regs.intmask+24, 0); */
				}
			}
		}
	}
	return Data;
}

/* devide address space into banks, some of which are mapped to real memory */

GLOBALVAR ui3b *BankReadAddr[NumMemBanks];
GLOBALVAR ui3b *BankWritAddr[NumMemBanks];

#define kROM_BaseBank (kROM_Base >> ln2BytesPerMemBank)
#define kROM_TopBank (kROM_Top >> ln2BytesPerMemBank)

#define kROM_Overlay_BaseBank (kROM_Overlay_Base >> ln2BytesPerMemBank)
#define kROM_Overlay_TopBank (kROM_Overlay_Top >> ln2BytesPerMemBank)

#define kRAM_Overlay_BaseBank (kRAM_Overlay_Base >> ln2BytesPerMemBank)
#define kRAM_Overlay_TopBank (kRAM_Overlay_Top >> ln2BytesPerMemBank)

#define kRAM_BaseBank (kRAM_Base >> ln2BytesPerMemBank)
#define kRAM_TopBank (kRAM_Top >> ln2BytesPerMemBank)

#define Overlay_RAMmem_mask (0x00100000 - 1)
#define Overlay_ROMmem_mask ROMmem_mask
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

LOCALPROC SetUpMemBanks(void)
{
	ui5b RAMmem_mask = kRAM_Size - 1;

	SetPtrVecToNULL(BankReadAddr, NumMemBanks);
	SetPtrVecToNULL(BankWritAddr, NumMemBanks);

	SetUpBankRange(kROM_BaseBank, kROM_TopBank, (ui3b *) ROM, kROM_Base, ROMmem_mask, falseblnr);

	if (vOverlay) {
		SetUpBankRange(kROM_Overlay_BaseBank, kROM_Overlay_TopBank, (ui3b *)ROM, kROM_Overlay_Base, Overlay_ROMmem_mask, falseblnr);
#if kRAM_Size >= 0x00100000
		SetUpBankRange(kRAM_Overlay_BaseBank, kRAM_Overlay_TopBank, (ui3b *)RAM, kRAM_Overlay_Base, Overlay_RAMmem_mask, trueblnr);
#else
		SetUpBankRange(kRAM_Overlay_BaseBank, kRAM_Overlay_TopBank, (ui3b *)RAM, kRAM_Overlay_Base, RAMmem_mask, trueblnr);
#endif
	} else {
#if kRAM_Size == 0x00280000
		SetUpBankRange(kRAM_BaseBank, (0x00200000 >> ln2BytesPerMemBank), (ui3b *)RAM, kRAM_Base, 0x00200000 - 1, trueblnr);
		SetUpBankRange((0x00200000 >> ln2BytesPerMemBank), kRAM_TopBank, 0x00200000 + (ui3b *)RAM, kRAM_Base, 0x00080000 - 1, trueblnr);
#else
		SetUpBankRange(kRAM_BaseBank, kRAM_TopBank, (ui3b *)RAM, kRAM_Base, RAMmem_mask, trueblnr);
#endif
	}
}

GLOBALPROC ZapNMemoryVars(void)
{
	vOverlay = 2; /* MemBanks uninitialized */
}

GLOBALPROC VIA_PORA4(ui3b Data)
{
	if (vOverlay != Data) {
		vOverlay = Data;
		SetUpMemBanks();
	}
}

GLOBALFUNC ui3b VIA_GORA4(void) // Overlay/Normal Memory Mapping
{
#ifdef _VIA_Interface_Debug
	printf("VIA ORA4 attempts to be an input\n");
#endif
	return 0;
}

GLOBALPROC Memory_Reset(void)
{
	VIA_PORA4(1);
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
