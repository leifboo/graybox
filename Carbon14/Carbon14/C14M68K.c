
#include "C14M68K.h"

#include "C14LowMem.h"

#include "SYSDEPNS.h"
#include "ENDIANAC.h"
#include "MINEM68K.h"


static ui3b *bankReadAddr[NumMemBanks];
static ui3b *bankWritAddr[NumMemBanks];
static blnr viaInterruptRequest;


ui5b MM_Access(ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);
ui5b MM_Access(ui5b data, blnr writeMem, blnr byteSize, CPTR addr) {
    ui3b *bytePtr;
    ui4b *wordPtr;
    
    if (0x100 <= addr && addr < 0xE00) {
        return C14LowMemAccess(data, writeMem, byteSize, addr);
    }
    
    if (addr == 0x2a000008) { /* ROM version */
        /* XXX: allocate ROM block */
        return 0x7D; /*various*/ /* 0x75 Mac Plus */
    }
    
    if (addr == 0xdead0000) /* TopMapHndl */
        return 0xbeef;
    if (addr == 0xdead0002) /* TopMapHndl */
        return 0x0000;
    
    /* real addressing */
    if (byteSize) {
        bytePtr = (ui3b *)addr;
        if (writeMem) {
            *bytePtr = data;
        } else {
            data = *bytePtr;
        }
    } else {
        wordPtr = (ui4b *)addr;
        if (writeMem) {
            *wordPtr = data;
        } else {
            data = *wordPtr;
        }
    }
    return data;
}


static void setUpBankRange(ui5b StartBank, ui5b StopBank, ui3b * RealStart, CPTR VirtualStart, ui5b vMask, blnr Writeable)
{
	ui5b i;

	for (i = StartBank; i < StopBank; i++) {
		bankReadAddr[i] = RealStart + (((i << ln2BytesPerMemBank) - VirtualStart) & vMask);
		if (Writeable) {
			bankWritAddr[i] = bankReadAddr[i];
		}
	}
}


/*
 * m68k_mmap
 *
 * Map real memory into the M68K emulator's address space.
 */
static void m68k_mmap(Ptr ptr, unsigned long size) {
    ui5b base, baseBank;
    ui5b top, topBank;
	ui5b mask;
    
    base = (ui5b)ptr & ~MemBankAddrMask;
    top = ((ui5b)(ptr + size) + MemBankAddrMask) & ~MemBankAddrMask;
    baseBank = base >> ln2BytesPerMemBank;
    topBank = top >> ln2BytesPerMemBank;
    mask = size - 1;
	setUpBankRange(baseBank, topBank, (ui3b *)base, base, mask, trueblnr);
}


static void junk(void) {
    THz sysZone, applZone;
    
    /* What could this possibly be used for under Carbon? */
    sysZone = LMGetSysZone();
    m68k_mmap((Ptr)sysZone, sysZone->bkLim - (Ptr)sysZone);

    applZone = LMGetApplZone();
    /* XXX: What if the heap grows? */
    m68k_mmap((Ptr)applZone, applZone->bkLim - (Ptr)applZone);

}


void
C14M68KStart(Ptr pc, Ptr sp, Ptr a5) {
    MINEM68K_Init(bankReadAddr, bankWritAddr, &viaInterruptRequest);
	m68k_reset((CPTR)pc, (ui5b)sp, (ui5b)a5);
	
	while (1) {
        m68k_go_nInstructions(0xFFFFFFFF);
    }
}


UInt32
C14M68KGetPC(void) {
    return m68k_getpc();
}


void
C14M68KSetPC(UInt32 newPC) {
    m68k_setpc(newPC);
}
