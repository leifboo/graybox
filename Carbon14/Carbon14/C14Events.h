
#ifndef __C14Events_h__
#define __C14Events_h__


#include "C14Macros.h"
#include "C14Types.h"


C14_API( Boolean )
C14WaitNextEvent(
  EventMask      eventMask,
  EventRecord *  theEvent,
  UInt32         sleep,
  C14RgnHandle   mouseRgn);


#endif /* __C14Events_h__ */
