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



/*
 * This is an "MFS level" implementation of the File Manager.
 * It creates a pair of virtual drives and a pair of virtual volumes
 * which map to a pair of folders on the user's HFS+ hard drive:
 * our "System Folder", and the user's "Documents" folder.
 *
 * This code is just good enough to boot, fool Standard File, and open
 * and save files in MacDraw.  Since we neglect to set the low memory
 * global 'FsFcbLen' (0x3F6), Standard File assumes that HFS is not
 * present, and avoids HFS calls.
 *
 * The one HFS call that does occur comes from the Mac Plus ROM:
 * PBHGetVInfoSync() at offset 0x9E2, shortly after InitFs().
 * So we handle that one case.
 * 
 */


Boolean InitMFS(void);


#include "Gateway.h"
#include "GrayBox.h"
#include "Logging.h"
#include "Patching.h"
#include "vMac.h"

#include <FSM.h>
#include <Traps.h>


enum {
    /* IM II-98 */
    asyncTrpMask = 1 << asyncTrpBit,
    hfsTrpMask = 1 << noQueueBit,
    
    wdMagic = -32767
};


typedef union ParamBlock {
    ParamBlockRec pb;
    HParamBlockRec hpb;
} ParamBlock;



static struct {
    short vRefNum;
    long dirID;
} wd[nDrives];

static short defaultVolume;



static OSErr resolveVRefNum(short *vRefNumPtr, long *dirIDPtr) {
    short vRefNum, wdIndex;
    
    vRefNum = *vRefNumPtr;
    if (vRefNum == 0)
        vRefNum = defaultVolume;
    
    if (vRefNum < 0) {
        wdIndex = vRefNum - wdMagic;
        if (wdIndex < 0 || nDrives <= wdIndex) {
            fprintlog("    resolveVRefNum: %d: no such volume\n", vRefNum);
            return nsvErr;
        }
    } else if (0 < vRefNum && vRefNum <= nDrives) {
        wdIndex = vRefNum - 1;
    } else {
        fprintlog("    resolveVRefNum: %d: no such volume\n", vRefNum);
        return nsvErr;
    }
    
    *vRefNumPtr = wd[wdIndex].vRefNum;
    if (dirIDPtr)
        *dirIDPtr = wd[wdIndex].dirID;
    return noErr;
}



static OSErr MountVol(ParmBlkPtr paramBlock) {
    /* see IM II-103 */
    short drvNum, wdNum;
    OSErr err;
    
    drvNum = paramBlock->ioParam.ioVRefNum;
    if (0 < drvNum && drvNum <= nDrives) {
        wdNum = drvNum - 1 + wdMagic;
        paramBlock->ioParam.ioVRefNum = wdNum;
        err = noErr;
        fprintlog("    drvNum = %d --> ioVRefNum %d\n",
                  drvNum, paramBlock->ioParam.ioVRefNum);
    } else if (drvNum > 0) {
        err = nsDrvErr;
    } else {
        err = paramErr;
    }
    return paramBlock->ioParam.ioResult = err;
}



static OSErr GetVol(ParmBlkPtr paramBlock) {
    StringPtr ioNamePtr;
    
    ioNamePtr = paramBlock->ioParam.ioNamePtr;
    if (ioNamePtr) {
        /* XXX */
        ioNamePtr = (StringPtr)get_real_address((CPTR)ioNamePtr);
        ioNamePtr[0] = 3;
        ioNamePtr[1] = 'f';
        ioNamePtr[2] = 'o';
        ioNamePtr[3] = 'o';
    }
    paramBlock->ioParam.ioVRefNum = defaultVolume;
    fprintlog("    ioVRefNum = %d\n",
              paramBlock->ioParam.ioVRefNum);
    return paramBlock->ioParam.ioResult = noErr;
}



static OSErr SetVol(ParmBlkPtr paramBlock) {
    /*
     * XXX: Currently, we ignore this call.
     *
     * All the SetVol calls specify a 'ioVRefNum' of zero.
     * What does it mean?  Set the default volume...
     * to the default volume?
     */
    StringPtr ioNamePtr;
    
    fprintlog("    ioVRefNum = %d\n",
              paramBlock->ioParam.ioVRefNum);
    ioNamePtr = paramBlock->ioParam.ioNamePtr;
    if (ioNamePtr) {
        /* XXX */
        ioNamePtr = (StringPtr)get_real_address((CPTR)ioNamePtr);
        fprintlog("    ioNamePtr = %.*s\n",
                  ioNamePtr[0], (char *)ioNamePtr + 1);
    }
    return paramBlock->ioParam.ioResult = noErr;
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
    
    /*logTrap(trapWord, regs);*/
    
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
        err = resolveVRefNum(&pb.hpb.fileParam.ioVRefNum,
                             &pb.hpb.fileParam.ioDirID);
        if (err != noErr)
            goto leave;
        }
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
        err = resolveVRefNum(&pb.hpb.fileParam.ioVRefNum,
                             &pb.hpb.fileParam.ioDirID);
        if (err != noErr)
            goto leave;
        pb.hpb.fileParam.ioFVersNum = parmBlkPtr->fileParam.ioFVersNum;
        pb.hpb.fileParam.ioFDirIndex = parmBlkPtr->fileParam.ioFDirIndex;
        break;
    case kFSMGetVolInfo:
        pb.hpb.volumeParam.ioVRefNum = parmBlkPtr->volumeParam.ioVRefNum;
        pb.hpb.volumeParam.ioVolIndex = parmBlkPtr->volumeParam.ioVolIndex;
        err = resolveVRefNum(&pb.hpb.volumeParam.ioVRefNum, &volDirID);
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
    case kFSMGetVol:        err = GetVol(&pb.pb);               break;
    case kFSMSetVol:        err = SetVol(&pb.pb);               break;
    case kFSMEject:         err = NoOp(&pb.pb);                 break;
    case kFSMGetFPos:       err = PBGetFPosSync(&pb.pb);        break;

    default:
        err = pb.pb.ioParam.ioResult = unimpErr;
        break;
    }
    
    if (err != noErr)
        goto leave;
    
    
    /*
     * copy out result
     */
    
    parmBlkPtr->ioParam.ioResult = pb.pb.ioParam.ioResult;
    
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
    case kFSMGetFileInfo:
    case kFSMSetFileInfo:
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
        break;

    case kFSMGetVolInfo:
        if (pb.hpb.volumeParam.ioNamePtr) {
            /* Replace volume name with folder name. */
            CInfoPBRec cInfoPB;
            
            BlockZero(&cInfoPB, sizeof(cInfoPB));
            cInfoPB.dirInfo.ioNamePtr = pb.hpb.volumeParam.ioNamePtr;
            cInfoPB.dirInfo.ioFDirIndex = -1;
            cInfoPB.dirInfo.ioVRefNum = pb.hpb.volumeParam.ioVRefNum;
            cInfoPB.dirInfo.ioDrDirID = volDirID;
            PBGetCatInfoSync(&cInfoPB);
        }
        
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
            /* drive number passed in; convert to WD ref num */
            parmBlkPtr->volumeParam.ioVRefNum += - 1 + wdMagic;
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
        
        if (hfs) {
            HParmBlkPtr hParmBlkPtr = (HParmBlkPtr)parmBlkPtr;
            int i;
            
            hParmBlkPtr->volumeParam.ioVSigWord = 0;
            hParmBlkPtr->volumeParam.ioVDrvInfo = 0;
            hParmBlkPtr->volumeParam.ioVDRefNum = 0;
            hParmBlkPtr->volumeParam.ioVFSID = 0;
            hParmBlkPtr->volumeParam.ioVBkUp = 0;
            hParmBlkPtr->volumeParam.ioVSeqNum = 0;
            hParmBlkPtr->volumeParam.ioVWrCnt = 0;
            hParmBlkPtr->volumeParam.ioVFilCnt = 0;
            hParmBlkPtr->volumeParam.ioVDirCnt = 0;
            
            /*
             * Make sure the boot code doesn't try to create
             * a working directory for the system folder:
             * the "blessed folder" is the root directory
             * of our virtual volume.
             */
            hParmBlkPtr->volumeParam.ioVFndrInfo[0] = fsRtDirID;
            
            for (i = 1; i < 8; ++i)
                hParmBlkPtr->volumeParam.ioVFndrInfo[i] = 0;
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
    logTrap(trapWord, regs);
    regs[0] = unimpErr;
    m68k_test_d0();
    (void)trapWord; (void)regs;
}


static void trapInitFs(UInt16 trapWord, UInt32 regs[16]) {
    UInt32 vDrvQ; BBDrvQEl *drvQ;
    UInt32 vVCBQ; VCB *vcbQ;
    QHdr *drvQHdrPtr = (QHdr *)get_real_address(0x308);
    UInt32 *defVCBPtrPtr = (UInt32 *)get_real_address(0x352);
    QHdr *vcbQHdrPtr = (QHdr *)get_real_address(0x356);
    QHdr *fsQHdrPtr = (QHdr *)get_real_address(0x360);
    Ptr *fcbSPtrPtr = (Ptr *)get_real_address(0x34E);
    short *bootDrivePtr = (short *)get_real_address(0x210);
    unsigned int i;
    OSErr err;
    
    fprintlog("InitFs\n");
    
    wd[0].vRefNum = bootVRefNum;
    wd[0].dirID = sysDirID;
    
    err = FindFolder(kUserDomain, kDocumentsFolderType, kDontCreateFolder,
                     &wd[1].vRefNum, &wd[1].dirID);
    fprintlog("    FindFolder: %d %d %ld\n", err, wd[1].vRefNum, wd[1].dirID);
    
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
        vcbQ[i].vcbFSID = 0;
        vcbQ[i].vcbVRefNum = (short)(0x8001 + i);
    }
    vcbQ[i-1].qLink = 0;
    
    /* default volume control block */
    *defVCBPtrPtr = vVCBQ;
    defaultVolume = wdMagic;
    
    /* file I/O queue header */
    fsQHdrPtr->qFlags = 0;
    fsQHdrPtr->qHead = 0;
    fsQHdrPtr->qTail = 0;
    
    /* file control blocks */
    *fcbSPtrPtr = 0;
    
    *bootDrivePtr = 1;
    
    (void)trapWord; (void)regs;
}



Boolean InitMFS(void)
{
    UInt16 trapNum;
    
    for (trapNum = (_Open & 0xFF); trapNum <= (_GetFPos & 0xFF); ++trapNum) {
        osTrapTable[trapNum] = &trapFMRoutine;
    }
    
    osTrapTable[_HFSDispatch    & 0xFF]     = &trapHFSDispatch;
    osTrapTable[_InitFS         & 0xFF]     = &trapInitFs;
    
    return true;
}
