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

WindowPtr windowsWindow, menusWindow;
CGrafPtr windowsPort, menusPort;
static CGContextRef menusGC, windowsGC, deskGC;
static CGrafPtr dragPort;
static RgnHandle deskRgn, maskRgn;

static Rect dirtyRect;

Boolean inMenuSelect;
static short trimMenuFlush;

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
    windowsPort = GetWindowPort(windowsWindow);
    menusPort = GetWindowPort(menusWindow);
    CreateCGContextForPort(windowsPort, &windowsGC);
    CreateCGContextForPort(windowsPort, &deskGC);
    CreateCGContextForPort(menusPort, &menusGC);

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
        CGrafPtr port;
        CGRect cgRect;
        
        GetPort(&port);
        
        /* clear alpha */
        if (port == windowsPort) {
            cgRect = CGRectFromQDRect(&dirtyRect);
            CGContextFillRect(windowsGC, cgRect);
        } else {
            if (trimMenuFlush) {
                dirtyRect.bottom = trimMenuFlush;
                trimMenuFlush = 0;
            }
            cgRect = CGRectFromQDRect(&dirtyRect);
            CGContextFillRect(menusGC, cgRect);
        }
        
        CopyBits(&screenBitMap,
                 GetPortBitMapForCopyBits(port),
                 &dirtyRect, &dirtyRect, srcCopy,
                 /* This makes document window drawing perfect,
                    but alerts get clipped. */
                 NULL /*port == windowsPort ? maskRgn : NULL*/ );
        //ForeColor(debugColor = (debugColor == redColor ? blueColor : redColor));
        //FrameRect(&dirtyRect);

        QDFlushPortBuffer(port, NULL);
        
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



/*
 * menus
 */

static void trapDrawMenuBar(UInt16 trapWord, UInt32 regs[16])
{
    ShowWindow(windowsWindow);
    ShowWindow(menusWindow);
    HideMenuBar();
    
    /* actually perform the trap */
    m68k_backup_pc();
    m68k_exception(0xA);

    (void)trapWord; (void)regs;
}


static void trapMenuSelect(UInt16 trapWord, UInt32 regs[16])
{
    UInt32 pc; UInt16 *sp;
    
    FlushDisplay();
    SetPort(menusPort);
    
    if (true) {
        /* XXX: code below causes freeze !@#$% ??? */
        increaseIndent();
        inMenuSelect = true;
        m68k_backup_pc();
        m68k_exception(0xA);
        return;
    }
    
    /*
     * Change the M68K processor context so that it is
     * as if MenuSelect() had been called from the gateway.
     *
     * Stack
     * -----
     *
     * before          after
     *
     * +8 +----------+ +12 +----------+
     *    |          |     |          |  sp[5]
     *    + result   +     + ret addr +
     *    |          |     |          |  sp[4]
     * +4 +----------+  +8 +----------+
     *    |          |     |          |  sp[3]
     *    + startPt  +     + result   +
     *    |          |     |          |  sp[2]
     * +0 +----------+  +4 +----------+
     *                     |          |  sp[1]
     *                     + startPt  +
     *                     |          |  sp[0]
     *                A7+0 +----------+
     *
     */
    
    regs[8+7] -= 4;
    sp = (UInt16 *)get_real_address(regs[8+7]);
    
    /* move argument 'startPt' */
    sp[0] = sp[2];
    sp[1] = sp[3];
    
    /* save return address
       (pointer to instruction following original trap) */
    pc = m68k_getpc();
    sp[4] = HiWord(pc);
    sp[5] = LoWord(pc);
    
    m68k_setpc(GetGatewayAddress(sysMenuSelectReturn));
    //m68k_exception(0xA);
    
    (void)trapWord;
}


void MenuSelectReturn(UInt16 trapWord, UInt32 regs[16])
{
    UInt16 *sp;
    UInt16 callerHi, callerLo;
    
    FlushDisplay();
    SetPort(windowsPort);
    inMenuSelect = false;

    /*
     * Next instruction is "RTS"; swap return value with
     * the saved return address.
     *
     * XXX:  It would be more elegant to do all of this
     *       in M68K glue code.
     */
    sp = (UInt16 *)get_real_address(regs[8+7]);
    callerHi = sp[2];
    callerLo = sp[3];
    sp[2] = sp[0];
    sp[3] = sp[1];
    sp[0] = callerHi;
    sp[1] = callerLo;
    
    (void)trapWord;
}


static void trapCopyBits(UInt16 trapWord, UInt32 regs[16])
{
    Ptr sp;
    BitMap *dstBits;
    Rect *dstRect;
    CGRect cgRect;
    
    FlushDisplay(); /*see comment below*/
    
    sp = (Ptr)get_real_address(regs[8+7]);
    
    dstBits = (BitMap *)get_real_address(*(CPTR *)(sp + 14));
    
    if (inMenuSelect && dstBits->baseAddr == vScreenBitMap.baseAddr) {
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
         * So we let CopyBits() happen, trim the next flush to preserve
         * transparency, and force the next flush to happen at the
         * next call to CopyBits(), as the Menu Manager is saving
         * the area under the next menu.
         */
        if (0) {
            regs[8+7] += 22;
            return;
        } else {
            trimMenuFlush = 20;
        }
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
    
    tbTrapTable[_DrawMenuBar    & 0x3FF]    = trapDrawMenuBar;
    tbTrapTable[_MenuSelect     & 0x3FF]    = trapMenuSelect;
    tbTrapTable[_CopyBits       & 0x3FF]    = trapCopyBits;
    
    return true;
}
