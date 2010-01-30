//
//  main.c
//  Transparent
//
//  Created by Leif Strand on 1/25/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#include <Carbon/Carbon.h>

static DEFINE_API(OSStatus) AppEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static OSStatus        HandleNew();
static DEFINE_API(OSStatus) WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );

static IBNibRef        sNibRef;


//--------------------------------------------------------------------------------------------

void doTransparent(void)
{
    Rect r;
    CGRect hr;
    WindowRef w;
    CGContextRef cg;
    int i;
    PixMapHandle pixMapHandle;
    UInt32 *baseAddr;
    
    r.left = 0;
    r.top = 0;
    r.right = CGDisplayPixelsWide(kCGDirectMainDisplay);
    r.bottom = CGDisplayPixelsHigh(kCGDirectMainDisplay);
    
    CreateNewWindow(kOverlayWindowClass, kWindowNoAttributes, &r, &w);
    SetWindowGroup(w, GetWindowGroupOfClass(kDocumentWindowClass));
    ShowWindow(w);
    SetPortWindowPort(w);
    EraseRect(&r);

    CreateCGContextForPort(GetWindowPort(w), &cg);
    hr = CGRectMake(r.left, r.top, r.right-r.left, r.bottom-r.top);
    CGContextClearRect(cg, hr);

    // Do your drawing here
    for (i = 0; i < 5; ++i) {
        r.left = 100*i;
        r.top = 10;
        r.right = 100*i + 50;
        //r.bottom = 300;
        PaintRect(&r);
    }
    r.top += 100;
    hr = CGRectMake(r.left, r.top, r.right-r.left, r.bottom-r.top);
    CGContextClearRect(cg, hr);
    r.left = 0; r.top = 0;
    r.right = 500;
    r.bottom = 400;
    FrameRect(&r);
    
    LockPortBits(GetWindowPort(w));
    //QDAddRectToDirtyRegion(GetWindowPort(w), &r);
    pixMapHandle = GetPortPixMap(GetWindowPort(w));
    baseAddr = (UInt32 *)((**pixMapHandle).baseAddr);
    for (i = 1600*100; i < 1600*101; ++i) {
        baseAddr[i] = 0x44ff0000;
    }
    UnlockPortBits(GetWindowPort(w));

    CGContextRelease(cg);
    //QDFlushPortBuffer(GetWindowPort(w), NULL);
    HideMenuBar();
    
    {
        RgnHandle rgn;
        Handle fRgn;
        
        rgn = NewRgn();
        OpenRgn();
        FrameRect(&r);
        CloseRgn(rgn);
        fRgn = NewHandle(0);
        RgnToHandle(rgn, fRgn); /*seems to be in classic format/
        DisposeRgn(rgn);
        DisposeHandle(fRgn);
        /* HandleToRgn, and ClipCGContextToRegion */
        
        /*alternative: BitMapToRegion spans the gamut*/
    }
}

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    OSStatus                    err;
    static const EventTypeSpec    kAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess }
    };

    // Create a Nib reference, passing the name of the nib file (without the .nib extension).
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference( CFSTR("SimpleHello"), &sNibRef );
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib( sNibRef, CFSTR("MenuBar") );
    require_noerr( err, CantSetMenuBar );
    
    // Install our handler for common commands on the application target
    InstallApplicationEventHandler( NewEventHandlerUPP( AppEventHandler ),
                                    GetEventTypeCount( kAppEvents ), kAppEvents,
                                    0, NULL );
    
    // Create a new window. A full-fledged application would do this from an AppleEvent handler
    // for kAEOpenApplication.
    //HandleNew();
    doTransparent();
    
    // Run the event loop
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:
    return err;
}

//--------------------------------------------------------------------------------------------
static DEFINE_API(OSStatus) AppEventHandler ( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
    OSStatus    result = eventNotHandledErr;
    
    switch ( GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
        {
            HICommandExtended cmd;
            verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd ) );
            
            switch ( GetEventKind( inEvent ) )
            {
                case kEventCommandProcess:
                    switch ( cmd.commandID )
                    {
                        case kHICommandNew:
                            result = HandleNew();
                            break;
                            
                        // Add your own command-handling cases here
                        
                        default:
                            break;
                    }
                    break;
            }
            break;
        }
            
        default:
            break;
    }
    
    return result;
}

//--------------------------------------------------------------------------------------------
DEFINE_ONE_SHOT_HANDLER_GETTER( WindowEventHandler )

//--------------------------------------------------------------------------------------------
static OSStatus
HandleNew()
{
    OSStatus                    err;
    WindowRef                    window;
    static const EventTypeSpec    kWindowEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess }
    };
    
    // Create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib( sNibRef, CFSTR("MainWindow"), &window );
    require_noerr( err, CantCreateWindow );

    // Install a command handler on the window. We don't use this handler yet, but nearly all
    // Carbon apps will need to handle commands, so this saves everyone a little typing.
    InstallWindowEventHandler( window, GetWindowEventHandlerUPP(),
                               GetEventTypeCount( kWindowEvents ), kWindowEvents,
                               window, NULL );
    
    // Position new windows in a staggered arrangement on the main screen
    RepositionWindow( window, NULL, kWindowCascadeOnMainScreen );
    
    // The window was created hidden, so show it
    ShowWindow( window );
    
CantCreateWindow:
    return err;
}

//--------------------------------------------------------------------------------------------
static DEFINE_API(OSStatus) WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
    OSStatus    err = eventNotHandledErr;
    
    switch ( GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
        {
            HICommandExtended cmd;
            verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd ) );
            
            switch ( GetEventKind( inEvent ) )
            {
                case kEventCommandProcess:
                    switch ( cmd.commandID )
                    {
                        // Add your own command-handling cases here
                        
                        default:
                            break;
                    }
                    break;
            }
            break;
        }
            
        default:
            break;
    }
    
    return err;
}
