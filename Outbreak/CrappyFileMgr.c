
Boolean InitCrappyFiles(void);


#include "GrayBox.h"
#include "Patching.h"
#include "vMac.h"

#include <Traps.h>


enum {
    /* IM II-98 */
    asyncTrpMask = 1 << asyncTrpBit,
    hfsTrpMask = 1 << noQueueBit
};

enum {
    ioParam,
    fileParam,
    volumeParam,
    cntrlParam
};


typedef OSErr (*PBFuncPtr)(ParmBlkPtr paramBlock);


typedef struct OSTrap {
    PBFuncPtr sync, async;
    unsigned int paramBlkType;
} OSTrap;


/*
 * replacements for PBxxx routines missing from Carbon
 */

static OSErr PBMountVol(ParmBlkPtr paramBlock) {
    return paramBlock->ioParam.ioResult = unimpErr;
}


/*
 * PBGetVInfo
 */

static OSErr GetVolInfo(ParmBlkPtr paramBlock, Boolean async) {
    (void)async;
    return paramBlock->ioParam.ioResult = unimpErr;
}

static OSErr myPBGetVInfoSync(ParmBlkPtr paramBlock) {
    return GetVolInfo(paramBlock, false);
}

static OSErr PBGetVInfoAsync(ParmBlkPtr paramBlock) {
    return GetVolInfo(paramBlock, true);
}


/*
 * PBOpen, PBOpenRF
 * IM II pp. 108-109
 */

static OSErr Open(ParmBlkPtr paramBlock, Boolean async, int whichFork) {
    /*
         -> 12 ioCompletion  pointer 
        <Ñ  16 ioResult      word 
         Ñ> 18 ioNamePtr     pointer 
         Ñ> 22 ioVRefNum     word 
        <-  24 ioRefNum      word 
         Ñ> 26 ioVersNum     byte 
         Ñ> 27 ioPermssn     byte 
         -> 28 ioMisc        pointer 
    */
    
    StringPtr ioNamePtr;
    FSRefParam refParam;
    FSForkIOParam forkIOParam;
    HFSUniStr255 forkName;
    FSRef ref;
    OSErr err;
    
    ioNamePtr = paramBlock->ioParam.ioNamePtr;
    
    BlockZero(&refParam, sizeof(refParam));
    refParam.ioNamePtr = ioNamePtr;
    refParam.ioVRefNum = paramBlock->ioParam.ioVRefNum;
    refParam.ioDirID = paramBlock->ioParam.ioVRefNum; /* XXX: ??? */
    refParam.newRef = &ref;
    err = PBMakeFSRefSync(&refParam);
    if (err != noErr)
        goto leave;
        
    switch (whichFork) {
    case 0: default:
        err = FSGetDataForkName(&forkName);
        break;
    case 1:
        err = FSGetResourceForkName(&forkName);
        break;
    }
    if (err != noErr)
        goto leave;
    
    BlockZero(&forkIOParam, sizeof(forkIOParam));
    forkIOParam.ref = &ref;
    forkIOParam.forkNameLength = forkName.length;
    forkIOParam.forkName = forkName.unicode;
    forkIOParam.permissions = paramBlock->ioParam.ioPermssn;
    
    if (0 && async) {
        /* XXX: get async result into 'paramBlock' */
        PBOpenForkAsync(&forkIOParam);
        err = noErr;
    } else {
        err = PBOpenForkSync(&forkIOParam);
        paramBlock->ioParam.ioRefNum = forkIOParam.forkRefNum;
    }

 leave:
    paramBlock->ioParam.ioResult = err;
    return err;
}

static OSErr PBOpenSync(ParmBlkPtr paramBlock) {
    return Open(paramBlock, false, 0);
}

static OSErr PBOpenAsync(ParmBlkPtr paramBlock) {
    return Open(paramBlock, true, 0);
}

static OSErr PBOpenRFSync(ParmBlkPtr paramBlock) {
    return Open(paramBlock, false, 1);
}

static OSErr PBOpenRFAsync(ParmBlkPtr paramBlock) {
    return Open(paramBlock, true, 1);
}


/*
 * PBGetFInfo
 */

static OSErr GetFInfo(ParmBlkPtr paramBlock, Boolean async)
{
    /*
        -> 12 ioCompletion pointer 
        <Ñ 16 ioResult word 
        <-> 18 ioNamePtr pointer 
        Ñ> 22 ioVRefNum word 
        <Ñ 24 ioFRefNum word 
        -> 26 ioFVersNum byte 
        -> 28 ioFDirlndex word 
        <Ñ 30 ioFlAttrib byte 
        <Ñ 31 ioFlVersNum byte 
        <- 32 ioFlFndrlnfo 16 bytes 
        <Ñ 48 ioRNum long word 
        <Ñ 52 ioFlStBlk word 
        <Ñ 54 ioFlLgLen long word 
        <Ñ 58 ioFlPyLen long word 
        <Ñ 62 ioFlRStBlk word 
        <Ñ 64 ioHRLgLen long word 
        <Ñ 68 ioFlRPyLen long word 
        <Ñ 72 ioFlCrDat long word 
        <Ñ 76 ioHMdDat long word 
    */
    HParamBlockRec hParamBlock;
    OSErr err;
    
    BlockZero(&hParamBlock, sizeof(hParamBlock));
    
    hParamBlock.fileParam.ioNamePtr = paramBlock->fileParam.ioNamePtr;
    hParamBlock.fileParam.ioVRefNum = paramBlock->fileParam.ioVRefNum;
    hParamBlock.fileParam.ioFVersNum = paramBlock->fileParam.ioFVersNum;
    hParamBlock.fileParam.ioFDirIndex = paramBlock->fileParam.ioFDirIndex;
    
    if (0 && async) {
        err = PBHGetFInfoAsync(&hParamBlock);
    } else {
        err = PBHGetFInfoSync(&hParamBlock);
    }
    if (err != noErr)
        goto leave;
    
    paramBlock->fileParam.ioFRefNum = hParamBlock.fileParam.ioFRefNum;
    paramBlock->fileParam.ioFlAttrib = hParamBlock.fileParam.ioFlAttrib;
    paramBlock->fileParam.ioFlVersNum = hParamBlock.fileParam.ioFlVersNum;
    paramBlock->fileParam.ioFlFndrInfo = hParamBlock.fileParam.ioFlFndrInfo;
    paramBlock->fileParam.ioFlNum = 0 /*??? hParamBlock.fileParam.ioFlNum*/;
    paramBlock->fileParam.ioFlStBlk = hParamBlock.fileParam.ioFlStBlk;
    paramBlock->fileParam.ioFlLgLen = hParamBlock.fileParam.ioFlLgLen;
    paramBlock->fileParam.ioFlPyLen = hParamBlock.fileParam.ioFlPyLen;
    paramBlock->fileParam.ioFlRStBlk = hParamBlock.fileParam.ioFlRStBlk;
    paramBlock->fileParam.ioFlLgLen = hParamBlock.fileParam.ioFlLgLen;
    paramBlock->fileParam.ioFlRPyLen = hParamBlock.fileParam.ioFlRPyLen;
    paramBlock->fileParam.ioFlCrDat = hParamBlock.fileParam.ioFlCrDat;
    paramBlock->fileParam.ioFlMdDat = hParamBlock.fileParam.ioFlMdDat;
    
 leave:
    paramBlock->fileParam.ioResult = err;
    return err;
}

static OSErr PBGetFInfoSync(ParmBlkPtr paramBlock) {
    return GetFInfo(paramBlock, false);
}

static OSErr PBGetFInfoAsync(ParmBlkPtr paramBlock) {
    return GetFInfo(paramBlock, true);
}


/*
 * PBGetVol
 */

static OSErr PBGetVolSync(ParmBlkPtr paramBlock) {
    paramBlock->ioParam.ioVRefNum = 666;
    return paramBlock->ioParam.ioResult = noErr;
}

static OSErr PBGetVolAsync(ParmBlkPtr paramBlock) {
    paramBlock->ioParam.ioVRefNum = 666;
    return paramBlock->ioParam.ioResult = noErr;
}


/*
 * PBGetVol
 */

static OSErr PBSetVolSync(ParmBlkPtr paramBlock) {
    return paramBlock->ioParam.ioResult = noErr;
}

static OSErr PBSetVolAsync(ParmBlkPtr paramBlock) {
    return paramBlock->ioParam.ioResult = noErr;
}


/*
 * PBEject
 */

static OSErr PBEject(ParmBlkPtr paramBlock) {
    return paramBlock->ioParam.ioResult = noErr;
}


/*
 * misc.
 */

static void trapInitFs(UInt16 trapWord, UInt32 regs[16]) {
    SInt16 *FsFcbLenPtr =  (SInt16 *)(RAM + 0x3F6);
    *FsFcbLenPtr = 1; /* HFS present */
    (void)trapWord; (void)regs;
}

static OSErr NoOp(ParmBlkPtr paramBlock) {
    return paramBlock->ioParam.ioResult = noErr;
}

static OSErr Unimp(ParmBlkPtr paramBlock) {
    return paramBlock->ioParam.ioResult = unimpErr;
}



static OSTrap osTrap[] = {
    { &PBOpenSync,       &PBOpenAsync,       ioParam     }, /* not in Carbon */
    { &PBCloseSync,      &PBCloseAsync,      ioParam     },
    { &PBReadSync,       &PBReadAsync,       ioParam     },
    { &PBWriteSync,      &PBWriteAsync,      ioParam     },
    { &Unimp,            &Unimp,             cntrlParam  }, /* not in Carbon */
    { &Unimp,            &Unimp,             ioParam     }, /* not in Carbon */
    { &Unimp,            &Unimp,             ioParam     }, /* not in Carbon */
    { &myPBGetVInfoSync, &PBGetVInfoAsync,   volumeParam }, /* not in Carbon */
    { &Unimp,            &Unimp,             fileParam   }, /* not in Carbon */
    { &Unimp,            &Unimp,             fileParam   }, /* not in Carbon */
    { &PBOpenRFSync,     &PBOpenRFAsync,     ioParam     }, /* not in Carbon */
    { &Unimp,            &Unimp,             fileParam   }, /* not in Carbon */
    { &PBGetFInfoSync,   &PBGetFInfoAsync,   ioParam     }, /* not in Carbon */
    { &Unimp,            &Unimp,             fileParam   }, /* not in Carbon */
    { &PBUnmountVol,     0,                  volumeParam },
    { &PBMountVol,       0,                  volumeParam }, /* not in Carbon */
    { &PBAllocateSync,   &PBAllocateAsync,   ioParam     },
    { &PBGetEOFSync,     &PBGetEOFAsync,     ioParam     },
    { &PBSetEOFSync,     &PBSetEOFAsync,     ioParam     },
    { &PBFlushVolSync,   &PBFlushVolAsync,   ioParam     },
    { &PBGetVolSync,     &PBGetVolAsync,     volumeParam }, /* not in Carbon */
    { &PBSetVolSync,     &PBSetVolAsync,     volumeParam }, /* not in Carbon */
    { &Unimp,            &Unimp,             ioParam     }, /* not in Carbon */
    { &PBEject,          0,                  volumeParam }, /* not in Carbon */
    { &PBGetFPosSync,    &PBGetFPosAsync,    ioParam     },
};



/*
 * OSDispatch
 */

static void trapFileManager(UInt16 trapWord, UInt32 regs[16]) {
    OSTrap *trap;
    
    ParmBlkPtr paramBlock;
    IOCompletionUPP ioCompletion;
    StringPtr ioNamePtr;
    Ptr ioMisc, ioBuffer;
    
    OSErr err;
    
    trap = &osTrap[trapWord & 0xff];
    paramBlock = (ParmBlkPtr)get_real_address(regs[8+0]);
    
    
    /*
     * perform address tranlation
     */
    
    ioCompletion = paramBlock->ioParam.ioCompletion;
    if (ioCompletion && ioCompletion != (IOCompletionUPP)-1) {
        paramBlock->ioParam.ioCompletion
            = (IOCompletionUPP)get_real_address((CPTR)ioCompletion);
    }
    
    ioNamePtr = paramBlock->ioParam.ioNamePtr;
    if (ioNamePtr && ioNamePtr != (StringPtr)-1) {
        paramBlock->ioParam.ioNamePtr
            = (StringPtr)get_real_address((CPTR)ioNamePtr);
    }
    
    switch (trap->paramBlkType) {
    case ioParam:
        switch (trapWord) {
        case _Rename: /* new name */
        case _Rename | asyncTrpMask:
        case _Open: /* optional ptr to buffer */
        case _Open | asyncTrpMask:
            ioMisc = paramBlock->ioParam.ioMisc;
            if (ioMisc && ioMisc != (Ptr)-1) {
                paramBlock->ioParam.ioMisc
                    = (Ptr)get_real_address((CPTR)ioMisc);
            }
            break;
        }
        ioBuffer = paramBlock->ioParam.ioBuffer;
        if (ioBuffer && ioBuffer != (Ptr)-1) {
            paramBlock->ioParam.ioBuffer
                = (Ptr)get_real_address((CPTR)ioBuffer);
        }
        break;
    case fileParam:
    case volumeParam:
    case cntrlParam:
        break;
    }
    
    
    /* call */
    err = (*((trapWord & asyncTrpMask) ? trap->async : trap->sync))(paramBlock);
    
    
    /*
     * restore virtual addresses
     */
    
    paramBlock->ioParam.ioCompletion = ioCompletion;
    paramBlock->ioParam.ioNamePtr = ioNamePtr;
    
    switch (trap->paramBlkType) {
    case ioParam:
        switch (trapWord) {
        case _Rename: /* new name */
        case _Rename | asyncTrpMask:
        case _Open: /* optional ptr to buffer */
        case _Open | asyncTrpMask:
            paramBlock->ioParam.ioMisc = ioMisc;
            break;
        }
        paramBlock->ioParam.ioBuffer = ioBuffer;
        break;
    case fileParam:
    case volumeParam:
    case cntrlParam:
        break;
    }
    
    regs[0] = err;
	m68k_test_d0();
}


static void trapHFSDispatch(UInt16 trapWord, UInt32 regs[16]) {
    OSErr err;
    
    switch (regs[0]) {
    
    case 0x0001: { /* PBOpenWDSync */
        static short wd = -32768;
        WDPBPtr paramBlock = (WDPBPtr)get_real_address(regs[8+0]);
        paramBlock->ioVRefNum = ++wd;
        err = noErr;
        break; }
    
    case 0x0008: { /* PBGetFCBInfoSync */
        FCBPBPtr pb = (FCBPBPtr)get_real_address(regs[8+0]);
        StringPtr ioNamePtr = pb->ioNamePtr;
        if (pb->ioNamePtr) {
            pb->ioNamePtr = (StringPtr)get_real_address((CPTR)ioNamePtr);
        }
        err = PBGetFCBInfoSync(pb);
        pb->ioNamePtr = ioNamePtr;
        break; }
    
    case 0x0009: { /* PBGetCatInfoSync */
        CInfoPBPtr paramBlock = (CInfoPBPtr)get_real_address(regs[8+0]);
        StringPtr ioNamePtr = paramBlock->hFileInfo.ioNamePtr;
        if (ioNamePtr) {
            paramBlock->hFileInfo.ioNamePtr
                = (StringPtr)get_real_address((CPTR)ioNamePtr);
        }
        err = PBGetCatInfoSync(paramBlock);
        paramBlock->hFileInfo.ioNamePtr = ioNamePtr;
        break; }
    }

    regs[0] = err;
	m68k_test_d0();
	(void)trapWord;
}


Boolean InitCrappyFiles(void)
{
    UInt16 trapNum;
    
    for (trapNum = (_Open & 0xFF); trapNum <= (_GetFPos & 0xFF); ++trapNum) {
        osTrapTable[trapNum] = &trapFileManager;
    }
    
    osTrapTable[_HFSDispatch    & 0xFF]     = &trapHFSDispatch;
    osTrapTable[_InitFS         & 0xFF]     = &trapInitFs;
    
    /* This code is so crappy, we need to do this. */
    HSetVol(NULL, bootVRefNum, sysDirID);
    
    return true;
}
