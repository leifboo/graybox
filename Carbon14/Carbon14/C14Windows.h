
#ifndef __C14Windows_h__
#define __C14Windows_h__


#include "C14Macros.h"
#include "C14Types.h"


C14_API( void )
C14BeginUpdate(WindowRef window);

C14_API( void )
C14CloseWindow(WindowRef window);

C14_API( void )
C14DisposeWindow(WindowPtr classicWindow);

C14_API( void )
C14DragWindow(
  WindowRef     window,
  Point         startPt,
  const Rect *  boundsRect);

C14_API( void )
C14EndUpdate(WindowRef window);

C14_API( WindowPartCode )
C14FindWindow(
  Point        thePoint,
  WindowRef *  window);

C14_API( WindowRef )
C14FrontWindow(void);

C14_API( WindowRef )
C14GetNewWindow(
  short       windowID,
  void *      wStorage,
  WindowRef   behind);

C14_API( void )
C14InvalRect(const Rect * badRect);


#endif /* __C14Windows_h__ */
