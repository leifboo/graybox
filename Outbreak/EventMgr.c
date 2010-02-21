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

#include "EventMgr.h"

#include "Display.h"
#include "GrayBox.h"
#include "Patching.h"
#include "vMac.h"

#include <Traps.h>


static Boolean inBackground;


static void updateMouse(Point *mouseLoc)
{
    UInt32 newMouse = (mouseLoc->v << 16) | mouseLoc->h;

    if (get_ram_long(0x0828) != newMouse) {
        put_ram_long(0x0828, newMouse);
        put_ram_long(0x082C, newMouse);
        put_ram_byte(0x08CE, get_ram_byte(0x08CF));
        put_ram_long(0x0830, newMouse);
    }
}


static Boolean
GetOSEvent(
  EventMask      mask,
  EventRecord *  theEvent)
{
    WindowPtr window;
    
    FlushDisplay();
    
    mask |= highLevelEventMask | osMask;
    
    while (true) {
        WaitNextEvent(mask, theEvent, 0, NULL);
        
        updateMouse(&theEvent->where);

        switch (theEvent->what) {
        
        case nullEvent:
            /* allow update events to happen */
            return false;
        
        case mouseDown:
            switch (FindWindow(theEvent->where, &window)) {
            case inContent:
                if (window != FrontWindow()) {
                    SelectWindow(window);
                } else if (window == gMyMainWindow) {
                    SetPortWindowPort(gMyMainWindow);
                    GlobalToLocal(&theEvent->where);
                    return true;
                }
            }
            break;
        
        case keyDown:
        case keyUp:
        case autoKey:
            return true;
        
        case mouseUp:
            SetPortWindowPort(gMyMainWindow);
            GlobalToLocal(&theEvent->where);
            return true;
        
        case osEvt:
            if ((theEvent->message >> 24) & suspendResumeMessage) {
                inBackground = !(theEvent->message & 1);
            }
            break;
        
        case kHighLevelEvent:
            if ((AEEventClass)theEvent->message == kCoreEventClass) {
                AEProcessAppleEvent(theEvent);
            }
            break;
        }
    }
    
    /* not reached */
    return false;
}


static void trapOSEventAvail(UInt16 trapWord, UInt32 regs[16])
{
    EventMask eventMask = regs[0];
    EventRecord *eventRecord = (EventRecord *)get_real_address(regs[8+0]);
    
    (void)trapWord;
    
    FlushDisplay();
    
    if (EventAvail(eventMask, eventRecord)) {
        regs[0] = 0;
    } else {
        regs[0] = 0xffffffff;
    }
    
    updateMouse(&eventRecord->where);
    
    m68k_test_d0();
}


static void trapGetOSEvent(UInt16 trapWord, UInt32 regs[16])
{
    EventMask eventMask = regs[0];
    EventRecord *eventRecord = (EventRecord *)get_real_address(regs[8+0]);

    (void)trapWord;
    
    if (GetOSEvent(eventMask, eventRecord)) {
        regs[0] = 0;
    } else {
        regs[0] = 0xffffffff;
    }
    
    m68k_test_d0();
}


static void trapButton(UInt16 trapWord, UInt32 regs[16])
{
    Boolean *resultPtr = (Boolean *)get_real_address(regs[8+7]);
    *resultPtr = Button();
    (void)trapWord;
}


Boolean InitEvents(void)
{
    osTrapTable[_GetOSEvent     & 0xFF]     = &trapGetOSEvent;
    osTrapTable[_OSEventAvail   & 0xFF]     = &trapOSEventAvail;
    
    tbTrapTable[_Button         & 0x3FF]    = &trapButton;
    
    return true;
}
