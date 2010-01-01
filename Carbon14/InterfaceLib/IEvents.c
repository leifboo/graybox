
#include "C14Events.h"


DEFINE_API( Boolean )
OSEventAvail(
  EventMask      mask,
  EventRecord *  theEvent)
{
    return C14OSEventAvail(mask, theEvent);
}


DEFINE_API( Boolean )
WaitNextEvent(
  EventMask      eventMask,
  EventRecord *  theEvent,
  UInt32         sleep,
  RgnHandle      mouseRgn)
{
    return C14WaitNextEvent(eventMask, theEvent, sleep, (C14RgnHandle)mouseRgn);
}
