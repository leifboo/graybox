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
#include "Logging.h"
#include "Patching.h"
#include "vMac.h"

#include <Traps.h>


static Boolean inBackground;
static int nullEventTally, nullEventAvailTally;
static int windowCount = 16; /* XXX: obviously not a constant */


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


static void handleOSEvent(EventRecord *theEvent) {
    if ((theEvent->message >> 24) & suspendResumeMessage) {
        inBackground = !(theEvent->message & 1);
        
        if (inBackground) {
            HideWindow(menusWindow);
        } else {
            ShowWindow(menusWindow);
            /* make sure menus are always on top */
            SelectWindow(menusWindow);
        }
    }
}


static Boolean
GetOSEvent(
  EventMask      eventMask,
  EventRecord *  theEvent)
{
    WindowPtr window;
    Point where;
    UInt32 sleep;
    
    FlushDisplay();
    
    eventMask |= highLevelEventMask | osMask;
    
    while (true) {
        sleep = nullEventTally < windowCount
                ? 0 /* can't sleep: there might be windows that need updating */
                : (inBackground
                   ? (UInt32)-1 /* in background, nothing to do: sleep forever */
                   : 15); /* good enough for flashing text insertion point */
        
        WaitNextEvent(eventMask, theEvent, sleep, NULL);
        
        where = theEvent->where;
        GlobalToLocal(&theEvent->where);
        updateMouse(&theEvent->where);
        
        if (theEvent->what == nullEvent) {
            /* allow update events to happen */
            ++nullEventTally;
            return false;
        } else {
            nullEventTally = 0;
        }

        switch (theEvent->what) {
        
        case mouseDown:
            switch (FindWindow(where, &window)) {
            case inContent:
            case inMenuBar:
                return true;
            }
            break;
        
        case mouseUp:
            return true;
            
        case keyDown:
        case keyUp:
        case autoKey:
            return true;
        
        case osEvt:
            handleOSEvent(theEvent);
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
    EventRecord *theEvent = (EventRecord *)get_real_address(regs[8+0]);
    
    (void)trapWord;
    
    FlushDisplay();
    
    if (EventAvail(eventMask, theEvent)) {
        regs[0] = 0;
        nullEventAvailTally = 0;
    } else {
        EventRecord osEvent;
        
        /* We must return a null event at least twice
           in order for MenuSelect() to draw the initial menu. */
        if (nullEventAvailTally < 2) {
            ++nullEventAvailTally;
        } else {
            /*
             * Reduce CPU usage during dragging.  It's a pity there's
             * no WaitNextEventAvail(); but the app might be
             * animating something, so we can only sleep for a few
             * ticks anyway.  15 fps is good enough for marching ants,
             * and a delay of 4 ticks is not perceptible when releasing
             * the mouse on a menu item (for example).
             */
            RgnHandle mouseRgn = NewRgn();
            if (WaitNextEvent(osMask, &osEvent, 4, mouseRgn)) {
                handleOSEvent(&osEvent);
            }
            DisposeRgn(mouseRgn);
        }
        
        regs[0] = 0xffffffff;
    }
    
    GlobalToLocal(&theEvent->where);
    updateMouse(&theEvent->where);
    
    m68k_test_d0();
}


static void trapGetOSEvent(UInt16 trapWord, UInt32 regs[16])
{
    EventMask eventMask = regs[0];
    EventRecord *theEvent = (EventRecord *)get_real_address(regs[8+0]);

    (void)trapWord;
    
    if (GetOSEvent(eventMask, theEvent)) {
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
