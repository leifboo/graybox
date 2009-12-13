
#ifndef __C14Quickdraw_h__
#define __C14Quickdraw_h__


#include "C14Macros.h"
#include "C14Types.h"


struct C14Region {
    MacRegion region;
    RgnHandle carbonRgn;
};


struct C14Port {
    C14PortPtr next;
    Boolean isColor;
    GrafPtr classicPort;
    CGrafPtr carbonPort;
    WindowRef carbonWindow; /* nil for non-windows */
};


/* GrafPort routines */

C14_API( void )
C14InitGraf(void * globalPtr);

C14_API( void )
C14SetOrigin(
  short   h,
  short   v);

C14_API( void )
C14SetPort(GrafPtr port);


/* calculations with regions */

C14_API( void )
C14CopyRgn(
  C14RgnHandle   srcRgn,
  C14RgnHandle   dstRgn);

C14_API( void )
C14DiffRgn(
  C14RgnHandle   srcRgnA,
  C14RgnHandle   srcRgnB,
  C14RgnHandle   dstRgn);

C14_API( void )
C14DisposeRgn(C14RgnHandle rgn);

C14_API( Boolean )
C14EmptyRgn(C14RgnHandle rgn);

C14_API( C14RgnHandle )
C14NewRgn(void);

C14_API( Boolean )
C14PtInRgn(
  Point          pt,
  C14RgnHandle   rgn);

C14_API( void )
C14RectRgn(
  C14RgnHandle     rgn,
  const Rect *     r);

C14_API( void )
C14SectRgn(
  C14RgnHandle   srcRgnA,
  C14RgnHandle   srcRgnB,
  C14RgnHandle   dstRgn);

C14_API( void )
C14SetRectRgn(
  C14RgnHandle   rgn,
  short          left,
  short          top,
  short          right,
  short          bottom);



/* private routines */

void
C14PrivateCloseCPort(CGrafPtr classicPort);

void
C14PrivateClosePort(GrafPtr classicPort);

C14PortPtr *
C14PrivateFindCarbonWindow(WindowRef carbonWindow);

C14PortPtr *
C14PrivateFindClassicPort(GrafPtr classicPort);

C14PortPtr
C14PrivateNewPort(Boolean isColor);

void
C14PrivateSyncCPort(CGrafPtr classicPort, CGrafPtr carbonPort, Boolean init);

void
C14PrivateSyncPort(GrafPtr classicPort, CGrafPtr carbonPort, Boolean init);

void
C14PrivateSyncRgn(C14RgnHandle classicRgn);


/* private variables */

#if BUILDING_CARBON_14
extern C14PortPtr theC14Port;
#endif


#endif /* __C14Quickdraw_h__ */
