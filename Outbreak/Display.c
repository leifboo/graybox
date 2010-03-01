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

#include "Gateway.h"
#include "GrayBox.h"
#include "Logging.h"
#include "Patching.h"
#include "vMac.h"

#include <Traps.h>


BitMap screenBitMap, vScreenBitMap;
static UInt32 screenSize;

WindowPtr windowsWindow, menusWindow, debugWindow;
CGrafPtr windowsPort, menusPort, debugPort;
static CGContextRef menusGC, windowsGC, deskGC, debugGC;
static CGrafPtr dragPort;
static RgnHandle deskRgn, maskRgn;

static Rect dirtyRect;

static int inMenuManager, inPaintOne;

static long debugColor;


/*
 * drawing routines
 */

static CGRect CGRectFromQDRect(Rect *r)
{
    /* XXX: How to map from QD origin to CG origin? */
    return CGRectMake(r->left, 1200 - r->bottom, r->right - r->left, r->bottom - r->top);
}


static void initWindows(void)
{
    Rect r;
    CGRect db;
    
    dragPort = CreateNewPort();
    
    /* XXX: Y axis is reversed */
    db = CGDisplayBounds(kCGDirectMainDisplay);
    r.left   = db.origin.x;
    r.top    = db.origin.y;
    r.right  = db.origin.x + db.size.width;
    r.bottom = db.origin.y + db.size.height;
    
    screenBitMap.rowBytes = (r.right - r.left) / 8;
    screenBitMap.bounds = r;
    /* make sure screen origin is (0,0) */
    OffsetRect(&screenBitMap.bounds, -r.left, -r.top);
    screenSize = screenBitMap.rowBytes * screenBitMap.bounds.bottom;
    
    CreateNewWindow(kOverlayWindowClass, kWindowNoAttributes, &r, &windowsWindow);
    SetWindowGroup(windowsWindow, GetWindowGroupOfClass(kDocumentWindowClass));
    CreateNewWindow(kOverlayWindowClass, kWindowNoAttributes, &r, &menusWindow);
    SetWindowGroup(menusWindow, GetWindowGroupOfClass(kDocumentWindowClass));
    CreateNewWindow(kOverlayWindowClass, kWindowNoAttributes, &r, &debugWindow);
    SetWindowGroup(debugWindow, GetWindowGroupOfClass(kFloatingWindowClass));
    windowsPort = GetWindowPort(windowsWindow);
    menusPort = GetWindowPort(menusWindow);
    debugPort = GetWindowPort(debugWindow);
    CreateCGContextForPort(windowsPort, &windowsGC);
    CreateCGContextForPort(windowsPort, &deskGC);
    CreateCGContextForPort(menusPort, &menusGC);
    CreateCGContextForPort(debugPort, &debugGC);
    
    //CGContextClearRect(debugGC, db);
    
    SetPort(windowsPort);
    
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
        CGrafPtr port; Rect portRect;
        CGRect cgRect;
        
        GetPort(&port);
        GetPortBounds(port, &portRect);
        
        /* clear alpha */
        cgRect = CGRectFromQDRect(&dirtyRect);
        if (port == windowsPort) {
            ClipCGContextToRegion(windowsGC, &portRect, maskRgn);
        }
        CGContextFillRect(port == windowsPort ? windowsGC : menusGC, cgRect);
        
        CopyBits(&screenBitMap,
                 GetPortBitMapForCopyBits(port),
                 &dirtyRect, &dirtyRect, srcCopy,
                 port == windowsPort ? maskRgn : NULL);

        /*QDFlushPortBuffer(port, NULL);*/
        /* flush CGContextFillRect() in addition to CopyBits() */
        CGContextFlush(port == windowsPort ? windowsGC : menusGC);
        
        SetRect(&dirtyRect, 32767, 32767, -32767, -32767);
    }
}


void WriteFrameBuffer(UInt32 addr, UInt32 data, Boolean byteSize)
{
    /*
     * Compute a coarse 'dirtyRect' that bounds all of the
     * M68K B&W Quickdraw's drawing since the last call to FlushDisplay().
     *
     * Since the B&W buffer is eight pixels per byte with no alpha,
     * it is impossible for this routine to determine clipping/alpha
     * accurately.  Most of the code complexity in this file deals with
     * clipping the CopyBits() operation so that transparency is
     * preserved.
     */
    
    int h, v;
    UInt32 offset;
    Rect r;
    
    (void)data;
    (void)byteSize;
    
    if (inMenuManager)
        return;
    
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



/*
 * desktop
 */

void EraseDesktop(Handle classicDeskRgn)
{
    Rect portRect;
    CGRect cgRect;
    
    GetPortBounds(windowsPort, &portRect);
    
    HandleToRgn(classicDeskRgn, deskRgn);
    RectRgn(maskRgn, &portRect);
    DiffRgn(maskRgn, deskRgn, maskRgn);
    
    ClipCGContextToRegion(deskGC, &portRect, deskRgn);
    
    cgRect = CGRectFromQDRect(&portRect);
    CGContextClearRect(deskGC, cgRect);
    CGContextFlush(deskGC);
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
 * windows
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


static void trapPaintOne(UInt16 trapWord, UInt32 regs[16]) {
    ++inPaintOne;
    GBPerformTrap();
    --inPaintOne;
    (void)trapWord; (void)regs;
}



/*
 * menus
 */

static void trapMenuDrawing(UInt16 trapWord, UInt32 regs[16])
{
    static int once = 1;
    
    if (once) {
        ShowWindow(windowsWindow);
        ShowWindow(menusWindow);
        ShowWindow(debugWindow);
        HideMenuBar();
        once = 0;
    }
    
    if (inMenuManager) {
        /* Intercepting internal calls to HiliteMenu causes a system error.
           Is the Menu Manager code recursive? */
        m68k_backup_pc();
        m68k_exception(0xA);
        return;
    }
    
    ++inMenuManager;
    if (inMenuManager == 1) {
        FlushDisplay();
        SetPort(menusPort);
    }
    
    GBPerformTrap();
    
    --inMenuManager;
    if (inMenuManager == 0) {
        FlushDisplay();
        SetPort(windowsPort);
    }
    
    (void)trapWord; (void)regs;
}


static void trapCopyBits(UInt16 trapWord, UInt32 regs[16])
{
    Ptr sp;
    BitMap *dstBits;
    Rect *dstRect;
    CGRect cgRect;
    
    sp = (Ptr)get_real_address(regs[8+7]);
    
    dstBits = (BitMap *)get_real_address(*(CPTR *)(sp + 14));
    
    if (inMenuManager && dstBits->baseAddr == vScreenBitMap.baseAddr) {
        /*
         * The Menu Manager is attempting to clear a menu;
         * clear to transparent instead.
         */
        
        dstRect = (Rect *)get_real_address(*(CPTR *)(sp + 6));
        cgRect = CGRectFromQDRect(dstRect);
        CGContextClearRect(menusGC, cgRect);
        CGContextFlush(menusGC);
        
        /*
         * Skipping the trap leaves garbage on the document windows;
         * but letting the CopyBits() happen clobbers the transparency.
         * So we let CopyBits() happen, and force a trimmed flush
         * to preserve transparency.
         */
        GBPerformTrap(); /* CopyBits() */
        UnionRect(&dirtyRect, dstRect, &dirtyRect);
        dirtyRect.bottom = 20;
        FlushDisplay();
    
    } else {
        m68k_backup_pc();
        m68k_exception(0xA);
    }
    
    (void)trapWord;
}


static void trapLines(UInt16 trapWord, UInt32 regs[16]) {
    Point p;
    
    if (inMenuManager) {
        p = *(Point *)get_real_address(regs[8+7]);
        
        if (trapWord == _LineTo) {
            /* clear alpha, assuming straight lines */
            PenState penState;
            Rect lineRect, begin, end;
            CGRect alphaRect;
            
            GetPenState(&penState);
            SetRect(&begin,
                    penState.pnLoc.h,
                    penState.pnLoc.v,
                    penState.pnLoc.h + penState.pnSize.h,
                    penState.pnLoc.v + penState.pnSize.h);
            SetRect(&end,
                    p.h,
                    p.v,
                    p.h + penState.pnSize.h,
                    p.v + penState.pnSize.h);
            UnionRect(&begin, &end, &lineRect);
            alphaRect = CGRectFromQDRect(&lineRect);
            CGContextFillRect(menusGC, alphaRect);
        }
        
        switch (trapWord) {
        case _MoveTo: MoveTo(p.h, p.v); break;
        case _LineTo: LineTo(p.h, p.v); break;
        case _PenSize: PenSize(p.h, p.v); break;
        }
        
    }
    
    m68k_backup_pc();
    m68k_exception(0xA);
    (void)trapWord;
}

static void trapPenNormal(UInt16 trapWord, UInt32 regs[16]) {
    if (inMenuManager) {
        PenNormal();
    }
    m68k_backup_pc();
    m68k_exception(0xA);
    (void)trapWord; (void)regs;
}



/*
 * misc. drawing
 */

static void trapRect(UInt16 trapWord, UInt32 regs[16]) {
    Ptr sp;
    Rect *r;
    RgnHandle rgn;
    
    sp = (Ptr)get_real_address(regs[8+7]);
    r = (Rect *)get_real_address(*(CPTR *)(sp + 0));
    
    if (inMenuManager) {
        /*
         * This gives us the precision needed for pull-down menus,
         * which don't use regions.  The shadow is handled by
         * trapLines().
         *
         */
        UnionRect(&dirtyRect, r, &dirtyRect);
    
    } else if (inPaintOne) {
        /*
         * When a new window appears, the visible desktop region
         * changes without our "DeskHook", EraseDesktop(), being called.
         * So here, we accumulate new windows into our mask region.
         */
        rgn = NewRgn();
        RectRgn(rgn, r);
        UnionRgn(rgn, maskRgn, maskRgn);
        DisposeRgn(rgn);
    }
    
    m68k_backup_pc();
    m68k_exception(0xA);
    
    (void)trapWord;
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
    
    tbTrapTable[_DrawMenuBar    & 0x3FF]    = trapMenuDrawing;
    tbTrapTable[_MenuSelect     & 0x3FF]    = trapMenuDrawing;
    tbTrapTable[_HiliteMenu     & 0x3FF]    = trapMenuDrawing;
    tbTrapTable[_CopyBits       & 0x3FF]    = trapCopyBits;
    tbTrapTable[_FrameRect      & 0x3FF]    = trapRect;
    tbTrapTable[_PaintRect      & 0x3FF]    = trapRect;
    tbTrapTable[_EraseRect      & 0x3FF]    = trapRect;
    tbTrapTable[_InverRect      & 0x3FF]    = trapRect;
    tbTrapTable[_MoveTo         & 0x3FF]    = trapLines;
    tbTrapTable[_LineTo         & 0x3FF]    = trapLines;
    tbTrapTable[_PenSize        & 0x3FF]    = trapLines;
    tbTrapTable[_PenNormal      & 0x3FF]    = trapPenNormal;
    tbTrapTable[_PaintOne       & 0x3FF]    = trapPaintOne;

    return true;
}
