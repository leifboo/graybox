/*
    GrayBox
    Copyright (C) 2010  Leif Strand

    This program is free software: you can redistribute this file
    and/or modify it under the terms of version 2 of the GNU General
    Public License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Patching.h"

#include "Gateway.h"
#include "GrayBox.h"
#include "Logging.h"
#include "vMac.h"

#include <Traps.h>


TrapProcPtr osTrapTable[0x100];
TrapProcPtr tbTrapTable[0x400];



/*
 * misc. patches
 */

static void trapRDrvrInstall(UInt16 trapWord, UInt32 regs[16]) {
    regs[0] = noErr;
	m68k_test_d0();
    (void)trapWord;
}

static void trapTickCount(UInt16 trapWord, UInt32 regs[16]) {
    UInt32 *resultPtr = (UInt32 *)get_real_address(regs[8+7]);
    *resultPtr = TickCount();
    (void)trapWord;
}

static void trapSysBeep(UInt16 trapWord, UInt32 regs[16]) {
    short *argPtr = (short *)get_real_address(regs[8+7]);
    regs[8+7] += 2;
    SysBeep(*argPtr);
    (void)trapWord;
}

static void trapExitToShell(UInt16 trapWord, UInt32 regs[16]) {
    closeLog();
    ExitToShell();
    (void)trapWord; (void)regs;
}

static void trapAppendResMenu(UInt16 trapWord, UInt32 regs[16]) {
    ResType *theTypePtr = (ResType *)get_real_address(regs[8+7]);
    if (*theTypePtr == 'DRVR') {
        /* desk accessories don't work */
        regs[8+7] += 8;
    } else {
        /* font menu */
        m68k_backup_pc();
        m68k_exception(0xA);
    }
    (void)trapWord;
}


Boolean InitMisc(void)
{
    osTrapTable[_RDrvrInstall   & 0xFF]     = &trapRDrvrInstall;

    tbTrapTable[_TickCount      & 0x3FF]    = &trapTickCount;
    tbTrapTable[_SysBeep        & 0x3FF]    = &trapSysBeep;
    tbTrapTable[_ShutDown       & 0x3FF]    = &trapExitToShell;
    tbTrapTable[_ExitToShell    & 0x3FF]    = &trapExitToShell;
    tbTrapTable[_AppendResMenu  & 0x3FF]    = &trapAppendResMenu;
}


/*
 * patching traps
 */

void GBTrapDispatcher(UInt16 trapWord, UInt32 regs[16])
{
    TrapProcPtr routine;
    
    /*logTrap(trapWord, regs);*/
    
    if (trapWord & 0x800) {
        routine = tbTrapTable[trapWord & 0x3FF];
    } else {
        routine = osTrapTable[trapWord & 0xFF];
    }
    
    if (routine) {
        (*routine)(trapWord, regs);
    } else {
        m68k_backup_pc();
        m68k_exception(0xA);
    }
}


void GBPerformTrap(void) {
    /*
     * Perform the current trap, regaining control afterwards.
     */
    UInt16 *pc;
    UInt16 save[2];
    
    increaseIndent();
    
    /* temporarily patch the code calling the trap */
    pc = (UInt16 *)get_real_address(m68k_getpc());
    save[0] = pc[0];
    save[1] = pc[1];
    pc[0] = 0x7000 | sysReturn; /* MOVEQ #sysReturn,D0 */
    pc[1] = 0x4E40;             /* TRAP #0 */
    
    /* trap */
    m68k_backup_pc();
    m68k_exception(0xA);
    m68k_go();
    
    /* restore */
    m68k_backup_pc(); /* TRAP */
    m68k_backup_pc(); /* MOVEQ */
    pc[0] = save[0];
    pc[1] = save[1];

    decreaseIndent();
}



/*
 * ROM patches
 */

void GBPatchROM(void)
{
    /* These patches are for Mac Plus ROM 0x75 0x4D1F8172 "Loud Harmonicas". */
    UInt16 *rom = (UInt16 *)ROM;
    unsigned int offset;
    
    
    /*
     * vMac patches
     */
    rom[3450 >> 1] = 0x6022; /* skip the rom checksum */
    rom[3752 >> 1] = 0x4E71; /* shorten the ram check read */
    rom[3728 >> 1] = 0x4E71; /* shorten the ram check write*/
    #if 0
    /* This might work if low memory globals still got zapped. */
    rom[862 >> 1] = 0x4E71; /* shorten set memory*/
    #endif
    
    
    /*
     * GrayBox patches
     */
    
    /*    624: 6100 0256      100087C          BSR     proc11 */
    rom[(0x624+0) >> 1] = 0x4E71;
    rom[(0x624+2) >> 1] = 0x4E71;
    
    /*    6DA: 6100 00F2      10007CE          BSR     proc7 */
    rom[(0x6DA+0) >> 1] = 0x4E71;
    rom[(0x6DA+2) >> 1] = 0x4E71;
    
    /* 624 patch causes hang:
        6E4: 2C38 030A          30A          MOVE.L  DrvQHdr+QHead,D6
        6E8: 67FE           10006E8 lag_40   BEQ     lag_40
    */
    rom[(0x6E4+0) >> 1] = 0x4E71; /*NOP*/
    rom[(0x6E4+2) >> 1] = 0x4E71; /*NOP*/
    rom[(0x6E4+4) >> 1] = 0x4E71; /*NOP*/

    /*
       6EA: 0838 0004 020B     20B          BTST    #4,SpMisc2
       6F0: 6600 008A      100077C          BNE     lag_46
    */
    rom[(0x6EA+0) >> 1] = 0x4E71; /*NOP*/
    rom[(0x6EA+2) >> 1] = 0x4E71; /*NOP*/
    rom[(0x6EA+4) >> 1] = 0x4E71; /*NOP*/
    rom[(0x6EA+6) >> 1] = 0x4E71; /*NOP*/
    rom[(0x6EA+8) >> 1] = 0x4E71; /*NOP*/

    /*
       70C: 4A86           'J.'             TST.L   D6
       70E: 6700 006C      100077C          BEQ     lag_46
    */
    rom[(0x70C+0) >> 1] = 0x4E71; /*NOP*/
    rom[(0x70C+2) >> 1] = 0x4E71; /*NOP*/
    rom[(0x70C+4) >> 1] = 0x4E71; /*NOP*/
    
    /*
       72E: 4A38 0172          172          TST.B   MbState
       732: 6A2E           1000762          BPL.S   lag_44
       734: A002           '..'             _Read   ; (A0|IOPB:ParamBlockRec):D0\OSErr 
    */
    rom[(0x72E +0) >> 1] = 0x4E71; /*NOP*/
    rom[(0x72E +2) >> 1] = 0x4E71; /*NOP*/
    rom[(0x72E +4) >> 1] = 0x7000 | sysBootBlock; /* MOVEQ #i,D0 */
    rom[(0x72E +6) >> 1] = 0x4E40;                /* TRAP #0 */
    
    #if 0 /* harmless */
    /*    9C4: A06D           '.m'             _InitEvents  */
    rom[(0x9C4+0) >> 1] = 0x4E71; /*NOP*/
    #endif

    #if 0 /* We use this to do misc. file-related things. */
    /*    9CA: A06C           '.l'             _InitFs  */
    rom[(0x9CA+0) >> 1] = 0x4E71; /*NOP*/
    #endif

    #if 0 /* We now use this. */
    /*
       9D4: A00F           '..'             _MountVol ; (A0|IOPB:ParamBlockRec):D0\OSErr 
       9D6: 6600 01AA      1000B82          BNE     lag_79
    */
    rom[(0x9D4+0) >> 1] = 0x4E71; /*NOP*/
    rom[(0x9D4+2) >> 1] = 0x4E71; /*NOP*/
    rom[(0x9D4+4) >> 1] = 0x4E71; /*NOP*/
    #endif

    /* Evil!!!
      FF10: 2038 0358          358          MOVE.L  VCBQHdr+QHead,D0
      FF14: 6710           100FF26 lbl_70   BEQ.S   lbl_72
      FF16: 2240           '"@'             MOVEA.L D0,A1
      FF18: 3169 0048 0016 '1i.H..'         MOVE    72(A1),ioVRefNum(A0)
      FF1E: 6702           100FF22          BEQ.S   lbl_71
      FF20: A013           '..'             _FlushVol ; (A0|IOPB:ParamBlockRec):D0\OSErr 
      FF22: 2011           ' .'    lbl_71   MOVE.L  (A1),D0
      FF24: 60EE           100FF14          BRA     lbl_70
      FF26: DEFC 0040      '...@'  lbl_72   ADDA.W  #64,A7
    */
    for (offset = 0xFF10; offset < 0xFF26; ++offset) {
        rom[offset >> 1] = 0x4E71; /*NOP*/
    }
        
    /* mysterious Control call from Launch */
    for (offset = 0xF560; offset < 0xF59C; ++offset) {
        rom[offset >> 1] = 0x4E71; /*NOP*/
    }
    
    /* "Welcome to Macintosh"
    /*    A60: 6100 0150      1000BB2          BSR     proc13 */
    rom[0xA60 >> 1] = 0x4E71; /*NOP*/
    
    /* Startup chime
    
        EE: 7628           'v('    data5    MOVEQ   #40,D3
        F0: 4DFA 0006      10000F8          LEA     data6,A6
        F4: 6000 0194      100028A          BRA     lae_19
    */
    rom[(0xEE +0) >> 1] = 0x4E71; /*NOP*/
    rom[(0xF0+0) >> 1] = 0x4E71; /*NOP*/
    rom[(0xF0+2) >> 1] = 0x4E71; /*NOP*/
    rom[(0xF4+0) >> 1] = 0x4E71; /*NOP*/
    rom[(0xF4+2) >> 1] = 0x4E71; /*NOP*/
    
    /* IWM init hang
        F8: 08D5 0007      '....'  data6    BSET    #7,(A5)
        FC: 207C 00DF E1FF ' |....'         MOVEA.L #$DFE1FF,A0
       102: 701F           'p.'             MOVEQ   #31,D0
       104: 4A28 1000      'J(..'  lae_4    TST.B   $1000(A0)
       108: 4A28 1A00      'J(..'           TST.B   $1A00(A0)
       10C: 1428 1C00      '.(..'           MOVE.B  $1C00(A0),D2
       110: 0802 0005      '....'           BTST    #5,D2
       114: 66EE           1000104          BNE     lae_4
       116: C400           '..'             AND.B   D0,D2
       118: B400           '..'             CMP.B   D0,D2
       11A: 670A           1000126          BEQ.S   lae_5
       11C: 1140 1E00      '.@..'           MOVE.B  D0,$1E00(A0)
       120: 4A28 1C00      'J(..'           TST.B   $1C00(A0)
       124: 60DE           1000104          BRA     lae_4
       126: 4A28 1800      'J(..'  lae_5    TST.B   $1800(A0)
       12A: 4DFA 0006      1000132          LEA     data7,A6
       12E: 6000 0C46      1000D76          BRA     lae_21
    */
    for (offset = 0xFC; offset < 0x12A; ++offset) {
        rom[offset >> 1] = 0x4E71; /*NOP*/
    }
    
    /* Cursor stuff */
    rom[0x1C9A >> 1] = 0x4E75; /* RTS */
    rom[0x1CA8 >> 1] = 0x4E75; /* RTS */
    
    /* desktop gray region
     11386: 42A7           'B.'    proc490  CLR.L   -(A7)
     11388: A90B           '..'             _ClipAbove ; (window:WindowPeek) 
     1138A: 2038 0A6C          A6C          MOVE.L  DeskHook,D0
     1138E: 6706           1011396          BEQ.S   lbl_392
     11390: 2040           ' @'             MOVEA.L D0,A0
     11392: 7000           'p.'             MOVEQ   #0,D0
     11394: 4ED0           'N.'             JMP     (A0)
     11396: 486B 0010      'Hk..'  lbl_392  PEA     portRect(A3)
     1139A: 4878 0A3C      'Hx.<'           PEA     $A3C
     1139E: A8A5           '..'             _FillRect ; (r:Rect; pat:Pattern) 
     113A0: 4E75           'Nu'             RTS     
    */
    for (offset = 0x11386; offset < 0x113A0; ++offset) {
        rom[offset >> 1] = 0x4E71; /*NOP*/
    }
    rom[(0x11386 +  0) >> 1] = 0x2F38;               /* PUSH.L  GrayRgn */
    rom[(0x11386 +  2) >> 1] = 0x09EE;
    rom[(0x11386 +  4) >> 1] = 0xA879;               /* _SetClip ; (rgn:RgnHandle) */
    rom[(0x11386 +  6) >> 1] = 0x42A7;               /* CLR.L   -(A7) */
    rom[(0x11386 +  8) >> 1] = 0xA90B;               /* _ClipAbove ; (window:WindowPeek) */
    rom[(0x11386 + 10) >> 1] = 0x262B;               /* MOVE.L  clipRgn(A3),D3 */
    rom[(0x11386 + 12) >> 1] = 0x001C;
    rom[(0x11386 + 14) >> 1] = 0x7000 | sysDeskHook; /* MOVEQ #i,D0 */
    rom[(0x11386 + 16) >> 1] = 0x4E40;               /* TRAP #0 */
    
    #if 0 /* for debugging */
    rom[0x352 >> 1] = 0x4E40; /*TRAP #0 */
    #endif
}
