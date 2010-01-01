
#include "C14LowMem.h"


/*
 * Names, values, & quotations from MacNosy / The Debugger "Sys Syms" file.
 * See also IM III, Appendix D.
 */


enum {
    LoadTrap        = 0x12D,
    CPUFlag         = 0x12F,
    ApplLimit       = 0x130,
    ApplZone        = 0x2AA,
    CurrentA5       = 0x904,
    CurStackBase    = 0x908,
    FPFlgMode       = 0xA4A,
    ResLoad         = 0xA5E,
    SegHiEnable     = 0xBB2
};


static struct {
    Ptr CurrentA5; /* "current value of A5" */
    Ptr CurStackBase; /* "current stack base" */
} lowMem;



static UInt32 accessLong(UInt32 selector, UInt32 *longPtr,
                         UInt32 data, Boolean write, Boolean byte, UInt32 addr)
{
    UInt8 *bytePtr;
    UInt16 *wordPtr;
    
    if (byte)
        goto crash; /* XXX */
    
    switch (addr - selector) {
    case 0:
        if (write) {
            data = (data << 16) | (*longPtr & 0xffff);
        } else {
            data = *longPtr >> 16;
        }
        break;
    case 2:
        if (write) {
            data = (*longPtr & 0xffff0000) | data;
        } else {
            data = *longPtr & 0xffff;
        }
        break;
    default:
 crash:
        /* crash */
        if (byte) {
            bytePtr = (UInt8 *)addr;
            if (write) {
                *bytePtr = data;
            } else {
                data = *bytePtr;
            }
        } else {
            wordPtr = (UInt16 *)addr;
            if (write) {
                *wordPtr = data;
            } else {
                data = *wordPtr;
            }
        }
        break;
    }
    
    return data;
}


void
C14InitLowMem(Ptr a5, Ptr stackBase)
{
    lowMem.CurrentA5 = a5;
    lowMem.CurStackBase = stackBase;
}


UInt32
C14LowMemAccess(UInt32 data, Boolean write, Boolean byte, UInt32 addr)
{
    UInt8 *bytePtr;
    UInt16 *wordPtr;
    UInt32 tmpLong;
    
    switch (addr) {
    
    case LoadTrap: /* "trap before launch" */
        /* ignore */
        data = 0;
        break;
    
    case CPUFlag:
        /* XXX: ignore (KidBricks) */
        data = 0; /* 68000 ??? */
        break;
    
    case ApplLimit: /* "application limit" */
        /* ignore writes */
        data = 0x0040;
        break;
    case ApplLimit + 2:
        data = 0x0000;
        break;
    
    case ApplZone: /* "application heap zone" */
    case ApplZone + 2:
        tmpLong = (UInt32)LMGetApplZone();
        data = accessLong(ApplZone, &tmpLong,
                          data, write, byte, addr);
        if (write) {
            LMSetApplZone((THz)tmpLong);
        }
        break;
    
    case CurrentA5: /* "current value of A5" */
    case CurrentA5 + 2:
        data = accessLong(CurrentA5, (UInt32 *)&lowMem.CurrentA5,
                          data, write, byte, addr);
        break;
    
    case CurStackBase: /* "current stack base" */
    case CurStackBase + 2:
        data = accessLong(CurStackBase, (UInt32 *)&lowMem.CurStackBase,
                          data, write, byte, addr);
        break;
    
    case FPFlgMode: /* "floating point flags and mode" */
        /* XXX: ignore (KidBricks) */
        data = 0;
        break;
    
    case ResLoad: /* "Auto-load feature" */
        if (write) {
            LMSetResLoad(data);
        } else {
            data = LMGetResLoad();
        }
        break;
    
    case SegHiEnable: /* "0 to disable MoveHHi in LoadSeg" */
        /* ignore */
        data = 0;
        break;
        
    default:
        /* crash */
        if (byte) {
            bytePtr = (UInt8 *)addr;
            if (write) {
                *bytePtr = data;
            } else {
                data = *bytePtr;
            }
        } else {
            wordPtr = (UInt16 *)addr;
            if (write) {
                *wordPtr = data;
            } else {
                data = *wordPtr;
            }
        }
        break;
    }
    
    return data;
}
