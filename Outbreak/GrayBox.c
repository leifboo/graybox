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

#include "GrayBox.h"

#include "Display.h"
#include "EventMgr.h"
#include "Gateway.h"
#include "Patching.h"
#include "vMac.h"


#define kMainNibFileName "SimpleHello"

#define kMenuBarNibName "MenuBar"
#define kSimpleHelloNibName "MainWindow"

#define RAMSize 0x00400000 /* 4MB */
#define ROMSize 0x020000 /* 128K */


CFBundleRef gBundle;
IBNibRef gNibs;

short bootVRefNum;
long rsrcDirID, sysDirID;

Ptr ROM, RAM, RAMTop;
UInt32 vRAMTop, vROMTop, vScreenTop;

static blnr dummy;


static Boolean Initialize(void)
{
    OSErr err;
    Boolean isInitialized;
    
    isInitialized = false;
    
    gBundle = CFBundleGetMainBundle();
    
    if (gBundle) {
        err = CreateNibReferenceWithCFBundle(gBundle, CFSTR(kMainNibFileName), &gNibs);
        
        if (err == noErr && gNibs)
            isInitialized = true;
    }
    
    InitCursor();
    
    return isInitialized;
}


static Boolean InstallMenus(void)
{
    OSErr err;
    
    err = SetMenuBarFromNib(gNibs, CFSTR(kMenuBarNibName));
    return err == noErr;
}


static Boolean FindResources(void)
{
    /*
     * Determine the vRefNum and dirID of our bundle's
     * "Resources" subdirectory.
     */
    CFURLRef nibURLRef;
    FSRef nibFSRef;
    FSSpec nibFSSpec;
    CInfoPBRec infoRec;
    OSErr err;
    
    nibURLRef = CFBundleCopyResourceURL(
        gBundle,
        CFSTR("Sosumi"), NULL,
        NULL);
    if (!nibURLRef)
        return false;
    if (!CFURLGetFSRef(nibURLRef, &nibFSRef)) {
        CFRelease(nibURLRef);
        return false;
    }
    CFRelease(nibURLRef);
    err = FSGetCatalogInfo(&nibFSRef, kFSCatInfoNone, NULL, NULL, 
                           &nibFSSpec, NULL);
    if (err != noErr)
        return false;
    
    bootVRefNum = nibFSSpec.vRefNum;
    rsrcDirID = nibFSSpec.parID;
    
    infoRec.dirInfo.ioCompletion = 0;
    infoRec.dirInfo.ioNamePtr = "\pSystem Folder";
    infoRec.dirInfo.ioVRefNum = bootVRefNum;
    infoRec.dirInfo.ioFDirIndex = 0;
    infoRec.dirInfo.ioDrDirID = rsrcDirID;
    err = PBGetCatInfoSync(&infoRec);
    if (err != noErr)
        return false;
    sysDirID = infoRec.dirInfo.ioDrDirID;
    
    return true;
}


static Boolean InitMemory(void)
{
    OSErr err;
    FSSpec spec;
    short refNum;
    Boolean isFolder, isAlias;
    long count;

    RAM = NewPtr(RAMSize + ROMSize);
    ROM = RAM + RAMSize;
    RAMTop = ROM;
    vRAMTop = RAMTop - RAM;
    vROMTop = vRAMTop + ROMSize;
    
    count = ROMSize;
    err = FSMakeFSSpec(bootVRefNum, rsrcDirID, "\pSosumi", &spec);
    if (err != noErr) return false;
    err = ResolveAliasFile(&spec, true, &isFolder, &isAlias);
    if (err != noErr) return false;
    err = FSpOpenDF(&spec, fsRdPerm, &refNum);
    if (err != noErr) return false;
    err = FSRead(refNum, &count, ROM);
    if (err != noErr) return false;
    FSClose(refNum);
    
    InitGateway();
    
    return true;
}


Boolean InitCrappyFiles(void); /* XXX: temporary */


int main(void)
{
#define _(e) if (!(e)) goto leave

    _(Initialize());
    if (gNibs)
        _(InstallMenus());
    
    _(FindResources());
    
    _(InitDisplay());
    _(InitMemory());
    _(InitEvents());
    _(InitMisc());
    _(InitCrappyFiles());
    
    GBPatchROM();
    
    MINEM68K_Init(BankReadAddr, BankWritAddr, &dummy);    
    m68k_reset(/*pc =*/ *(CPTR *)(ROM + 4), /*sp =*/ *(ui5b *)ROM);
    while (true)
        m68k_go_nInstructions(1024);

leave:
    if (gNibs != NULL)
        DisposeNibReference(gNibs);
    
    if (gBundle != NULL)
        CFRelease(gBundle);
    
    return 0;
}
