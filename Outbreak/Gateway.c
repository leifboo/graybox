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

#include "Gateway.h"

#include "Display.h"
#include "GrayBox.h"
#include "vMac.h"

#include <Traps.h>


enum {
    gatewayInstCount = 4,
    gatewayEntrySize = gatewayInstCount*sizeof(UInt16)
};


UInt16 *gateway;


void InitGateway(void)
{
    unsigned int i, o;

    gateway = (UInt16 *)NewPtr(nSysCalls * gatewayEntrySize);
    
    for (i = 0; i < nSysCalls; ++i) {
        o = i * gatewayInstCount;
        gateway[o + 0] = 0x4E71;     /* NOP */
        gateway[o + 1] = 0x7000 | i; /* MOVEQ #i,D0 */
        gateway[o + 2] = 0x4E40;     /* TRAP #0 */
        gateway[o + 3] = 0x4E75;     /* RTS */
    }
    
    gateway[sysMenuSelect * gatewayInstCount] = _MenuSelect;
    gateway[sysMenuSelect * gatewayInstCount + 1] = 0x7000 | sysReturn;
}


UInt32 GetGatewayAddress(unsigned int sysCallNum)
{
    return vGateway + sysCallNum*gatewayEntrySize;
}


static void bootBlock(UInt16 trapWord, UInt32 regs[16])
{
    ParmBlkPtr paramBlock;
    FSSpec spec;
    short refNum;
    long count;
    OSErr result;

    paramBlock = (ParmBlkPtr)get_real_address(regs[8+0]);
    
    count = paramBlock->ioParam.ioReqCount;
    result = FSMakeFSSpec(bootVRefNum, rsrcDirID, "\pMBR", &spec);
    if (result != noErr) goto leave;
    result = FSpOpenDF(&spec, fsRdPerm, &refNum);
    if (result != noErr) goto leave;
    result = FSRead(refNum, &count, get_real_address((UInt32)paramBlock->ioParam.ioBuffer));
    if (result != noErr) goto leave;
    FSClose(refNum);
    
    /*XXX: patch MBR */
    
    paramBlock->ioParam.ioActCount = count;
    result = noErr;
    
leave:
    paramBlock->ioParam.ioResult = result;
    regs[0] = result;
    
    (void)trapWord;
}


void GatewayDispatcher(UInt16 trapWord, UInt32 regs[16])
{
    (void)trapWord;

    switch (regs[0]) {
    
    case sysBootBlock:
        bootBlock(trapWord, regs);
        break;
    
    case sysDeskHook: {
        Handle classicClipRgn = (Handle)get_real_address(regs[3]);
        Ptr vPtr = *classicClipRgn;
        *classicClipRgn = (Ptr)get_real_address((CPTR)vPtr);
        EraseDesktop(classicClipRgn);
        *classicClipRgn = vPtr;
        break; }
    
    case sysReturn:
        m68k_stop();
        break;
    }
}
