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

#ifndef __Patching_h__
#define __Patching_h__


typedef void (*TrapProcPtr)(UInt16 trapWord, UInt32 regs[16]);


extern TrapProcPtr osTrapTable[];
extern TrapProcPtr tbTrapTable[];


Boolean InitMisc(void);
void GBTrapDispatcher(UInt16 trapWord, UInt32 regs[16]);
void GBPerformTrap(void);
void GBPatchROM(void);


#endif /* __Patching_h__ */
