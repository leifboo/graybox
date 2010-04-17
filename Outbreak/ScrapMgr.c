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

#include "ScrapMgr.h"

#include "Patching.h"
#include "vMac.h"
#include "Logging.h"

#include <Traps.h>


#pragma options align=mac68k

typedef struct ScrapStuff {
  SInt32              scrapSize;
  Handle              scrapHandle;
  SInt16              scrapCount;
  SInt16              scrapState;
  StringPtr           scrapName;
} ScrapStuff;


static void trapXScrap(UInt16 trapWord, UInt32 regs[16])
{
    /*
     * IM II-108:
     *
     *     "If you attempt to open a file for writing
     *      and it already has an access path that allows writing,
     *      PBOpen will return the reference number
     *      of the existing access path in ioRefNum and opWrErr 
     *      as its function result."
     *
     * Carbon does not seem to exhibit the above behavior --
     * it instead returns a new reference number that lacks write
     * permission (write attempts fail with wrPermErr).
     *
     * MacDraw running on System 6 relies upon the old behavior.
     * Upon quitting, if there is anything in the clipboard,
     * MacDraw calls UnloadScrap(), which leaves the "Clipboard File" open
     * (so that subsequent calls to PutScrap can write to it).
     * Then, MacDraw reopens the clipboard file:  it bypasses
     * the Scrap Manager, instead opening ScrapStuff.scrapName
     * directly.  The resulting wrPermErr results in an error message.
     */
    
    OSStatus *resultPtr = (OSStatus *)get_real_address(regs[8+7]);
    ScrapStuff *scrapStuff = (ScrapStuff *)get_real_address(0x960);
    
    fprintlog("prescrap %lx %ld %p %d %d\n",
              *resultPtr,
              scrapStuff->scrapSize,
              scrapStuff->scrapHandle,
              scrapStuff->scrapCount,
              scrapStuff->scrapState);
    GBPerformTrap();
    fprintlog("postscrap %lx %ld %p %d %d\n",
              *resultPtr,
              scrapStuff->scrapSize,
              scrapStuff->scrapHandle,
              scrapStuff->scrapCount,
              scrapStuff->scrapState);
    (void)trapWord;
}


static void trapInfoScrap(UInt16 trapWord, UInt32 regs[16])
{
    UInt32 *resultPtr = (UInt32 *)get_real_address(regs[8+7]);
    ScrapStuff *scrapStuff = (ScrapStuff *)get_real_address(0x960);
    GBPerformTrap();
    fprintlog("infoscrap %lx %ld %p %d %d\n",
              *resultPtr,
              scrapStuff->scrapSize,
              scrapStuff->scrapHandle,
              scrapStuff->scrapCount,
              scrapStuff->scrapState);
    (void)trapWord;
}


Boolean InitScrap(void)
{
    tbTrapTable[_UnlodeScrap    & 0x3FF]    = &trapXScrap;
    tbTrapTable[_LoadScrap      & 0x3FF]    = &trapXScrap;
    tbTrapTable[_ZeroScrap      & 0x3FF]    = &trapXScrap;
    
    tbTrapTable[_InfoScrap      & 0x3FF]    = &trapInfoScrap;
    
    return true;
}
