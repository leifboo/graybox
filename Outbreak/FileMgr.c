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


#include "FileMgr.h"

#include "Gateway.h"
#include "GrayBox.h"
#include "Logging.h"
#include "Patching.h"
#include "vMac.h"

#include <FSM.h>
#include <Traps.h>
#include <stdlib.h>


enum {
    /* IM II-98 */
    asyncTrpMask = 1 << asyncTrpBit,
    hfsTrpMask = 1 << noQueueBit,
    
    wdMagic = -32767
};


typedef union ParamBlock {
    ParamBlockRec pb;
    HParamBlockRec hpb;
    WDPBRec wdPBRec;
    CInfoPBRec cInfoPBRec;
    FCBPBRec fcbPBRec;
} ParamBlock;


/* our own version of 'WDCBRec' */
typedef struct WDRec {
    long procID;
    short vRefNum;
    long dirID;
} WDRec;


static WDRec *wd;
static short nWD;
static short defaultVolume;
static long defaultDirectory;
static short mountedVolumes[nDrives];



static OSErr resolveWDRefNum(short *vRefNumPtr, long *dirIDPtr) {
    short vRefNum, wdIndex;
    
    vRefNum = *vRefNumPtr;
    if (vRefNum == 0) {
        vRefNum = defaultVolume;
        if (defaultDirectory && dirIDPtr)
            *dirIDPtr = defaultDirectory;
    }
    
    if (vRefNum < 0) {
        wdIndex = vRefNum - wdMagic;
        if (wdIndex < 0 || nWD <= wdIndex) {
            /* volume ID */
            *vRefNumPtr = vRefNum;
            if (dirIDPtr && *dirIDPtr == 0)
                *dirIDPtr = fsRtDirID;
            return noErr;
        }
    } else if (0 < vRefNum && vRefNum <= nDrives) {
        /* drive number */
        *vRefNumPtr = mountedVolumes[vRefNum - 1];
        return noErr;
    } else {
        fprintlog("    resolveWDRefNum: %d: no such volume\n", vRefNum);
        return nsvErr;
    }
    
    if (!wd)
        return nsvErr;
    
    if (wd[wdIndex].vRefNum == 0) /* free entry */
        return nsvErr;
    
    *vRefNumPtr = wd[wdIndex].vRefNum;
    if (dirIDPtr)
        *dirIDPtr = wd[wdIndex].dirID;
    return noErr;
}



static OSErr MountVol(ParmBlkPtr paramBlock) {
    /* see IM II-103 */
    short drvNum;
    OSErr err;
    
    drvNum = paramBlock->ioParam.ioVRefNum;
    if (0 < drvNum && drvNum <= nDrives) {
        paramBlock->ioParam.ioVRefNum = mountedVolumes[drvNum - 1];
        err = noErr;
        fprintlog("    MountVol %d --> ioVRefNum %d\n",
                  drvNum, paramBlock->ioParam.ioVRefNum);
    } else if (drvNum > 0) {
        err = nsDrvErr;
    } else {
        err = paramErr;
    }
    return paramBlock->ioParam.ioResult = err;
}



static OSErr GetVol(ParmBlkPtr paramBlock, Boolean hfs) {
    StringPtr ioNamePtr;
    OSErr err;
    
    err = noErr;
    ioNamePtr = paramBlock->ioParam.ioNamePtr;
    if (ioNamePtr) {
        /* XXX */
        ioNamePtr = (StringPtr)get_real_address((CPTR)ioNamePtr);
        ioNamePtr[0] = 3;
        ioNamePtr[1] = 'f';
        ioNamePtr[2] = 'o';
        ioNamePtr[3] = 'o';
    }
    if (hfs) {
        WDPBPtr wdPBPtr = (WDPBPtr)paramBlock;
        wdPBPtr->ioVRefNum = defaultVolume;
        wdPBPtr->ioWDProcID = 0; /* XXX */
        wdPBPtr->ioWDVRefNum = defaultVolume;
        if (defaultDirectory) {
            /* PBHSetVol was called */
            wdPBPtr->ioWDDirID = defaultDirectory;
        } else {
            wdPBPtr->ioWDDirID = 0;
            err = resolveWDRefNum(&wdPBPtr->ioWDVRefNum, &wdPBPtr->ioWDDirID);
        }
        fprintlog("    err = %d\n"
                  "    ioVRefNum = %d\n"
                  "    ioWDVRefNum = %d\n"
                  "    ioWDDirID = %d\n",
                  err,
                  wdPBPtr->ioVRefNum,
                  wdPBPtr->ioWDVRefNum,
                  wdPBPtr->ioWDDirID
                  );
    } else {
        paramBlock->ioParam.ioVRefNum = defaultVolume;
        fprintlog("    ioVRefNum = %d\n",
                  paramBlock->ioParam.ioVRefNum);
    }
    return paramBlock->ioParam.ioResult = err;
}



static OSErr SetVol(ParmBlkPtr paramBlock, Boolean hfs) {
    WDPBPtr wdPBPtr = (WDPBPtr)paramBlock;
    StringPtr ioNamePtr;
    OSErr err;
    
    err = noErr;
    fprintlog("    ioVRefNum = %d\n",
              paramBlock->ioParam.ioVRefNum);
    if (hfs) {
        fprintlog("    ioWDDirID = %d\n",
                  wdPBPtr->ioWDDirID);
    }
    ioNamePtr = paramBlock->ioParam.ioNamePtr;
    if (ioNamePtr) {
        /* XXX */
        ioNamePtr = (StringPtr)get_real_address((CPTR)ioNamePtr);
        fprintlog("    ioNamePtr = %.*s\n",
                  ioNamePtr[0], (char *)ioNamePtr + 1);
    } else if (paramBlock->ioParam.ioVRefNum != 0) {
        if (hfs) {
            short vRefNum; long dirID;
            vRefNum = paramBlock->ioParam.ioVRefNum;
            dirID = wdPBPtr->ioWDDirID;
            err = resolveWDRefNum(&vRefNum, &dirID);
            if (err == noErr) {
                defaultVolume = vRefNum;
                defaultDirectory = dirID;
            }
        } else {
            defaultVolume = paramBlock->ioParam.ioVRefNum;
            defaultDirectory = 0;
        }
    } else {
        /*
         * Sometimes, 'ioVRefNum' is zero.
         * What does it mean?  Set the default volume...
         * to the default volume?
         */
    }
    return paramBlock->ioParam.ioResult = err;
}



static OSErr NoOp(ParmBlkPtr paramBlock) {
    return paramBlock->ioParam.ioResult = noErr;
}



/*
 * dispatch routine
 */

static void trapFMRoutine(UInt16 trapWord, UInt32 regs[16]) {
    Boolean hfs;
    UInt16 selectCode;
    ParmBlkPtr parmBlkPtr;
    ParamBlock pb;
    StringPtr ioNamePtr;
    OSErr err;
    long volDirID;
    
    logTrap(trapWord, regs);
    
    hfs = (trapWord & hfsTrpMask) != 0;
    selectCode = trapWord & ~(asyncTrpMask | hfsTrpMask);
    parmBlkPtr = (ParmBlkPtr)get_real_address(regs[8+0]);
    
    if (hfs)
        fprintlog("    *** HFS ***\n");
    
    /*
     * initialize Carbon parameter block
     */
    
    BlockZero(&pb, sizeof(pb));
    /* XXX: for now */
    BlockMove(parmBlkPtr, &pb, sizeof(ParamBlockRec));
    
    ioNamePtr = parmBlkPtr->ioParam.ioNamePtr;
    if (ioNamePtr) {
        ioNamePtr = (StringPtr)get_real_address((CPTR)ioNamePtr);
        pb.pb.ioParam.ioNamePtr = ioNamePtr;
    }
    
    if (selectCode == kFSMGetVolInfo) {
        short *BootDrivePtr = (short *)get_real_address(0x210);
        short *SfSaveDiskPtr = (short *)get_real_address(0x214);
        long *CurDirStorePtr = (long *)get_real_address(0x398);
        fprintlog("    GetVolInfo ioVolIndex = %d, ioVRefNum = %d\n",
                  parmBlkPtr->volumeParam.ioVolIndex, parmBlkPtr->volumeParam.ioVRefNum);
        fprintlog("    (BootDrive = %d, SfSaveDisk = %d, CurDirStore = %ld)\n",
                  *BootDrivePtr, *SfSaveDiskPtr, *CurDirStorePtr);
        fprintlog("    (bootVRefNum,sysDirID = %d,%d)\n",
                  bootVRefNum, sysDirID);
    } else if (selectCode == kFSMUnmountVol) {
        fprintlog("    ioVRefNum = %d, A1 = %lx\n",
                  parmBlkPtr->volumeParam.ioVRefNum,
                  regs[8+1]);
    } else if (selectCode == kFSMOpen || selectCode == kFSMOpenRF) {
        fprintlog("    ioVRefNum = %d\n",
                  parmBlkPtr->volumeParam.ioVRefNum);
    }
    
    switch (selectCode) {
    case kFSMOpen:
    case kFSMOpenRF:
    case kFSMRename: {
        Ptr ioMisc = parmBlkPtr->ioParam.ioMisc;
        if (ioMisc) {
            pb.hpb.ioParam.ioMisc
                = (Ptr)get_real_address((CPTR)ioMisc);
        }
        pb.hpb.fileParam.ioVRefNum = parmBlkPtr->fileParam.ioVRefNum;
        pb.hpb.fileParam.ioDirID = 0;
        fprintlog("    resolving ioVRefNum = %d, ioDirID = %d\n",
                  pb.hpb.fileParam.ioVRefNum,
                  pb.hpb.fileParam.ioDirID);
        err = resolveWDRefNum(&pb.hpb.fileParam.ioVRefNum,
                              &pb.hpb.fileParam.ioDirID);
        if (err != noErr)
            goto leave;
        }
        fprintlog("    resolved ioVRefNum = %d, ioDirID = %d\n",
                  pb.hpb.fileParam.ioVRefNum,
                  pb.hpb.fileParam.ioDirID);
        /* fall through */
    case kFSMClose:
    case kFSMRead:
    case kFSMWrite:
    case kFSMUnmountVol:
    case kFSMAllocate:
    case kFSMGetEOF:
    case kFSMSetEOF:
    case kFSMFlushVol:
    case kFSMGetVol:
    case kFSMGetFPos: {
        Ptr ioBuffer = parmBlkPtr->ioParam.ioBuffer;
        if (ioBuffer) {
            pb.pb.ioParam.ioBuffer
                = (Ptr)get_real_address((CPTR)ioBuffer);
        }
        break; }
    case kFSMCreate:
    case kFSMDelete:
    case kFSMGetFileInfo:
    case kFSMSetFileInfo:
        pb.hpb.fileParam.ioVRefNum = parmBlkPtr->fileParam.ioVRefNum;
        pb.hpb.fileParam.ioDirID = 0;
        err = resolveWDRefNum(&pb.hpb.fileParam.ioVRefNum,
                              &pb.hpb.fileParam.ioDirID);
        if (err != noErr)
            goto leave;
        pb.hpb.fileParam.ioFVersNum = parmBlkPtr->fileParam.ioFVersNum;
        pb.hpb.fileParam.ioFDirIndex = parmBlkPtr->fileParam.ioFDirIndex;
        break;
    case kFSMGetVolInfo:
        pb.hpb.volumeParam.ioVRefNum = parmBlkPtr->volumeParam.ioVRefNum;
        pb.hpb.volumeParam.ioVolIndex = parmBlkPtr->volumeParam.ioVolIndex;
        volDirID = 0;
        err = resolveWDRefNum(&pb.hpb.volumeParam.ioVRefNum, &volDirID);
        if (err != noErr)
            goto leave;
        break;
    }
    
    
    /*
     * call Carbon equivalent
     */
    
    switch (selectCode) {
    case kFSMOpen:          err = PBHOpenSync(&pb.hpb);         break;
    case kFSMClose:         err = PBCloseSync(&pb.pb);          break;
    case kFSMRead:          err = PBReadSync(&pb.pb);           break;
    case kFSMWrite:         err = PBWriteSync(&pb.pb);          break;
    case kFSMGetVolInfo:    err = PBHGetVInfoSync(&pb.hpb);     break;
    case kFSMCreate:        err = PBHCreateSync(&pb.hpb);       break;
    case kFSMDelete:        err = PBHDeleteSync(&pb.hpb);       break;
    case kFSMOpenRF:        err = PBHOpenRFSync(&pb.hpb);       break;
    case kFSMRename:        err = PBHRenameSync(&pb.hpb);       break;
    case kFSMGetFileInfo:   err = PBHGetFInfoSync(&pb.hpb);     break;
    case kFSMSetFileInfo:   err = PBHSetFInfoSync(&pb.hpb);     break;
    case kFSMUnmountVol:    err = PBUnmountVol(&pb.pb);         break;
    case kFSMMountVol:      err = MountVol(&pb.pb);             break;
    case kFSMAllocate:      err = PBAllocateSync(&pb.pb);       break;
    case kFSMGetEOF:        err = PBGetEOFSync(&pb.pb);         break;
    case kFSMSetEOF:        err = PBSetEOFSync(&pb.pb);         break;
    case kFSMFlushVol:      err = PBFlushVolSync(&pb.pb);       break;
    case kFSMGetVol:        err = GetVol(&pb.pb, hfs);          break;
    case kFSMSetVol:        err = SetVol(&pb.pb, hfs);          break;
    case kFSMEject:         err = NoOp(&pb.pb);                 break;
    case kFSMGetFPos:       err = PBGetFPosSync(&pb.pb);        break;

    default:
        err = pb.pb.ioParam.ioResult = unimpErr;
        break;
    }
    
    parmBlkPtr->ioParam.ioResult = pb.pb.ioParam.ioResult;
    
    if (err != noErr)
        goto leave;
    
    
    /*
     * copy out result
     */
    
    switch (selectCode) {
    case kFSMOpen:
    case kFSMOpenRF:
    case kFSMRename:
        parmBlkPtr->ioParam.ioRefNum = pb.hpb.ioParam.ioRefNum;
        /* fall through */
    case kFSMClose:
    case kFSMRead:
    case kFSMWrite:
    case kFSMUnmountVol:
    case kFSMAllocate:
    case kFSMGetEOF:
    case kFSMSetEOF:
    case kFSMFlushVol:
    case kFSMGetVol:
    case kFSMGetFPos:
        parmBlkPtr->ioParam.ioMisc = pb.pb.ioParam.ioMisc;
        parmBlkPtr->ioParam.ioActCount = pb.pb.ioParam.ioActCount;
        parmBlkPtr->ioParam.ioPosOffset = pb.pb.ioParam.ioPosOffset;
        break;
    
    case kFSMCreate:
    case kFSMDelete:
    case kFSMSetFileInfo:
        break;

    case kFSMGetFileInfo:
        if (hfs) {
            StringPtr ioNamePtr = parmBlkPtr->fileParam.ioNamePtr;
            BlockMove(&pb, parmBlkPtr, sizeof(HParamBlockRec));
            parmBlkPtr->fileParam.ioNamePtr = ioNamePtr;
        } else {
            parmBlkPtr->fileParam.ioFRefNum = pb.hpb.fileParam.ioFRefNum;
            parmBlkPtr->fileParam.ioFlAttrib = pb.hpb.fileParam.ioFlAttrib;
            parmBlkPtr->fileParam.ioFlVersNum = 0 /*hParamBlock.fileParam.ioFlVersNum*/ ;
            parmBlkPtr->fileParam.ioFlFndrInfo = pb.hpb.fileParam.ioFlFndrInfo;
            parmBlkPtr->fileParam.ioFlNum = 0; /*???*/;
            parmBlkPtr->fileParam.ioFlStBlk = pb.hpb.fileParam.ioFlStBlk;
            parmBlkPtr->fileParam.ioFlLgLen = pb.hpb.fileParam.ioFlLgLen;
            parmBlkPtr->fileParam.ioFlPyLen = pb.hpb.fileParam.ioFlPyLen;
            parmBlkPtr->fileParam.ioFlRStBlk = pb.hpb.fileParam.ioFlRStBlk;
            parmBlkPtr->fileParam.ioFlRLgLen = pb.hpb.fileParam.ioFlRLgLen;
            parmBlkPtr->fileParam.ioFlRPyLen = pb.hpb.fileParam.ioFlRPyLen;
            parmBlkPtr->fileParam.ioFlCrDat = pb.hpb.fileParam.ioFlCrDat;
            parmBlkPtr->fileParam.ioFlMdDat = pb.hpb.fileParam.ioFlMdDat;
        }
        break;

    case kFSMGetVolInfo:
        if (hfs) {
            StringPtr ioNamePtr = parmBlkPtr->volumeParam.ioNamePtr;
            BlockMove(&pb, parmBlkPtr, sizeof(HParamBlockRec));
            parmBlkPtr->volumeParam.ioNamePtr = ioNamePtr;
            
            if (pb.hpb.volumeParam.ioVRefNum == bootVRefNum ||
                pb.hpb.volumeParam.ioVRefNum == 1) {
                HParmBlkPtr hParmBlkPtr = (HParmBlkPtr)parmBlkPtr;
                int i;
                
                hParmBlkPtr->volumeParam.ioVFndrInfo[0] = sysDirID;
                for (i = 1; i < 8; ++i)
                    hParmBlkPtr->volumeParam.ioVFndrInfo[i] = 0;
            }
        } else {
            /*
             * IM IV-129: "If a working directory reference number is
             * passed in ioVRefNum (or if the default directory is a
             * subdirectory), ...the volume reference number won't be
             * returned; ioVRefNum will still contain the working
             * directory reference number."
             */
            if (parmBlkPtr->volumeParam.ioVRefNum < 0) {
                /* working directory reference number (i.e., pseudo-volume);
                   don't let the caller see behind the curtain */
            } else if (parmBlkPtr->volumeParam.ioVRefNum == 0) {
                parmBlkPtr->volumeParam.ioVRefNum = defaultVolume;
            } else {
                /* drive number passed in; convert to volume ref num */
                parmBlkPtr->volumeParam.ioVRefNum = bootVRefNum; /* XXX */
            }
            
            parmBlkPtr->volumeParam.ioVCrDate = pb.hpb.volumeParam.ioVCrDate;
            parmBlkPtr->volumeParam.ioVLsBkUp = 0; /*XXX: ???*/
            parmBlkPtr->volumeParam.ioVAtrb = pb.hpb.volumeParam.ioVAtrb;
            parmBlkPtr->volumeParam.ioVNmFls = pb.hpb.volumeParam.ioVNmFls;
            parmBlkPtr->volumeParam.ioVDirSt = 0; /*XXX: ???*/
            parmBlkPtr->volumeParam.ioVBlLn = 0; /*XXX: ???*/
            
            /*
             * IM IV-130: "Warning: IOVNmAlBlks and ioVFrBlks [sic],
             * which are actually unsigned integers, are clipped to
             * 31744 ($7C00) regardless of the size of the volume."
             */
            
            if (pb.hpb.volumeParam.ioVNmAlBlks < 0x7C00) {
                parmBlkPtr->volumeParam.ioVNmAlBlks = pb.hpb.volumeParam.ioVNmAlBlks;
            } else {
                parmBlkPtr->volumeParam.ioVNmAlBlks = 0x7C00;
            }
            
            /* header: "for compatibilty ioVAlBlkSiz is <= $0000FE00 (65,024)" */
            if (pb.hpb.volumeParam.ioVAlBlkSiz < 0xFE00) {
                parmBlkPtr->volumeParam.ioVAlBlkSiz = pb.hpb.volumeParam.ioVAlBlkSiz;
            } else {
                parmBlkPtr->volumeParam.ioVAlBlkSiz = 0xFE00;
            }
            
            /*
             * Header: "for compatibilty ioVNmAlBlks * ioVAlBlkSiz <= 2 GB".
             * Indeed, 0x7C00 * 0xFE00 == 0x7b080000.
             */
            
            parmBlkPtr->volumeParam.ioVClpSiz = pb.hpb.volumeParam.ioVClpSiz;
            parmBlkPtr->volumeParam.ioAlBlSt = pb.hpb.volumeParam.ioAlBlSt;
            parmBlkPtr->volumeParam.ioVNxtFNum = 0; /*XXX: ???*/
            
            if (pb.hpb.volumeParam.ioVFrBlk < 0x7C00) {
                parmBlkPtr->volumeParam.ioVFrBlk = pb.hpb.volumeParam.ioVFrBlk;
            } else {
                parmBlkPtr->volumeParam.ioVFrBlk = 0x7C00;
            }
        }
        
        break;
    }

 leave:
    if (err != noErr)
        logOSErr(err, ioNamePtr);
    regs[0] = err;
    m68k_test_d0();
}



static void trapHFSDispatch(UInt16 trapWord, UInt32 regs[16]) {
    OSErr err;
    ParamBlock pb, *parmBlkPtr;
    WDPBPtr wdPBPtr;
    CInfoPBPtr cInfoPBPtr;
    FCBPBPtr fcbPBPtr;
    StringPtr ioNamePtr;
    
    logTrap(trapWord, regs);
    
    parmBlkPtr = (ParamBlock *)get_real_address(regs[8+0]);
    
    /* initialize Carbon parameter block */
    BlockZero(&pb, sizeof(pb));
    /* XXX: for now */
    BlockMove(parmBlkPtr, &pb, sizeof(ParamBlock));
    
    wdPBPtr = &parmBlkPtr->wdPBRec; /* direct */
    cInfoPBPtr = &pb.cInfoPBRec;
    fcbPBPtr = &pb.fcbPBRec;
    
    ioNamePtr = parmBlkPtr->hpb.ioParam.ioNamePtr;
    if (ioNamePtr) {
        ioNamePtr = (StringPtr)get_real_address((CPTR)ioNamePtr);
        pb.pb.ioParam.ioNamePtr = ioNamePtr;
    }
    
    err = unimpErr;
    
    switch (regs[0]) {
    
    case kFSMOpenWD:
        fprintlog("    @@@ kFSMOpenWD\n");
        if (wdPBPtr->ioNamePtr) {
            /* XXX */
            err = unimpErr;
        } else if (wdPBPtr->ioWDDirID < fsRtDirID) {
            /* XXX: ??? */
            err = paramErr;
        } else if (wdPBPtr->ioWDDirID == fsRtDirID) {
            /* "If the directory specified by the ioWDDirID parameter
             * is the volume’s root directory, no working directory is created;
             * instead, the volume reference number is returned
             * in the ioVRefNum parameter."
             */
            err = noErr;
        } else {
            /* "If a working directory having the specified user identifier
             * already exists for the specified directory,
             * no new working directory is opened; instead, the existing 
             * working directory reference number is returned in ioVRefNum."
             */
            short i;
            
            if (wd) {
                for (i = 0; i < nWD; ++i) {
                    if (wd[i].procID   == wdPBPtr->ioWDProcID &&
                        wd[i].vRefNum  == wdPBPtr->ioVRefNum &&
                        wd[i].dirID    == wdPBPtr->ioWDDirID)
                    {
                        wdPBPtr->ioVRefNum = i + wdMagic;
                        err = noErr;
                        goto leave;
                    }
                }
                
                /* search for a free entry */
                for (i = 0; i < nWD; ++i) {
                    if (wd[i].vRefNum == 0)
                        goto initWD;
                }
            }
            
            i = nWD;
            ++nWD;
            wd = (WDRec *)realloc(wd, nWD * sizeof(WDRec));

 initWD:
            wd[i].procID   = wdPBPtr->ioWDProcID;
            wd[i].vRefNum  = wdPBPtr->ioVRefNum;
            wd[i].dirID    = wdPBPtr->ioWDDirID;
            
            wdPBPtr->ioVRefNum = i + wdMagic;
            err = noErr;
        }
        break;
    
    case kFSMCloseWD:
        fprintlog("    @@@ kFSMCloseWD\n");
        if (wdPBPtr->ioVRefNum >= 0) {
            err = paramErr;
        } else {
            short i;
            
            i = wdPBPtr->ioVRefNum - wdMagic;
            if (0 <= i && i < nWD) {
                wd[i].procID   = 0;
                wd[i].vRefNum  = 0;
                wd[i].dirID    = 0;
                err = noErr;
            } else {
                /* "If you specify a volume reference number, 
                   PBCloseWDSync does nothing." */
                err = noErr;
            }
        }
        break;
    
    case kFSMGetWDInfo:
        fprintlog("    @@@ kFSMGetWDInfo\n"
                  "            ioNamePtr: %p\n"
                  "            ioVRefNum: %d\n"
                  "            ioWDIndex: %d\n"
                  "            ioWDProcID: %lx\n",
                  wdPBPtr->ioNamePtr,
                  wdPBPtr->ioVRefNum,
                  wdPBPtr->ioWDIndex,
                  wdPBPtr->ioWDProcID
                  );
        /* XXX: just handle the common case */
        wdPBPtr->ioWDVRefNum = wdPBPtr->ioVRefNum;
        err = resolveWDRefNum(&wdPBPtr->ioWDVRefNum, &wdPBPtr->ioWDDirID);
        fprintlog("        err = %d; %d:%d\n", err, wdPBPtr->ioWDVRefNum, wdPBPtr->ioWDDirID);
        break;
    
    case kFSMGetFCBInfo:
        fprintlog("    @@@ kFSMGetFCBInfo\n");
        err = PBGetFCBInfoSync(fcbPBPtr);
        if (err == noErr) {
            /* XXX: for now */
            BlockMove(fcbPBPtr, parmBlkPtr, sizeof(FCBPBRec));
        }
        break;
    
    case kFSMGetCatInfo:
        fprintlog("    @@@ kFSMGetCatInfo\n");
        err = resolveWDRefNum(&cInfoPBPtr->hFileInfo.ioVRefNum, 0);
        if (err != noErr)
            goto leave;
        err = PBGetCatInfoSync(cInfoPBPtr);
        if (err == noErr) {
            /* XXX: for now */
            BlockMove(cInfoPBPtr, parmBlkPtr, sizeof(CInfoPBRec));
        }
        break;

    default:
        fprintlog("    @@@ unimp %lx\n", regs[0]);
    }
    
 leave:
    if (err != noErr)
        logOSErr(err, ioNamePtr);
    regs[0] = parmBlkPtr->pb.ioParam.ioResult = err;
    m68k_test_d0();
    (void)trapWord;
}


static void defaultToDocumentsFolder(void) {
    /* Force Standard File to default to the "Documents" folder. */
    long *CurDirStorePtr = (long *)get_real_address(0x398);
    short *SfSaveDiskPtr = (short *)get_real_address(0x214);
    short vRefNum;
    long dirID;
    OSErr err;
    
    err = FindFolder(kUserDomain, kDocumentsFolderType, kDontCreateFolder,
                     &vRefNum, &dirID);
    if (err == noErr) {
        *SfSaveDiskPtr = -vRefNum; /* must be negated (empirically determined) */
        *CurDirStorePtr = dirID;
    }
}


void ScanVolumes(void) {
    UInt32 vDrvQ; BBDrvQEl *drvQ;
    UInt32 vVCBQ; VCB *vcbQ;
    HParamBlockRec paramBlock;
    int drvIndex, volIndex;
    OSErr err;
    
    fprintlog("scanning for mounted volumes\n");
    
    BlockZero(&paramBlock, sizeof(paramBlock));
    GetGatewayFMDataStructs(&vDrvQ, &drvQ, &vVCBQ, &vcbQ);
    
    /* account for the "boot" volume which contains our system folder */
    volIndex = 0;
    vcbQ[volIndex].vcbVRefNum = mountedVolumes[volIndex] = bootVRefNum;
    vcbQ[volIndex].qLink = (QElemPtr)(vGateway + (Ptr)&vcbQ[volIndex+1] - gateway);
    fprintlog("    boot volume %d in drive %d\n", bootVRefNum, volIndex + 1);
    ++volIndex;
    
    for (drvIndex = 0; drvIndex < nDrives; ++drvIndex) {
        paramBlock.volumeParam.ioVolIndex = drvIndex + 1;
        err = PBHGetVInfoSync(&paramBlock);
        if (err != noErr)
            break;
        if (paramBlock.volumeParam.ioVRefNum == bootVRefNum)
            continue;
        fprintlog("    volume %d in drive %d\n", paramBlock.volumeParam.ioVRefNum, drvIndex + 1);
        vcbQ[volIndex].vcbVRefNum = mountedVolumes[volIndex] = paramBlock.volumeParam.ioVRefNum;
        vcbQ[volIndex].qLink = (QElemPtr)(vGateway + (Ptr)&vcbQ[volIndex+1] - gateway);
        ++volIndex;
    }
    
    vcbQ[volIndex-1].qLink = 0;
    
    for ( ; volIndex < nDrives; ++volIndex)
        mountedVolumes[volIndex] = 0;
}


static void trapInitFs(UInt16 trapWord, UInt32 regs[16]) {
    UInt32 vDrvQ; BBDrvQEl *drvQ;
    UInt32 vVCBQ; VCB *vcbQ;
    UInt32 *fsFcbLenPtr = (UInt32 *)get_real_address(0x3F6);
    QHdr *drvQHdrPtr = (QHdr *)get_real_address(0x308);
    UInt32 *defVCBPtrPtr = (UInt32 *)get_real_address(0x352);
    QHdr *vcbQHdrPtr = (QHdr *)get_real_address(0x356);
    QHdr *fsQHdrPtr = (QHdr *)get_real_address(0x360);
    Ptr *fcbSPtrPtr = (Ptr *)get_real_address(0x34E);
    short *bootDrivePtr = (short *)get_real_address(0x210);
    unsigned int i;
    
    fprintlog("InitFs\n");
    
    /* HFS present */
    *fsFcbLenPtr = 42;
    
    GetGatewayFMDataStructs(&vDrvQ, &drvQ, &vVCBQ, &vcbQ);
    
    /*
     * see "The Drive Queue", IM-II 127-128
     */
    drvQHdrPtr->qFlags = 0; /*???*/
    drvQHdrPtr->qHead = (QElemPtr)(vDrvQ + offsetof(BBDrvQEl, elem));
    drvQHdrPtr->qTail = (QElemPtr)(vDrvQ + (nDrives-1)*sizeof(BBDrvQEl) +
                                   offsetof(BBDrvQEl, elem));
    for (i = 0; i < nDrives; ++i) {
        drvQ[i].dQFlags = 0x00080000; /* nonejectable disk in drive */
        drvQ[i].elem.qLink = (QElemPtr)(vGateway + (Ptr)&drvQ[i+1].elem - gateway);
        drvQ[i].elem.qType = drvQType;
        drvQ[i].elem.dQDrive = i + 1;
        drvQ[i].elem.dQRefNum = -5; /* .Sony */
        drvQ[i].elem.dQFSID = 0;
        drvQ[i].elem.dQDrvSz = 0;
        drvQ[i].elem.dQDrvSz2 = 0;
    }
    drvQ[i-1].elem.qLink = 0;
    
    /*
     * see "Volume Control Blocks", IM-II 125
     */
    vcbQHdrPtr->qFlags = 0; /*???*/
    vcbQHdrPtr->qHead = (QElemPtr)vVCBQ;
    vcbQHdrPtr->qTail = (QElemPtr)(vVCBQ + (nDrives-1)*sizeof(VCB));
    for (i = 0; i < nDrives; ++i) {
        BlockZero(&vcbQ[i], sizeof(VCB));
        vcbQ[i].qLink = (QElemPtr)(vGateway + (Ptr)&vcbQ[i+1] - gateway);
        vcbQ[i].qType = fsQType;
        BlockMove("\pvcb", &vcbQ[i].vcbVN, 4);
        vcbQ[i].vcbDrvNum = i + 1;
        vcbQ[i].vcbDRefNum = -5; /* .Sony */
    }
    vcbQ[i-1].qLink = 0;
    
    /* default volume control block */
    *defVCBPtrPtr = vVCBQ;
    defaultVolume = bootVRefNum;
    
    /* file I/O queue header */
    fsQHdrPtr->qFlags = 0;
    fsQHdrPtr->qHead = 0;
    fsQHdrPtr->qTail = 0;
    
    /* file control blocks */
    *fcbSPtrPtr = 0;
    
    *bootDrivePtr = 1;
    
    ScanVolumes();
    defaultToDocumentsFolder();
    
    (void)trapWord; (void)regs;
}



Boolean InitFiles(void)
{
    UInt16 trapNum;
    
    for (trapNum = (_Open & 0xFF); trapNum <= (_GetFPos & 0xFF); ++trapNum) {
        osTrapTable[trapNum] = &trapFMRoutine;
    }
    
    osTrapTable[_HFSDispatch    & 0xFF]     = &trapHFSDispatch;
    osTrapTable[_InitFS         & 0xFF]     = &trapInitFs;
    
    return true;
}
