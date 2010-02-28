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

#ifndef __Gateway_h__
#define __Gateway_h__


enum {
    sysBootBlock,
    sysDeskHook,
    sysMenuSelectReturn,
    
    nSysCalls
};


enum {
    vGateway = 0x00600000
};


void InitGateway(void);
UInt32 GetGatewayAddress(unsigned int sysCallNum);
void GatewayDispatcher(UInt16 trapWord, UInt32 regs[16]);


extern UInt16 *gateway;


#endif /* __Gateway_h__ */
