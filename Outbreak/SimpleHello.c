/*
 *  Copyright © 1997-2003 Metrowerks Corporation.  All Rights Reserved.
 *
 *  Questions and comments to:
 *       <mailto:support@metrowerks.com>
 *       <http://www.metrowerks.com/>
 */

#include <Carbon/Carbon.h>

#define kMainNibFileName "SimpleHello"

#define kMenuBarNibName "MenuBar"
#define kSimpleHelloNibName "MainWindow"

CFBundleRef gBundle;
IBNibRef gNibs;

static Boolean Initialize(void)
{
    OSErr theErr;
    Boolean isInitialized;
    
    isInitialized = false;
    gBundle = NULL;
    gNibs = NULL;
    
    gBundle = CFBundleGetMainBundle();
    
    if (gBundle != NULL)
    {
        theErr = CreateNibReferenceWithCFBundle(gBundle, CFSTR(kMainNibFileName), &gNibs);
        
        if ((theErr == noErr) && (gNibs != NULL))
            isInitialized = true;
    }
    
    InitCursor();
    
    return isInitialized;
}

static Boolean InstallMenus(void)
{
    OSErr theErr;
    Boolean isMenuInstalled;
    
    isMenuInstalled = false;
    
    theErr = SetMenuBarFromNib(gNibs, CFSTR(kMenuBarNibName));
    
    if (theErr == noErr)
        isMenuInstalled = true;
    
    return isMenuInstalled;
}

static void DisplaySimpleHelloWindow(void)
{
    OSErr theErr;
    WindowRef theWindow;
    
    theErr = CreateWindowFromNib(gNibs, CFSTR(kSimpleHelloNibName), &theWindow);
    
    if ((theErr == noErr) && (theWindow != NULL))
        ShowWindow(theWindow);
}

#if 0
int main(void)
{
    Boolean isSetupGood;
    
    isSetupGood = Initialize();
    
    if (isSetupGood)
        isSetupGood = InstallMenus();
    
    if (isSetupGood)
    {
        DisplaySimpleHelloWindow();
        
        RunApplicationEventLoop();
    }
    
    if (gNibs != NULL)
        DisposeNibReference(gNibs);
    
    if (gBundle != NULL)
        CFRelease(gBundle);
    
    return 0;
}
#endif