
#include "C14Events.h"



DEFINE_API( Boolean )
WaitNextEvent(
  EventMask      eventMask,
  EventRecord *  theEvent,
  UInt32         sleep,
  RgnHandle      mouseRgn)
{
    return C14WaitNextEvent(eventMask, theEvent, sleep, (C14RgnHandle)mouseRgn);
}
