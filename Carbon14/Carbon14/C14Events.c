
#include "C14Events.h"

#include "C14Quickdraw.h"
#include "C14Windows.h"



DEFINE_API( Boolean )
C14WaitNextEvent(
  EventMask      eventMask,
  EventRecord *  theEvent,
  UInt32         sleep,
  C14RgnHandle   mouseRgn)
{
    Boolean result;
    C14PortPtr *c14Port;
    
    result = WaitNextEvent(eventMask, theEvent, sleep,
                           mouseRgn ? (**mouseRgn).carbonRgn : nil);
    if (result) {
        switch (theEvent->what) {
        case activateEvt:
        case updateEvt:
            c14Port = C14PrivateFindCarbonWindow((WindowRef)theEvent->message);
            theEvent->message = (UInt32)(**c14Port).classicPort;
            break;
        }
    }
    return result;
}
