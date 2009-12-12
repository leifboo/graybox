
#include "C14Quickdraw.h"



/*
 * GrafPort routines
 */

DEFINE_API( void )
InitGraf(void * globalPtr)
{
    C14InitGraf(globalPtr);
}

DEFINE_API( void )
SetPort(GrafPtr port)
{
    C14SetPort(port);
}



/*
 * calculations with regions
 */

DEFINE_API( void )
CopyRgn(
  RgnHandle   srcRgn,
  RgnHandle   dstRgn)
{
    C14CopyRgn((C14RgnHandle)srcRgn, (C14RgnHandle)dstRgn);
}


DEFINE_API( void )
DiffRgn(
  RgnHandle   srcRgnA,
  RgnHandle   srcRgnB,
  RgnHandle   dstRgn)
{
    C14DiffRgn((C14RgnHandle)srcRgnA, (C14RgnHandle)srcRgnB, (C14RgnHandle)dstRgn);
}


DEFINE_API( void )
DisposeRgn(RgnHandle rgn)
{
    C14DisposeRgn((C14RgnHandle)rgn);
}


DEFINE_API( Boolean )
EmptyRgn(RgnHandle rgn)
{
    return C14EmptyRgn((C14RgnHandle)rgn);
}


DEFINE_API( RgnHandle )
NewRgn(void)
{
    return (RgnHandle)C14NewRgn();
}


DEFINE_API( Boolean )
PtInRgn(
  Point       pt,
  RgnHandle   rgn)
{
    return C14PtInRgn(pt, (C14RgnHandle)rgn);
}


DEFINE_API( void )
RectRgn(
  RgnHandle     rgn,
  const Rect *  r)
{
    C14RectRgn((C14RgnHandle)rgn, r);
}


DEFINE_API( void )
SectRgn(
  RgnHandle   srcRgnA,
  RgnHandle   srcRgnB,
  RgnHandle   dstRgn)
{
    C14SectRgn((C14RgnHandle)srcRgnA, (C14RgnHandle)srcRgnB, (C14RgnHandle)dstRgn);
}


DEFINE_API( void )
SetRectRgn(
  RgnHandle   rgn,
  short       left,
  short       top,
  short       right,
  short       bottom)
{
    C14SetRectRgn((C14RgnHandle)rgn, left, top, right, bottom);
}
