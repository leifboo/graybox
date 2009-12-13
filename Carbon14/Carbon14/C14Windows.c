
#include "C14Windows.h"

#include "C14Quickdraw.h"


static AuxWinHandle awHead; /*XXX*/


DEFINE_API( void )
C14BeginUpdate(WindowPtr classicWindow)
{
    C14PortPtr *c14Port;
    
    c14Port = C14PrivateFindClassicPort(classicWindow);
    BeginUpdate((**c14Port).carbonWindow);
}


DEFINE_API( void )
C14CloseWindow(WindowPtr window)
{
    C14PortPtr *c14Port;
    WindowPeek classicWindow;
    
    c14Port = C14PrivateFindClassicPort(window);
    DisposeWindow((*c14Port)->carbonWindow);
    (*c14Port)->carbonPort = nil; /* destroyed by DisposeWindow() */
    (*c14Port)->carbonWindow = nil;
    
    if ((*c14Port)->isColor) {
        C14PrivateCloseCPort((CGrafPtr)window);
    } else {
        C14PrivateClosePort(window);
    }

    classicWindow = (WindowPeek)window;    
    DisposeHandle((Handle)classicWindow->titleHandle);
    C14DisposeRgn((C14RgnHandle)classicWindow->updateRgn);
    C14DisposeRgn((C14RgnHandle)classicWindow->contRgn);
    C14DisposeRgn((C14RgnHandle)classicWindow->strucRgn);
    
    *c14Port = (*c14Port)->next;
    DisposePtr((Ptr)*c14Port);
}


DEFINE_API( void )
C14DisposeWindow(WindowPtr classicWindow)
{
    C14CloseWindow(classicWindow);
    DisposePtr((Ptr)classicWindow);
}


DEFINE_API( void )
C14DragWindow(
  WindowPtr     classicWindow,
  Point         startPt,
  const Rect *  boundsRect)
{
    C14PortPtr *c14Port;
    
    c14Port = C14PrivateFindClassicPort(classicWindow);
    DragWindow((**c14Port).carbonWindow, startPt, boundsRect);
    C14PrivateSyncWindowPosition(*c14Port);
}


DEFINE_API( void )
C14EndUpdate(WindowPtr classicWindow)
{
    C14PortPtr *c14Port;
    
    c14Port = C14PrivateFindClassicPort(classicWindow);
    EndUpdate((**c14Port).carbonWindow);
}


DEFINE_API( WindowPartCode )
C14FindWindow(
  Point        thePoint,
  WindowPtr *  classicWindow)
{
    C14PortPtr *c14Port;
    WindowRef carbonWindow;
    WindowPartCode result;
    
    carbonWindow = nil;
    result = FindWindow(thePoint, &carbonWindow);
    if (carbonWindow) {
        c14Port = C14PrivateFindCarbonWindow(carbonWindow);
        *classicWindow = (**c14Port).classicPort;
    } else {
        *classicWindow = nil;
    }
    return result;
}


DEFINE_API( WindowPtr )
C14FrontWindow(void)
{
    C14PortPtr *c14Port;
    WindowRef carbonWindow;
    
    carbonWindow = FrontWindow();
    if (!carbonWindow)
        return nil;
    c14Port = C14PrivateFindCarbonWindow(carbonWindow);
    if (!c14Port)
        return nil;
    return (*c14Port)->classicPort;
}


DEFINE_API( WindowPtr )
C14GetNewWindow(
  short       windowID,
  void *      wStorage,
  WindowRef   behind)
{
    WindowRef carbonWindow; CGrafPtr carbonPort;
    WindowPeek classicWindow;
    C14PortPtr c14Port;
    void *classicStorage;
    C14RgnHandle strucRgn, contRgn, updateRgn;
    Str255 **titleHandle; short titleWidth; RgnHandle titleRgn; Rect titleBounds;
    WindowAttributes attributes;
    OSStatus status;

    carbonWindow = nil;
    classicWindow = nil;
    classicStorage = nil;
    strucRgn = contRgn = updateRgn = nil;
    titleHandle = nil;
    
    /* allocate the classic window, if necessary */
    if (!wStorage) {
        wStorage = classicStorage = NewPtr(sizeof(WindowRecord));
        if (!wStorage) {
            goto fail;
        }
    }
    classicWindow = (WindowPeek)wStorage;
    
    if (behind != kFirstWindowOfClass &&
        behind != kLastWindowOfClass) {
        goto fail; /*XXX*/
    }
    
    /* create the Carbon window */
    carbonWindow = GetNewWindow(windowID, nil, behind);
    if (!carbonWindow) {
        goto fail;
    }
    
    /* allocate classic window structures */
    strucRgn = C14NewRgn();
    if (!strucRgn) {
        goto fail;
    }
    contRgn = C14NewRgn();
    if (!contRgn) {
        goto fail;
    }
    updateRgn = C14NewRgn();
    if (!updateRgn) {
        goto fail;
    }
    titleHandle = (Str255 **)NewHandle(sizeof(Str255));
    if (!titleHandle) {
        goto fail;
    }
    
    /* C14SyncClassicWindow begin */
    status = GetWindowAttributes(carbonWindow, &attributes);
    if (status != noErr) {
        goto fail;
    }
    status = GetWindowRegion(carbonWindow, kWindowStructureRgn, (**strucRgn).carbonRgn);
    if (status != noErr) {
        goto fail;
    }
    status = GetWindowRegion(carbonWindow, kWindowContentRgn, (**contRgn).carbonRgn);
    if (status != noErr) {
        goto fail;
    }
    status = GetWindowRegion(carbonWindow, kWindowUpdateRgn, (**updateRgn).carbonRgn);
    if (status != noErr) {
        goto fail;
    }
    C14PrivateSyncRgn(strucRgn);
    C14PrivateSyncRgn(contRgn);
    C14PrivateSyncRgn(updateRgn);
    
    GetWTitle(carbonWindow, **titleHandle);
    titleRgn = NewRgn();
    if (!titleRgn) {
        goto fail;
    }
    status = GetWindowRegion(carbonWindow, kWindowTitleTextRgn, titleRgn);
    if (status != noErr) {
        goto fail;
    }
    GetRegionBounds(titleRgn, &titleBounds);
    titleWidth = titleBounds.right - titleBounds.left;
    DisposeRgn(titleRgn);
    titleRgn = nil;
    
    carbonPort = GetWindowPort(carbonWindow);
    C14PrivateSyncPort(&classicWindow->port, carbonPort, TRUE);
    
    classicWindow->windowKind = GetWindowKind(carbonWindow);
    classicWindow->visible = IsWindowVisible(carbonWindow);
    classicWindow->hilited = IsWindowHilited(carbonWindow);
    classicWindow->goAwayFlag = (attributes & kWindowCloseBoxAttribute) ? TRUE : FALSE;
    classicWindow->spareFlag = (attributes & kWindowResizableAttribute) ? TRUE : FALSE; /*?*/
    classicWindow->strucRgn = (RgnHandle)strucRgn;
    classicWindow->contRgn = (RgnHandle)contRgn;
    classicWindow->updateRgn = (RgnHandle)updateRgn;
    classicWindow->windowDefProc = nil; /*???*/
    classicWindow->dataHandle = nil; /*???*/
    classicWindow->titleHandle = (StringHandle)titleHandle;
    classicWindow->titleWidth = titleWidth;
    /*XXX these must be classic-transformed */
    GetRootControl(carbonWindow, (ControlRef *)&classicWindow->controlList);
    classicWindow->nextWindow = (WindowPeek)GetNextWindow(carbonWindow);
    classicWindow->windowPic = GetWindowPic(carbonWindow);
    classicWindow->refCon = GetWRefCon(carbonWindow);
    
    /* C14SyncClassicWindow end */
    
    c14Port = C14PrivateNewPort(FALSE);
    c14Port->classicPort = &classicWindow->port;
    c14Port->carbonPort = carbonPort;
    c14Port->carbonWindow = carbonWindow;
    
    C14PrivateSyncWindowPosition(c14Port);
    
    return c14Port->classicPort;
    
fail:
    if (titleRgn) {
        DisposeRgn(titleRgn);
    }
    if (titleHandle) {
        DisposeHandle((Handle)titleHandle);
    }
    if (updateRgn) {
        C14DisposeRgn(updateRgn);
    }
    if (contRgn) {
        C14DisposeRgn(contRgn);
    }
    if (strucRgn) {
        C14DisposeRgn(strucRgn);
    }
    if (carbonWindow) {
        DisposeWindow(carbonWindow);
    }
    if (classicStorage) {
        DisposePtr(classicStorage);
    }
    /* XXX: error code? */
    return nil;
}


DEFINE_API( void )
C14InvalRect(const Rect * badRect)
{
    InvalWindowRect(theC14Port->carbonWindow, badRect);
}



/*
 * private routines
 */

void
C14PrivateSyncWindowPosition(C14PortPtr c14Port)
{
    BitMap *portBits;
    Rect windowBounds;
    C14RgnHandle visRgn;
    
    if (!c14Port->isColor) {
        /* Window PixMaps appear to refer to buffers, rather than the screen.
           Construct the rectangle classic apps expect. */
        portBits = &c14Port->classicPort->portBits;
        GetQDGlobalsScreenBits(portBits);
        GetWindowBounds(c14Port->carbonWindow,
                        kWindowGlobalPortRgn,
                        &windowBounds);
        OffsetRect(&portBits->bounds,
                   -windowBounds.left,
                   -windowBounds.top);
    }
    
    visRgn = (C14RgnHandle)c14Port->classicPort->visRgn;
    GetPortVisibleRegion(c14Port->carbonPort, (**visRgn).carbonRgn);
    C14PrivateSyncRgn(visRgn);
}
