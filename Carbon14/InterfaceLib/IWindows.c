
#include "C14Windows.h"


DEFINE_API( void )
BeginUpdate(WindowRef window)
{
    C14BeginUpdate(window);
}


DEFINE_API( void )
CloseWindow(WindowRef window)
{
    C14CloseWindow(window);
}


DEFINE_API( void )
DisposeWindow(WindowRef window)
{
    C14DisposeWindow(window);
}


DEFINE_API( void )
DragWindow(
  WindowRef     window,
  Point         startPt,
  const Rect *  boundsRect)
{
    C14DragWindow(window, startPt, boundsRect);
}


DEFINE_API( void )
EndUpdate(WindowRef window)
{
    C14EndUpdate(window);
}


DEFINE_API( WindowPartCode )
FindWindow(
  Point        thePoint,
  WindowRef *  window)
{
    return C14FindWindow(thePoint, window);
}


DEFINE_API( WindowRef )
FrontWindow(void)
{
    return C14FrontWindow();
}


DEFINE_API( WindowRef )
GetNewWindow(
  short       windowID,
  void *      wStorage,
  WindowRef   behind)
{
    return C14GetNewWindow(windowID, wStorage, behind);
}


DEFINE_API( void )
InvalRect(const Rect * badRect)
{
    C14InvalRect(badRect);
}
