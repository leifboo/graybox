/*
    GrayBox
    Copyright (C) 2010  Leif Strand

    This program is free software: you can redistribute this file
    and/or modify it under the terms of version 2 of the GNU General
    Public License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Display.h"

#include "GrayBox.h"
#include "Patching.h"
#include "vMac.h"

#include <Traps.h>


BitMap screenBitMap, vScreenBitMap;
static UInt32 screenSize;

WindowPtr gMyMainWindow;
static CGrafPtr dragPort;
static CGContextRef gc;
static RgnHandle deskRgn, maskRgn;

static Rect dirtyRect;

static long debugColor;


/*
 * drawing routines
 */

static CGRect CGRectFromQDRect(Rect *r)
{
    return CGRectMake(r->left, r->top, r->right - r->left, r->bottom - r->top);
}


static void initWindows(void)
{
    Rect r;
    CGRect hr;
    WindowRef w;
    
    dragPort = CreateNewPort();
    
    hr = CGDisplayBounds(kCGDirectMainDisplay);
    r.left   = hr.origin.x;
    r.top    = hr.origin.y;
    r.right  = hr.origin.x + hr.size.width;
    r.bottom = hr.origin.y + hr.size.height;
    
    screenBitMap.rowBytes = (r.right - r.left) / 8;
    screenBitMap.bounds = r;
    /* make sure screen origin is (0,0) */
    OffsetRect(&screenBitMap.bounds, -r.left, -r.top);
    screenSize = screenBitMap.rowBytes * screenBitMap.bounds.bottom;
    
    CreateNewWindow(kOverlayWindowClass, kWindowNoAttributes, &r, &w);
    SetWindowGroup(w, GetWindowGroupOfClass(kDocumentWindowClass));
    SetPortWindowPort(w);
    EraseRect(&r);

    CreateCGContextForPort(GetWindowPort(w), &gc);
    hr = CGRectFromQDRect(&screenBitMap.bounds);
    CGContextClearRect(gc, hr);
    
    gMyMainWindow = w;
    
    screenBitMap.baseAddr = NewPtr(screenSize);
    vScreenBitMap = screenBitMap;
    vScreenBitMap.baseAddr = (Ptr)0xCD0000;
    vScreenTop = 0xCD0000 + screenSize;
    
    deskRgn = NewRgn();
    maskRgn = NewRgn();
    
    //ForeColor(debugColor = redColor);
}


static void trapScrnBitMap(UInt16 trapWord, UInt32 regs[16]) {
    BitMap *bitMap = (BitMap *)get_real_address(*(ui5b *)get_real_address(regs[8+7]));
    *bitMap = vScreenBitMap;
    regs[8+7] += 4;
    (void)trapWord;
}


void FlushDisplay(void)
{
    if (!EmptyRect(&dirtyRect)) {
        CGrafPtr port;
        Rect portRect;
        CGRect hr;
        
        port = GetWindowPort(gMyMainWindow);
        GetPortBounds(port, &portRect);
    
        CopyBits(&screenBitMap,
                 GetPortBitMapForCopyBits(port),
                 &dirtyRect, &dirtyRect, srcCopy, maskRgn);
        //ForeColor(debugColor = (debugColor == redColor ? blueColor : redColor));
        //FrameRect(&dirtyRect);

        hr = CGRectFromQDRect(&portRect);
        CGContextClearRect(gc, hr);
        QDAddRegionToDirtyRegion(port, deskRgn); /* XXX: too big? */

        QDFlushPortBuffer(GetWindowPort(gMyMainWindow), NULL);
        
        SetRect(&dirtyRect, 32767, 32767, -32767, -32767);
    }
}


void WriteFrameBuffer(UInt32 addr, UInt32 data, Boolean byteSize)
{
    int h, v;
    UInt32 offset;
    Rect r;
    
    (void)data;
    (void)byteSize;
    
    offset = addr - (UInt32)vScreenBitMap.baseAddr;
    
    v = offset / vScreenBitMap.rowBytes;
    h = 8 * (offset % vScreenBitMap.rowBytes);
    
    r.left = h;
    r.top = v;
    r.right = h + 16;
    r.bottom = v + 1;
    UnionRect(&dirtyRect, &r, &dirtyRect);
}


void WriteSmallFrameBuffer(UInt32 addr, UInt32 data, Boolean byteSize)
{
    int h, v;
    UInt32 offset;
    Rect r;
    
    (void)data;
    (void)byteSize;
    
    offset = addr - 0x3FA700;
    
    v = offset / 64;
    h = 8 * (offset % 64);
    
    r.left = h;
    r.top = v;
    r.right = h + 16;
    r.bottom = v + 1;
    UnionRect(&dirtyRect, &r, &dirtyRect);
}


void EraseDesktop(Handle classicDeskRgn)
{
    CGrafPtr port;
    Rect portRect;
    
    port = GetWindowPort(gMyMainWindow);
    GetPortBounds(port, &portRect);
    
    HandleToRgn(classicDeskRgn, deskRgn);
    RectRgn(maskRgn, &portRect);
    DiffRgn(maskRgn, deskRgn, maskRgn);
    
    ClipCGContextToRegion(gc, &portRect, deskRgn);
    
    UnionRect(&dirtyRect, &portRect, &dirtyRect);
}


/*
 * cursor routines
 */

static void trapInitCursor(UInt16 trapWord, UInt32 regs[16]) {
    InitCursor();
    (void)trapWord; (void)regs;
}

static void trapSetCursor(UInt16 trapWord, UInt32 regs[16]) {
    Cursor **argPtr = (Cursor **)get_real_address(regs[8+7]);
    SetCursor((Cursor *)get_real_address((CPTR)*argPtr));
    regs[8+7] += 4;
    (void)trapWord;
}

static void trapHideCursor(UInt16 trapWord, UInt32 regs[16]) {
    HideCursor();
    (void)trapWord; (void)regs;
}

static void trapShowCursor(UInt16 trapWord, UInt32 regs[16]) {
    ShowCursor();
    (void)trapWord; (void)regs;
}

static void trapShieldCursor(UInt16 trapWord, UInt32 regs[16]) {
    const Rect *shieldRect = (Rect *)get_real_address(*(ui5b *)get_real_address(regs[8+7] + 4));
    Point offsetPt = *(Point *)get_real_address(regs[8+7]);
    ShieldCursor(shieldRect, offsetPt);
    (void)trapWord;
}

static void trapObscureCursor(UInt16 trapWord, UInt32 regs[16]) {
    ObscureCursor();
    (void)trapWord; (void)regs;
}



/*
 * menus, windows
 */

static void trapDragGrayRgn(UInt16 trapWord, UInt32 regs[16])
{
    long result;        /* 22 */
    Handle theRgn;      /* 18 */
    Point startPt;      /* 14 */
    Rect *limitRect;    /* 10 */
    Rect *slopRect;     /*  6 */
    short axis;         /*  4 */
    ProcPtr actionProc; /*  0 */
    
    Ptr sp; long *resultPtr;
    RgnHandle carbonRgn;
    CGrafPtr oldPort;
    
    sp = (Ptr)get_real_address(regs[8+7]);
    
    theRgn = (Handle)get_real_address(*(CPTR *)(sp + 18));
    startPt = *(Point *)(sp + 14);
    limitRect = (Rect *)get_real_address(*(CPTR *)(sp + 10));
    slopRect = (Rect *)get_real_address(*(CPTR *)(sp + 6));
    axis = *(short *)(sp + 4);
    actionProc = *(ProcPtr *)(sp + 0);
    
    Ptr save = *theRgn;
    *theRgn = (Ptr)get_real_address((CPTR)save);
    carbonRgn = NewRgn();
    HandleToRgn(theRgn, carbonRgn);
    
    QDSwapPort(dragPort, &oldPort);
    result = DragGrayRgn(carbonRgn,
                         startPt,
                         limitRect,
                         slopRect,
                         axis,
                         NULL /*actionProc*/ );
    SetPort(oldPort);
    
    resultPtr = (long *)(sp + 22);
    *resultPtr = result;
    regs[8+7] += 22; /*pop*/
    
    RgnToHandle(carbonRgn, theRgn);
    *theRgn = save;
    DisposeRgn(carbonRgn);

    (void)trapWord;
}


static void trapDrawMenuBar(UInt16 trapWord, UInt32 regs[16])
{
    ShowWindow(gMyMainWindow);
    HideMenuBar();
    
    /* actually perform the trap */
    m68k_backup_pc();
    m68k_exception(0xA);

    (void)trapWord; (void)regs;
}


/*
 * initialization
 */

Boolean InitDisplay(void)
{
    initWindows();
    
    tbTrapTable[_ScrnBitMap     & 0x3FF]    = trapScrnBitMap;
    
    tbTrapTable[_InitCursor     & 0x3FF]    = trapInitCursor;
    tbTrapTable[_SetCursor      & 0x3FF]    = trapSetCursor;
    tbTrapTable[_HideCursor     & 0x3FF]    = trapHideCursor;
    tbTrapTable[_ShowCursor     & 0x3FF]    = trapShowCursor;
    tbTrapTable[_ShieldCursor   & 0x3FF]    = trapShieldCursor;
    tbTrapTable[_ObscureCursor  & 0x3FF]    = trapObscureCursor;
    /* let GetCursor pass through */

    tbTrapTable[_DragGrayRgn    & 0x3FF]    = trapDragGrayRgn;
    tbTrapTable[_DrawMenuBar    & 0x3FF]    = trapDrawMenuBar;

    return true;
}
