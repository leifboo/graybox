
#include "C14M68K.h"

#include "C14LowMem.h"

#include "SYSDEPNS.h"
#include "ENDIANAC.h"
#include "MINEM68K.h"


static ui3b *bankReadAddr[NumMemBanks];
static ui3b *bankWritAddr[NumMemBanks];
static blnr viaInterruptRequest;

static Boolean go;


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
 * C14M68KMapMemory
 *
 * Map real memory into the M68K emulator's address space.
 */
void C14M68KMapMemory(UInt32 virtualPtr, Ptr ptr, Size size) {
    ui5b base, baseBank;
    ui5b top, topBank;
    ui5b mask;
    
    base = virtualPtr & ~MemBankAddrMask;
    top = ((virtualPtr + size) + MemBankAddrMask) & ~MemBankAddrMask;
    baseBank = base >> ln2BytesPerMemBank;
    topBank = top >> ln2BytesPerMemBank;
    mask = size - 1;
    setUpBankRange(baseBank, topBank, (ui3b *)ptr, virtualPtr, mask, trueblnr);
}


void
C14M68KStart(Ptr pc, Ptr sp) {
    MINEM68K_Init(bankReadAddr, bankWritAddr, &viaInterruptRequest);
    m68k_reset((CPTR)pc, (ui5b)sp);
    
    go = true;
    while (go) {
        m68k_go_nInstructions(0xFFFFFFFF);
    }
}


void
C14M68KStop(void) {
    go = false;
    m68k_stop();
}


UInt32 *
C14M68KRegisters(void) {
    return m68k_regs();
}


UInt32
C14M68KGetPC(void) {
    return m68k_getpc();
}


void
C14M68KSetPC(UInt32 newPC) {
    m68k_setpc(newPC);
}
