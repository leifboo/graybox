
#include "C14Quickdraw.h"



C14PortPtr theC14Port;


static QDGlobals *qdGlobals;
static C14PortPtr c14PortHead;
static Ptr dummyPtr;



/*
 * GrafPort routines
 */

DEFINE_API( void )
C14InitGraf(void * globalPtr)
{
    qdGlobals = (QDGlobals *)((char *)globalPtr - offsetof(QDGlobals,thePort));
    qdGlobals->randSeed = GetQDGlobalsRandomSeed();
    GetQDGlobalsScreenBits(&qdGlobals->screenBits);
    GetQDGlobalsArrow(&qdGlobals->arrow);
    GetQDGlobalsDarkGray(&qdGlobals->dkGray);
    GetQDGlobalsLightGray(&qdGlobals->ltGray);
    GetQDGlobalsGray(&qdGlobals->gray);
    GetQDGlobalsBlack(&qdGlobals->black);
    GetQDGlobalsWhite(&qdGlobals->white);
    qdGlobals->thePort = nil;
}


DEFINE_API( void )
C14SetPort(GrafPtr classicPort)
{
    theC14Port = *C14PrivateFindClassicPort(classicPort);
    SetPort((GrafPort *)theC14Port->carbonPort);
    qdGlobals->thePort = classicPort;
}



/*
 * calculations with regions
 */

DEFINE_API( void )
C14CopyRgn(
  C14RgnHandle   srcRgn,
  C14RgnHandle   dstRgn)
{
    CopyRgn((**srcRgn).carbonRgn, (**dstRgn).carbonRgn);
    C14PrivateSyncRgn(dstRgn);
}


DEFINE_API( void )
C14DiffRgn(
  C14RgnHandle   srcRgnA,
  C14RgnHandle   srcRgnB,
  C14RgnHandle   dstRgn)
{
    DiffRgn((**srcRgnA).carbonRgn, (**srcRgnB).carbonRgn, (**dstRgn).carbonRgn);
    C14PrivateSyncRgn(dstRgn);
}


DEFINE_API( void )
C14DisposeRgn(C14RgnHandle rgn)
{
    DisposeRgn((**rgn).carbonRgn);
    DisposeHandle((Handle)rgn);
}


DEFINE_API( Boolean )
C14EmptyRgn(C14RgnHandle rgn)
{
    return EmptyRgn((**rgn).carbonRgn);
}


DEFINE_API( C14RgnHandle )
C14NewRgn()
{
    C14RgnHandle classicRgn;
    RgnHandle carbonRgn;
    
    carbonRgn = NewRgn();
    classicRgn = (C14RgnHandle)NewHandle(sizeof(C14Region));
    if (!classicRgn || !carbonRgn) {
        goto fail;
    }
    
    (**classicRgn).carbonRgn = carbonRgn;
    C14PrivateSyncRgn(classicRgn);
    
    return classicRgn;
    
fail:
    if (carbonRgn) {
        DisposeRgn(carbonRgn);
    }
    if (classicRgn) {
        DisposeHandle((Handle)classicRgn);
    }
    return nil;
}


DEFINE_API( Boolean )
C14PtInRgn(
  Point          pt,
  C14RgnHandle   rgn)
{
    return PtInRgn(pt, (**rgn).carbonRgn);
}


DEFINE_API( void )
C14RectRgn(
  C14RgnHandle  rgn,
  const Rect *  r)
{
    RectRgn((**rgn).carbonRgn, r);
    C14PrivateSyncRgn(rgn);
}


DEFINE_API( void )
C14SectRgn(
  C14RgnHandle   srcRgnA,
  C14RgnHandle   srcRgnB,
  C14RgnHandle   dstRgn)
{
    SectRgn((**srcRgnA).carbonRgn, (**srcRgnB).carbonRgn, (**dstRgn).carbonRgn);
    C14PrivateSyncRgn(dstRgn);
}


DEFINE_API( void )
C14SetRectRgn(
  C14RgnHandle  rgn,
  short         left,
  short         top,
  short         right,
  short         bottom)
{
    SetRectRgn((**rgn).carbonRgn, left, top, right, bottom);
    C14PrivateSyncRgn(rgn);
}



/*
 * private routines
 */

void
C14PrivateClosePort(CGrafPtr classicPort)
{
    DisposePixPat(classicPort->fillPixPat);
    DisposePixPat(classicPort->pnPixPat);
    DisposePixPat(classicPort->bkPixPat);
    C14DisposeRgn((C14RgnHandle)classicPort->clipRgn);
    C14DisposeRgn((C14RgnHandle)classicPort->visRgn);
}


C14PortPtr *
C14PrivateFindCarbonWindow(WindowRef carbonWindow)
{
    C14PortPtr *result;
    
    for (result = &c14PortHead; *result; result = &(*result)->next) {
        if ((*result)->carbonWindow == carbonWindow) {
            break;
        }
    }
    return result;
}


C14PortPtr *
C14PrivateFindClassicPort(GrafPtr classicPort)
{
    C14PortPtr *result;
    
    for (result = &c14PortHead; *result; result = &(*result)->next) {
        if ((*result)->classicPort == classicPort) {
            break;
        }
    }
    return result;
}


C14PortPtr
C14PrivateNewPort(void)
{
    C14PortPtr newC14Port;
    
    newC14Port = (C14PortPtr)NewPtr(sizeof(C14Port));
    newC14Port->next = c14PortHead;
    newC14Port->classicPort = nil;
    newC14Port->carbonPort = nil;
    newC14Port->carbonWindow = nil;
    
    c14PortHead = newC14Port;
    
    return newC14Port;
}


void
C14PrivateSyncPort(CGrafPtr classicPort, CGrafPtr carbonPort, Boolean init)
{
    C14RgnHandle visRgn, clipRgn;
    C14RgnHandle newVisRgn, newClipRgn;
    PixPatHandle backPattern, penPattern, fillPattern;
    PixPatHandle newBackPattern, newPenPattern, newFillPattern;
    Handle dummySave = &dummyPtr;
    
    newVisRgn = newClipRgn = nil;
    newBackPattern = newPenPattern = newFillPattern = nil;
    if (init) {
        visRgn = newVisRgn = C14NewRgn();
        clipRgn = newClipRgn = C14NewRgn();
        backPattern = newBackPattern = NewPixPat();
        penPattern = newPenPattern = NewPixPat();
        fillPattern = newFillPattern = NewPixPat();
        if (!newVisRgn || !newClipRgn ||
            !newBackPattern || !newPenPattern || !newFillPattern) {
            goto fail;
        }
    } else {
        visRgn = (C14RgnHandle)classicPort->visRgn;
        clipRgn = (C14RgnHandle)classicPort->clipRgn;
        backPattern = classicPort->bkPixPat;
        penPattern = classicPort->pnPixPat;
        fillPattern = classicPort->fillPixPat;
    }
    
    GetPortVisibleRegion(carbonPort, (**visRgn).carbonRgn);
    GetPortClipRegion(carbonPort, (**clipRgn).carbonRgn);
    
    GetPortBackPixPat(carbonPort, backPattern);
    GetPortPenPixPat(carbonPort, penPattern);
    GetPortFillPixPat(carbonPort, fillPattern);
    
    classicPort->device = 0; /* not available in Carbon */
    classicPort->portPixMap = GetPortPixMap(carbonPort);
    classicPort->portVersion = IsPortColor(carbonPort) ? 0xC000 : 0x0000; /* XXX: B&W? */
    classicPort->grafVars = nil; /* not available in Carbon */
    classicPort->chExtra = GetPortChExtra(carbonPort);
    classicPort->pnLocHFrac = GetPortFracHPenLocation(carbonPort);
    GetPortBounds(carbonPort, &classicPort->portRect);
    classicPort->visRgn = (RgnHandle)visRgn;
    classicPort->clipRgn = (RgnHandle)clipRgn;
    classicPort->bkPixPat = backPattern;
    GetPortForeColor(carbonPort, &classicPort->rgbFgColor);
    GetPortBackColor(carbonPort, &classicPort->rgbBkColor);
    GetPortPenLocation(carbonPort, &classicPort->pnLoc);
    GetPortPenSize(carbonPort, &classicPort->pnSize);
    classicPort->pnMode = GetPortPenMode(carbonPort);
    classicPort->pnPixPat = penPattern;
    classicPort->fillPixPat = fillPattern;
    classicPort->pnVis = GetPortPenVisibility(carbonPort);
    classicPort->txFont = GetPortTextFont(carbonPort);
    classicPort->txFace = GetPortTextFace(carbonPort);
    classicPort->txSize = GetPortTextMode(carbonPort);
    classicPort->spExtra = GetPortSpExtra(carbonPort);
    classicPort->fgColor = 0; /* not available in Carbon*/
    classicPort->bkColor = 0; /* not available in Carbon*/
    classicPort->colrBit = 0; /* not available in Carbon*/
    classicPort->patStretch = 0; /* not available in Carbon*/
    classicPort->picSave = IsPortPictureBeingDefined(carbonPort) ? dummySave : nil;
    classicPort->rgnSave = IsPortRegionBeingDefined(carbonPort) ? dummySave : nil;
    classicPort->polySave = IsPortPolyBeingDefined(carbonPort) ? dummySave : nil;
    classicPort->grafProcs = GetPortGrafProcs(carbonPort);
    return;

fail:
    if (newFillPattern) {
        DisposePixPat(newFillPattern);
    }
    if (newPenPattern) {
        DisposePixPat(newPenPattern);
    }
    if (newBackPattern) {
        DisposePixPat(newBackPattern);
    }
    if (newClipRgn) {
        C14DisposeRgn(newClipRgn);
    }
    if (newVisRgn) {
        C14DisposeRgn(newVisRgn);
    }
    return;
}


void
C14PrivateSyncRgn(C14RgnHandle classicRgn)
{
    C14RgnPtr classicRgnPtr;
    
    classicRgnPtr = *classicRgn;
    classicRgnPtr->region.rgnSize = 0;
    GetRegionBounds(classicRgnPtr->carbonRgn, &classicRgnPtr->region.rgnBBox);
}
