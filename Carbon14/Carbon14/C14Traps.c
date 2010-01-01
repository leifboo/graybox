
#include "C14Traps.h"

#include "C14Dialogs.h"
#include "C14Events.h"
#include "C14Fonts.h"
#include "C14M68K.h"
#include "C14Memory.h"
#include "C14Menus.h"
#include "C14MixedMode.h"
#include "C14OSUtils.h"
#include "C14Quickdraw.h"
#include "C14SegLoad.h"
#include "C14TextEdit.h"
#include "C14Windows.h"

#include <stdio.h>
#include <string.h>


static const char *osTrapName[0x100] = {
    "Open",
    "Close",
    "Read",
    "Write",
    "Control",
    "Status",
    "KillIO",
    "GetVolInfo",
    "Create",
    "Delete",
    "OpenRF",
    "ReName",
    "GetFileInfo",
    "SetFileInfo",
    "UnMountVol",
    "MountVol",
    "Allocate",
    "GetEOF",
    "SetEOF",
    "FlushVol",
    "GetVol",
    "SetVol",
    "FInitQueue",
    "Eject",
    "GetFPos",
    "InitZone",
    "GetZone",
    "SetZone",
    "FreeMem",
    "MaxMem",
    "NewPtr",
    "DisposPtr",
    "SetPtrSize",
    "GetPtrSize",
    "NewHandle",
    "DisposHandle",
    "SetHandleSize",
    "GetHandleSize",
    "HandleZone",
    "ReAllocHandle",
    "RecoverHandle",
    "HLock",
    "HUnLock",
    "EmptyHandle",
    "InitApplZone",
    "SetApplLimit",
    "BlockMove",
    "PostEvent",
    "OSEventAvail",
    "GetOSEvent",
    "FlushEvents",
    "VInstall",
    "VRemove",
    "OffLine",
    "MoreMasters",
    "ReadParam",
    "WriteParam",
    "ReadDateTime",
    "SetDateTime",
    "Delay",
    "CmpString",
    "DrvrInstall",
    "DrvrRemove",
    "InitUtil",
    "ResrvMem",
    "SetFilLock",
    "RstFilLock",
    "SetFilType",
    "SetFPos",
    "FlushFile",
    "GetTrapAddress",
    "SetTrapAddress",
    "PtrZone",
    "HPurge",
    "HNoPurge",
    "SetGrowZone",
    "CompactMem",
    "PurgeMem",
    "AddDrive",
    "RDrvrInstall",
    "RelString",
    "ReadXPRam",
    "WriteXPRam",
    "ClkNoMem",
    "UprString",
    "StripAddress",
    "LwrString",
    "SetAppBase",
    "InsTime",
    "RmvTime",
    "PrimeTime",
    "PowerOFF",
    "VM_Dispatch",
    "SwapMMUMode",
    "NMInstall",
    "NMRemove",
    "HFsDispatch",
    "MaxBlock",
    "PurgeSpace",
    "MaxApplZone",
    "MoveHHi",
    "StackSpace",
    "NewEmptyHandle",
    "HSetRBit",
    "HClrRBit",
    "HGetState",
    "HSetState",
    "SI_TestMgr",
    "InitFs",
    "InitEvents",
    "SlotManager",
    "SlotVInstall",
    "SlotVRemove",
    "AttachVBL",
    "DoVBLTask",
    "Reserved4",
    "CacheMgr",
    "SIntInstall",
    "SIntRemove",
    "CountADBs",
    "GetIndADB",
    "GetADBInfo",
    "SetADBInfo",
    "ADBReInit",
    "ADBOp",
    "GetDefaultStartup",
    "SetDefaultStartup",
    "InternalWait",
    "GetVideoDefault",
    "SetVideoDefault",
    "DTInstall",
    "SetOSDefault",
    "GetOSDefault",
    "PowerMgr",
    "IOPInfoAccess",
    "IOPMsgRequest",
    "IOPMoveData",
    "SCSIAtomic",
    "Sleep",
    "CommToolboxDispatch",
    "Wakeup",
    "DebugUtil",
    "BTreeDispatch",
    "DeferUserFn",
    "SysEnvirons",
    "Xlate24to32",
    "EgretDispatch",
    "MicroTickCount",
    "ServerDispatch",
    "Myst_95",
    "Myst_96",
    "FPPriv",
    "HWPriv",
    "XToolTable",
    "vProcHelper",
    "Messager",
    "NewPtrStartup",
    "MoveHLow",
    "PowerDispatchPublic",
    "Power",
    "Myst_A0",
    "Myst_A1",
    "Myst_A2",
    "Myst_A3",
    "HeapDispatch",
    "VisRegionChanged",
    "Myst_A6",
    "Myst_A7",
    "Myst_A8",
    "Myst_A9",
    "Myst_AA",
    "Myst_AB",
    "FileSysMgr",
    "Gestalt",
    "Myst_AE",
    "Myst_AF",
    "Myst_B0",
    "Myst_B1",
    "Myst_B2",
    "Myst_B3",
    "Myst_B4",
    "jGoDriver",
    "jWaitUntil",
    "jSyncWait",
    "jSoundDead",
    "jDisptch",
    "jIAZInit",
    "jIAZPostInit",
    "jLaunchInit",
    "jCacheFlush",
    "jSysUtil",
    "jLg2Phys",
    "jFlushCache",
    "jGetBlock",
    "jMarkBlock",
    "jRelBlock",
    "jTrashBlocks",
    "jTrashVBlks",
    "jCacheWrIP",
    "jCacheRdIP",
    "jBasicIO",
    "jRdBlocks",
    "jWrBlocks",
    "jSetUpTags",
    "jBTClose",
    "jBTDelete",
    "jBTFlush",
    "jBTGetRecord",
    "jBTInsert",
    "jBTOpen",
    "jBTSearch",
    "jBTUpdate",
    "jGetNode",
    "jRelNode",
    "jAllocNode",
    "jFreeNode",
    "jExtBTFile",
    "jDeallocFile",
    "jExtendFile",
    "jTruncateFile",
    "jCMSetUp",
    "PPCToolBoxMgr",
    "jDtrmV1",
    "jBlkAlloc",
    "jBlkDeAlloc",
    "jFileOpen",
    "jPermssnChk",
    "jFndFilName",
    "jRfNCall",
    "jAdjEOF",
    "jPixel2Char",
    "jChar2Pixel",
    "jHiliteText",
    "jFileClose",
    "jFileRead",
    "jFileWrite",
    "jDispatchHelper",
    "jUpdAldMDB",
    "jCkExtFs",
    "jDTrmV3",
    "jBMChk",
    "jTstMod",
    "jLocCRec",
    "jTreeSearch",
    "jMapFBlock",
    "jXFSearch",
    "jReadBM",
    "jDoEject",
    "jSegStack",
    "jSuperLoad",
    "jCmpFrm",
    "jNewMap",
    "jCheckLoad",
    "TETrimMeasure",
    "TEFindWord",
    "TEFindLine"
};


static const char *tbTrapName[0x400] = {
    "SoundDispatch",
    "SndDisposeChannel",
    "SndAddModifier",
    "SndDoCommand",
    "SndDoImmediate",
    "SndPlay",
    "SndControl",
    "SndNewChannel",
    "InitProcMenu",
    "GetCVariant",
    "GetWVariant",
    "PopUpMenuSelect",
    "RGetResource",
    "Count1Resources",
    "Get1IxResource",
    "Get1IxType",
    "Unique1ID",
    "TeSelView",
    "TePinScroll",
    "TeAutoView",
    "SetFractEnable",
    "SCSiDispatch",
    "Pack8",
    "CopyMask",
    "FixAtan2",
    "XMunger",
    "HOpenResFile",
    "HCreateResFile",
    "Count1Types",
    "InvalMenuBar",
    "SaveRestoreBits",
    "Get1Resource",
    "Get1NamedResource",
    "MaxSizeRsrc",
    "RsrcMgr",
    "AliasMgr",
    "HFSUtils",
    "MenuDispatch",
    "InsMenuItem",
    "HideD_Item",
    "ShowD_Item",
    "LayerDispatch",
    "ComponentDispatch",
    "PPCBrowser",
    "Pack10",
    "DataPubMgr",
    "Pack12",
    "DatBaseMgr",
    "Help_Pack",
    "Pack15",
    "QD_GX_Mgr",
    "ScrnBitMap",
    "SetFScaleDisable",
    "FontMetrics",
    "GetMaskTable",
    "MeasureText",
    "CalcMask",
    "SeedFill",
    "ZoomWindow",
    "TrackBox",
    "TEGetOffset",
    "TEDispatch",
    "TEStyleNew",
    "Long2Fix",
    "Fix2Long",
    "Fix2Frac",
    "Fract2Fix",
    "Fix2X",
    "X2Fix",
    "Frac2X",
    "X2Frac",
    "FracCos",
    "FracSin",
    "FracSqrt",
    "FracMul",
    "FracDiv",
    "ScrollDelay",
    "FixDiv",
    "GetItmCmdkey",
    "SetItmCmdkey",
    "InitCursor",
    "SetCursor",
    "HideCursor",
    "ShowCursor",
    "FontDispatch",
    "ShieldCursor",
    "ObscureCursor",
    "Myst_857",
    "BitAnd",
    "BitXOr",
    "BitNot",
    "BitOr",
    "BitShift",
    "BitTst",
    "BitSet",
    "BitClr",
    "WaitNextEvent",
    "Random",
    "ForeColor",
    "BackColor",
    "ColorBit",
    "GetPixel",
    "StuffHex",
    "LongMul",
    "FixMul",
    "FixRatio",
    "HiWord",
    "LoWord",
    "FixRound",
    "InitPort",
    "InitGraf",
    "OpenPort",
    "LocalToGlobal",
    "GlobalToLocal",
    "GrafDevice",
    "SetPort",
    "GetPort",
    "SetPBits",
    "PortSize",
    "MovePortTo",
    "SetOrigin",
    "SetClip",
    "GetClip",
    "ClipRect",
    "BackPat",
    "ClosePort",
    "AddPt",
    "SubPt",
    "SetPt",
    "EqualPt",
    "StdText",
    "DrawChar",
    "DrawString",
    "DrawText",
    "TextWidth",
    "TextFont",
    "TextFace",
    "TextMode",
    "TextSize",
    "GetFontInfo",
    "StringWidth",
    "CharWidth",
    "SpaceExtra",
    "OSDispatch",
    "StdLine",
    "LineTo",
    "Line",
    "MoveTo",
    "Move",
    "ShutDown",
    "HidePen",
    "ShowPen",
    "GetPenState",
    "SetPenState",
    "GetPen",
    "PenSize",
    "PenMode",
    "PenPat",
    "PenNormal",
    "UnimplTrap",
    "StdRect",
    "FrameRect",
    "PaintRect",
    "EraseRect",
    "InverRect",
    "FillRect",
    "EqualRect",
    "SetRect",
    "OffSetRect",
    "InSetRect",
    "SectRect",
    "UnionRect",
    "Pt2Rect",
    "PtInRect",
    "EmptyRect",
    "StdRRect",
    "FrameRoundRect",
    "PaintRoundRect",
    "EraseRoundRect",
    "InverRoundRect",
    "FillRoundRect",
    "ScriptUtil",
    "StdOval",
    "FrameOval",
    "PaintOval",
    "EraseOval",
    "InvertOval",
    "FillOval",
    "SlopeFromAngle",
    "StdArc",
    "FrameArc",
    "PaintArc",
    "EraseArc",
    "InvertArc",
    "FillArc",
    "PtToAngle",
    "AngleFromSlope",
    "StdPoly",
    "FramePoly",
    "PaintPoly",
    "ErasePoly",
    "InvertPoly",
    "FillPoly",
    "OpenPoly",
    "ClosePgon",
    "KillPoly",
    "OffSetPoly",
    "PackBits",
    "UnpackBits",
    "StdRgn",
    "FrameRgn",
    "PaintRgn",
    "EraseRgn",
    "InverRgn",
    "FillRgn",
    "BitmapToRegion",
    "NewRgn",
    "DisposRgn",
    "OpenRgn",
    "CloseRgn",
    "CopyRgn",
    "SetEmptyRgn",
    "SetRecRgn",
    "RectRgn",
    "OfSetRgn",
    "InSetRgn",
    "EmptyRgn",
    "EqualRgn",
    "SectRgn",
    "UnionRgn",
    "DiffRgn",
    "XOrRgn",
    "PtInRgn",
    "RectInRgn",
    "SetStdProcs",
    "StdBits",
    "CopyBits",
    "StdTxMeas",
    "StdGetPic",
    "ScrollRect",
    "StdPutPic",
    "StdComment",
    "PicComment",
    "OpenPicture",
    "ClosePicture",
    "KillPicture",
    "DrawPicture",
    "Reserved11",
    "ScalePt",
    "MapPt",
    "MapRect",
    "MapRgn",
    "MapPoly",
    "PrintMgr",
    "InitFonts",
    "GetFName",
    "GetFNum",
    "FMSwapFont",
    "RealFont",
    "SetFontLock",
    "DrawGrowIcon",
    "DragGrayRgn",
    "NewString",
    "SetString",
    "ShowHide",
    "CalcVis",
    "CalcVBehind",
    "ClipAbove",
    "PaintOne",
    "PaintBehind",
    "SaveOld",
    "DrawNew",
    "GetWMgrPort",
    "CheckUpDate",
    "InitWindows",
    "NewWindow",
    "DisposWindow",
    "ShowWindow",
    "HideWindow",
    "GetWRefCon",
    "SetWRefCon",
    "GetWTitle",
    "SetWTitle",
    "MoveWindow",
    "HiliteWindow",
    "SizeWindow",
    "TrackGoAway",
    "SelectWindow",
    "BringToFront",
    "SendBehind",
    "BeginUpdate",
    "EndUpdate",
    "FrontWindow",
    "DragWindow",
    "DragTheRgn",
    "InvalRgn",
    "InvalRect",
    "ValidRgn",
    "ValidRect",
    "GrowWindow",
    "FindWindow",
    "CloseWindow",
    "SetWindowPic",
    "GetWindowPic",
    "InitMenus",
    "NewMenu",
    "DisposMenu",
    "AppendMenu",
    "ClearMenuBar",
    "InsertMenu",
    "DeleteMenu",
    "DrawMenuBar",
    "HiliteMenu",
    "EnableItem",
    "DisableItem",
    "GetMenuBar",
    "SetMenuBar",
    "MenuSelect",
    "MenuKey",
    "GetItemIcon",
    "SetItemIcon",
    "GetItemStyle",
    "SetItemStyle",
    "GetItemMark",
    "SetItemMark",
    "CheckItem",
    "GetItem",
    "SetItem",
    "CalcMenuSize",
    "GetMHandle",
    "SetMFlash",
    "PlotIcon",
    "FlashMenuBar",
    "AddResMenu",
    "PinRect",
    "DeltaPoint",
    "CountMItems",
    "InsertResMenu",
    "DelMenuItem",
    "UpdateControl",
    "NewControl",
    "DisposControl",
    "KillControls",
    "ShowControl",
    "HideControl",
    "MoveControl",
    "GetCRefCon",
    "SetCRefCon",
    "SizeControl",
    "HiliteControl",
    "GetCTitle",
    "SetCTitle",
    "GetCtlValue",
    "GetMinCtl",
    "GetMaxCtl",
    "SetCtlValue",
    "SetMinCtl",
    "SetMaxCtl",
    "TestControl",
    "DragControl",
    "TrackControl",
    "DrawControls",
    "GetCtlAction",
    "SetCtlAction",
    "FindControl",
    "Draw1Control",
    "DeQueue",
    "EnQueue",
    "GetNextEvent",
    "EventAvail",
    "GetMouse",
    "StillDown",
    "Button",
    "TickCount",
    "GetKeys",
    "WaitMouseUp",
    "UpdtDialog",
    "CouldDialog",
    "FreeDialog",
    "InitDialogs",
    "GetNewDialog",
    "NewDialog",
    "SelIText",
    "IsDialogEvent",
    "DialogSelect",
    "DrawDialog",
    "CloseDialog",
    "DisposDialog",
    "FindD_Item",
    "Alert",
    "StopAlert",
    "NoteAlert",
    "CautionAlert",
    "CouldAlert",
    "FreeAlert",
    "ParamText",
    "ErrorSound",
    "GetDItem",
    "SetDItem",
    "SetIText",
    "GetIText",
    "ModalDialog",
    "DetachResource",
    "SetResPurge",
    "CurResFile",
    "InitResources",
    "RsrcZoneInit",
    "OpenResFile",
    "UseResFile",
    "UpdateResFile",
    "CloseResFile",
    "SetResLoad",
    "CountResources",
    "GetIndResource",
    "CountTypes",
    "GetIndType",
    "GetResource",
    "GetNamedResource",
    "LoadResource",
    "ReleaseResource",
    "HomeResFile",
    "SizeRsrc",
    "GetResAttrs",
    "SetResAttrs",
    "GetResInfo",
    "SetResInfo",
    "ChangedResource",
    "AddResource",
    "AddReference",
    "RmveResource",
    "RmveReference",
    "ResError",
    "WriteResource",
    "CreateResFile",
    "SystemEvent",
    "SystemClick",
    "SystemTask",
    "SystemMenu",
    "OpenDeskAcc",
    "CloseDeskAcc",
    "GetPattern",
    "GetCursor",
    "GetString",
    "GetIcon",
    "GetPicture",
    "GetNewWindow",
    "GetNewControl",
    "GetMenu",
    "GetNewMBar",
    "UniqueID",
    "SysEdit",
    "KeyTrans",
    "OpenRFPerm",
    "RsrcMapEntry",
    "Secs2Date",
    "Date2Secs",
    "SysBeep",
    "SysError",
    "SI_PutIcon",
    "TeGetText",
    "TeInit",
    "TeDispose",
    "TextBox",
    "TeSetText",
    "TeCalText",
    "TeSetSelect",
    "TeNew",
    "TeUpdate",
    "TeClick",
    "TeCopy",
    "TeCut",
    "TeDelete",
    "TeActivate",
    "TeDeactivate",
    "TeIdle",
    "TePaste",
    "TeKey",
    "TeScroll",
    "TeInsert",
    "TeSetJust",
    "Munger",
    "HandToHand",
    "PtrToXHand",
    "PtrToHand",
    "HandAndHand",
    "InitPack",
    "InitAllPacks",
    "Pack0",
    "Pack1",
    "Pack2",
    "Pack3",
    "Pack4",
    "Pack5",
    "Pack6",
    "Pack7",
    "PtrAndHand",
    "LoadSeg",
    "UnLoadSeg",
    "Launch",
    "Chain",
    "ExitToShell",
    "GetAppParms",
    "GetResFileAttrs",
    "SetResFileAttrs",
    "MethodDispatch",
    "InfoScrap",
    "UnlodeScrap",
    "LodeScrap",
    "ZeroScrap",
    "GetScrap",
    "PutScrap",
    "Debugger",
    "XOpenCPort",
    "InitCPort",
    "CloseCPort",
    "NewPixMap",
    "DisposPixMap",
    "CopyPixMap",
    "SetPortPix",
    "NewPixPat",
    "DisposPixPat",
    "CopyPixPat",
    "PenPixPat",
    "BackPixPat",
    "GetPixPat",
    "MakeRGBPat",
    "FillCRect",
    "FillCOval",
    "FillCRoundRect",
    "FillCArc",
    "FillCRgn",
    "FillCPoly",
    "RGBForeColor",
    "RGBBackColor",
    "SetCPixel",
    "GetCPixel",
    "GetCTable",
    "GetForeColor",
    "GetBackColor",
    "GetCCursor",
    "SetCCursor",
    "AllocCursor",
    "GetCIcon",
    "PlotCIcon",
    "OpenCPicture",
    "OpColor",
    "HiliteColor",
    "CharExtra",
    "DisposCTable",
    "DisposCIcon",
    "DisposCCursor",
    "GetMaxDevice",
    "GetCTSeed",
    "GetDeviceList",
    "GetMainDevice",
    "GetNextDevice",
    "TestDeviceAttribute",
    "SetDeviceAttribute",
    "InitGDevice",
    "NewGDevice",
    "DisposGDevice",
    "SetGDevice",
    "GetGDevice",
    "Color2Index",
    "Index2Color",
    "InvertColor",
    "RealColor",
    "GetSubTable",
    "UpdatePixMap",
    "MakeITable",
    "AddSearch",
    "AddComp",
    "SetClientID",
    "ProtectEntry",
    "ReserveEntry",
    "SetEntries",
    "QDError",
    "SetWinColor",
    "GetAuxWin",
    "SetCtlColor",
    "GetAuxCtl",
    "NewCWindow",
    "GetNewCWindow",
    "SetDeskCPat",
    "GetCWMgrPort",
    "SaveEntries",
    "RestoreEntries",
    "NewCDialog",
    "DelSearch",
    "DelComp",
    "SetStdCProcs",
    "CalcCMask",
    "SeedCFill",
    "CopyDeepMask",
    "HighLvlFSDispatch",
    "DictionaryDispatch",
    "TextServDispatch",
    "KobeDispatch",
    "PlainTalk",
    "DockingDispatch",
    "NuKernelDispatch",
    "MixedModeDispatch",
    "CodeFragDispatch",
    "RemoteAccess",
    "OCE_DigSig",
    "OCE_LetterPacking",
    "OCE_MailMessage",
    "OCE_Authentication",
    "DelMCEntries",
    "GetMCInfo",
    "SetMCInfo",
    "DispMCInfo",
    "GetMCEntry",
    "SetMCEntries",
    "MenuChoice",
    "ModalDialogMenuSetup",
    "DialogDispatch",
    "UserNameNotification",
    "DeviceDispatch",
    "PowerPCFuture",
    "PenMacDispatch",
    "LanguageMgrDispatch",
    "AppleGuideDispatch",
    "Myst_A6F",
    "Myst_A70",
    "Myst_A71",
    "Myst_A72",
    "ControlDispatch",
    "AppearanceDispatch",
    "IconServicesDispatch",
    "Myst_A76",
    "Myst_A77",
    "Myst_A78",
    "Myst_A79",
    "Myst_A7A",
    "Myst_A7B",
    "Myst_A7C",
    "Myst_A7D",
    "Myst_A7E",
    "Myst_A7F",
    "Myst_A80",
    "Myst_A81",
    "Myst_A82",
    "Myst_A83",
    "Myst_A84",
    "Myst_A85",
    "Myst_A86",
    "Myst_A87",
    "Myst_A88",
    "Myst_A89",
    "Myst_A8A",
    "Myst_A8B",
    "Myst_A8C",
    "Myst_A8D",
    "Myst_A8E",
    "Myst_A8F",
    "InitPalettes",
    "NewPalette",
    "GetNewPalette",
    "DisposPalette",
    "ActivatePalette",
    "SetPalette",
    "GetPalette",
    "PmForeColor",
    "PmBackColor",
    "AnimateEntry",
    "AnimatePalette",
    "GetEntryColor",
    "SetEntryColor",
    "GetEntryUsage",
    "SetEntryUsage",
    "CTab2Palette",
    "Palette2CTab",
    "CopyPalette",
    "PaletteDispatch",
    "CodecDispatch",
    "Myst_AA4",
    "Myst_AA5",
    "Myst_AA6",
    "Myst_AA7",
    "Myst_AA8",
    "Myst_AA9",
    "QTDispatch",
    "Myst_AAB",
    "Myst_AAC",
    "Myst_AAD",
    "Myst_AAE",
    "Myst_AAF",
    "Myst_AB0",
    "Myst_AB1",
    "Myst_AB2",
    "Myst_AB3",
    "Myst_AB4",
    "Myst_AB5",
    "Myst_AB6",
    "Myst_AB7",
    "Myst_AB8",
    "Myst_AB9",
    "Myst_ABA",
    "Myst_ABB",
    "Myst_ABC",
    "Myst_ABD",
    "Myst_ABE",
    "Myst_ABF",
    "Myst_AC0",
    "Myst_AC1",
    "Myst_AC2",
    "Myst_AC3",
    "Myst_AC4",
    "Myst_AC5",
    "Myst_AC6",
    "Myst_AC7",
    "Myst_AC8",
    "Myst_AC9",
    "Myst_ACA",
    "Myst_ACB",
    "Myst_ACC",
    "Myst_ACD",
    "Myst_ACE",
    "Myst_ACF",
    "Myst_AD0",
    "Myst_AD1",
    "Myst_AD2",
    "Myst_AD3",
    "Myst_AD4",
    "Myst_AD5",
    "Myst_AD6",
    "Myst_AD7",
    "Myst_AD8",
    "Myst_AD9",
    "Myst_ADA",
    "CursorADBDispatch",
    "Myst_ADC",
    "HumanInterfaceUtils",
    "Myst_ADE",
    "Myst_ADF",
    "Myst_AE0",
    "Myst_AE1",
    "Myst_AE2",
    "Myst_AE3",
    "Myst_AE4",
    "Myst_AE5",
    "Myst_AE6",
    "Myst_AE7",
    "Myst_AE8",
    "Myst_AE9",
    "Myst_AEA",
    "Myst_AEB",
    "Myst_AEC",
    "Myst_AED",
    "AppleScript",
    "Myst_AEF",
    "Myst_AF0",
    "ATAManager",
    "ControlStripMgr",
    "ExpansionBusDispatch",
    "InterruptMgrDispatch",
    "Myst_AF5",
    "Myst_AF6",
    "Myst_AF7",
    "Myst_AF8",
    "Myst_AF9",
    "InitApplication",
    "CleanupApplication",
    "Myst_AFC",
    "Myst_AFD",
    "MixedModeMagic",
    "Myst_AFF",
    "qd_BitBlt",
    "qd_BitsToMap",
    "qd_BitsToPix",
    "QD32Trap",
    "qd_ColorMap",
    "qd_CopyHandle",
    "qd_CullPoints",
    "qd_DputpicByte",
    "qd_DputPicop",
    "qd_DrawArc",
    "qd_DrawLine",
    "qd_DrawSlab",
    "qd_FastSlabMode",
    "qd_GetSeek",
    "qd_MakeScaleTbl",
    "qd_Checkpic",
    "qd_Line",
    "qd_OldPatToNew",
    "qd_PackRgn",
    "qd_PatConvert",
    "qd_PatDither",
    "qd_PatExpand",
    "qd_Pinit",
    "qd_PortToMap",
    "qd_PushVerb",
    "qd_PutLine",
    "qd_PutOval",
    "qd_PutRgn",
    "NewTempBuffer",
    "QDOffscreen",
    "DisposeTempBuffer",
    "qd_RgnBlt",
    "qd_RgnOp",
    "qd_Rsect",
    "qd_SeekRgn",
    "qd_SetFillPat",
    "qd_SetupStretch",
    "qd_SlabMode",
    "qd_SortPoints",
    "qd_StretchBits",
    "qd_StdDevLoop",
    "qd_TrimRect",
    "qd_XorSlab",
    "Myst_B2B",
    "Myst_B2C",
    "NewTempHandle",
    "Myst_B2E",
    "Myst_B2F",
    "qd_BMain0",
    "qd_BMain1",
    "qd_BMain2",
    "qd_BMain3",
    "qd_Bsetup8",
    "qd_BMain9",
    "qd_Bsetup10",
    "qd_BMain11",
    "qd_BXMain8",
    "qd_BXMain9",
    "qd_BXMain10",
    "qd_Bxmain11",
    "qd_BCMain0",
    "qd_BCMain1",
    "qd_BHilite",
    "qd_BCMain3",
    "qd_BEnd0",
    "qd_BEnd1",
    "qd_BEnd2",
    "qd_BEnd3",
    "qd_BLong8",
    "qd_BEnd9",
    "qd_BEnd10",
    "qd_BEnd11",
    "qd_BXLong8",
    "qd_BXEnd9",
    "qd_BXEnd10",
    "qd_BXEnd11",
    "qd_BCEnd0",
    "qd_BCEnd1",
    "qd_BSloHilite",
    "qd_BCEnd3",
    "qd_BAvg",
    "qd_BAddPin",
    "qd_BAddOver",
    "qd_BSubPin",
    "qd_BTransparent",
    "qd_BMax",
    "qd_BSubOver",
    "qd_BMin",
    "qd_Bsetup0",
    "qd_BLeft0",
    "qd_Rmask0",
    "qd_Rmask1",
    "qd_Rmask2",
    "qd_Rmask3",
    "qd_Rmask8",
    "qd_Rmask9",
    "qd_Rmask10",
    "qd_Rmask11",
    "qd_RXmask8",
    "qd_RXmask9",
    "qd_RXmask10",
    "qd_RXmask11",
    "qd_RAvg",
    "qd_RAddPin",
    "qd_RAddOver",
    "qd_RSubPin",
    "qd_RTransparent",
    "qd_RMax",
    "qd_RSubOver",
    "qd_RMin",
    "qd_RCmask0",
    "qd_RCmask1",
    "qd_RSlowHilite",
    "qd_RCmask3",
    "qd_RHilite",
    "qd_StMask0",
    "qd_StMask1",
    "qd_StMask2",
    "qd_StMask3",
    "qd_StAvg",
    "qd_StAddPin",
    "qd_StAddOver",
    "qd_StSubPin",
    "qd_StTransparent",
    "qd_StMax",
    "qd_StSubOver",
    "qd_StMin",
    "qd_StHilite",
    "qd_SlMask8",
    "qd_SlMask9",
    "qd_SlMask10",
    "qd_SlMask11",
    "qd_SlxMask8",
    "qd_SlxMask9",
    "qd_SlxMask10",
    "qd_SlxMask11",
    "qd_SlAvg",
    "qd_SlAddPin",
    "qd_SlAddOver",
    "qd_SlSubPin",
    "qd_SlTransparent",
    "qd_SlMax",
    "qd_SlSubOver",
    "qd_SlMin",
    "qd_SlHilite",
    "qd_ItabMatch",
    "qd_Colorthing2Index",
    "Myst_B93",
    "qd_AllocRunbuf",
    "qd_InitRgn",
    "qd_Scaleblt",
    "qd_StnoStack",
    "qd_Blitcase",
    "qd_Stscanloop",
    "qd_Picitem1",
    "qd_MakeGrayItab",
    "qd_FastLine",
    "qd_FastSlant",
    "qd_BitsDevloop",
    "Myst_B9F",
    "Myst_BA0",
    "Myst_BA1",
    "Myst_BA2",
    "Myst_BA3",
    "Myst_BA4",
    "Myst_BA5",
    "Myst_BA6",
    "Myst_BA7",
    "qd_Stcolortab",
    "qd_Stgraytab",
    "qd_StSearchtab",
    "qd_ScaleIndxd2Indxd",
    "Myst_BAC",
    "Myst_BAD",
    "Myst_BAE",
    "Myst_BAF",
    "Myst_BB0",
    "Myst_BB1",
    "Myst_BB2",
    "Myst_BB3",
    "Myst_BB4",
    "Myst_BB5",
    "Myst_BB6",
    "Myst_BB7",
    "Myst_BB8",
    "Myst_BB9",
    "Myst_BBA",
    "qd_Barith16Setup",
    "qd_Barith32Setup",
    "Myst_BBD",
    "Myst_BBE",
    "Myst_BBF",
    "qd_AlphaDispatch",
    "qd_StreamToMask",
    "QT_MatrixMathDispatch",
    "NQD_Misc",
    "qd_GetpmData",
    "Myst_BC5",
    "Myst_BC6",
    "Myst_BC7",
    "Myst_BC8",
    "IconDispatch",
    "DeviceLoop",
    "Myst_BCB",
    "Myst_BCC",
    "Myst_BCD",
    "UnicodeDispatch",
    "ProcessMgrDispatch",
    "Myst_BD0",
    "Myst_BD1",
    "Myst_BD2",
    "Myst_BD3",
    "Myst_BD4",
    "Myst_BD5",
    "Myst_BD6",
    "Myst_BD7",
    "Myst_BD8",
    "Myst_BD9",
    "Myst_BDA",
    "Myst_BDB",
    "Myst_BDC",
    "Myst_BDD",
    "Myst_BDE",
    "Myst_BDF",
    "Myst_BE0",
    "Myst_BE1",
    "Myst_BE2",
    "Myst_BE3",
    "Myst_BE4",
    "Myst_BE5",
    "Myst_BE6",
    "Myst_BE7",
    "Myst_BE8",
    "Myst_BE9",
    "ModemMgr",
    "DisplayMgr",
    "ButtonMgr",
    "DragDispatch",
    "ColorMatch",
    "TTSMgr",
    "AROSE",
    "GestaltValueDispatch",
    "ThreadDispatch",
    "EddyTrap",
    "XTNDMgr",
    "DSPManager",
    "CollectionMgr",
    "SynchIdleTime",
    "StdOpcodeProc",
    "AuxDispatch",
    "Myst_BFA",
    "GX_Message_Mgr",
    "TranslationMgr",
    "Myst_BFD",
    "GXPrint_Mgr"
};


static C14RoutineDescriptor osTrap[0x100] = {
    { _MixedModeMagic }, /* Open */
    { _MixedModeMagic }, /* Close */
    { _MixedModeMagic }, /* Read */
    { _MixedModeMagic }, /* Write */
    { _MixedModeMagic }, /* Control */
    { _MixedModeMagic }, /* Status */
    { _MixedModeMagic }, /* KillIO */
    { _MixedModeMagic }, /* GetVolInfo */
    { _MixedModeMagic }, /* Create */
    { _MixedModeMagic }, /* Delete */
    { _MixedModeMagic }, /* OpenRF */
    { _MixedModeMagic }, /* ReName */
    { _MixedModeMagic }, /* GetFileInfo */
    { _MixedModeMagic }, /* SetFileInfo */
    { _MixedModeMagic }, /* UnMountVol */
    { _MixedModeMagic }, /* MountVol */
    { _MixedModeMagic }, /* Allocate */
    { _MixedModeMagic }, /* GetEOF */
    { _MixedModeMagic }, /* SetEOF */
    { _MixedModeMagic }, /* FlushVol */
    { _MixedModeMagic }, /* GetVol */
    { _MixedModeMagic }, /* SetVol */
    { _MixedModeMagic }, /* FInitQueue */
    { _MixedModeMagic }, /* Eject */
    { _MixedModeMagic }, /* GetFPos */
    { _MixedModeMagic }, /* InitZone */
    { _MixedModeMagic }, /* GetZone */
    { _MixedModeMagic }, /* SetZone */
    { _MixedModeMagic }, /* FreeMem */
    { _MixedModeMagic }, /* MaxMem */
    { _MixedModeMagic, C14MM_L_L_A0_D0, (ProcPtr)&NewPtr }, /* NewPtr */
    { _MixedModeMagic }, /* DisposPtr */
    { _MixedModeMagic }, /* SetPtrSize */
    { _MixedModeMagic }, /* GetPtrSize */
    { _MixedModeMagic }, /* NewHandle */
    { _MixedModeMagic, C14MM_V_L_A0, (ProcPtr)&DisposeHandle }, /* DisposHandle */
    { _MixedModeMagic }, /* SetHandleSize */
    { _MixedModeMagic }, /* GetHandleSize */
    { _MixedModeMagic }, /* HandleZone */
    { _MixedModeMagic }, /* ReAllocHandle */
    { _MixedModeMagic }, /* RecoverHandle */
    { _MixedModeMagic, C14MM_V_L_A0, (ProcPtr)&HLock }, /* HLock */
    { _MixedModeMagic, C14MM_V_L_A0, (ProcPtr)&HUnlock }, /* HUnLock */
    { _MixedModeMagic }, /* EmptyHandle */
    { _MixedModeMagic }, /* InitApplZone */
    { _MixedModeMagic }, /* SetApplLimit */
    { _MixedModeMagic }, /* BlockMove */
    { _MixedModeMagic }, /* PostEvent */
    { _MixedModeMagic, C14MM_W_WL_D0_D0_A0, (ProcPtr)&C14OSEventAvail }, /* OSEventAvail */
    { _MixedModeMagic }, /* GetOSEvent */
    { _MixedModeMagic }, /* FlushEvents */
    { _MixedModeMagic }, /* VInstall */
    { _MixedModeMagic }, /* VRemove */
    { _MixedModeMagic }, /* OffLine */
    { _MixedModeMagic }, /* MoreMasters */
    { _MixedModeMagic }, /* ReadParam */
    { _MixedModeMagic }, /* WriteParam */
    { _MixedModeMagic }, /* ReadDateTime */
    { _MixedModeMagic }, /* SetDateTime */
    { _MixedModeMagic }, /* Delay */
    { _MixedModeMagic }, /* CmpString */
    { _MixedModeMagic }, /* DrvrInstall */
    { _MixedModeMagic }, /* DrvrRemove */
    { _MixedModeMagic }, /* InitUtil */
    { _MixedModeMagic }, /* ResrvMem */
    { _MixedModeMagic }, /* SetFilLock */
    { _MixedModeMagic }, /* RstFilLock */
    { _MixedModeMagic }, /* SetFilType */
    { _MixedModeMagic }, /* SetFPos */
    { _MixedModeMagic }, /* FlushFile */
    { _MixedModeMagic, C14MM_L_W_A0_D0, (ProcPtr)&C14GetTrapAddress }, /* GetTrapAddress */
    { _MixedModeMagic, C14MM_V_LW_A0_D0, (ProcPtr)&C14SetTrapAddress }, /* SetTrapAddress */
    { _MixedModeMagic }, /* PtrZone */
    { _MixedModeMagic, C14MM_V_L_A0, (ProcPtr)&HPurge }, /* HPurge */
    { _MixedModeMagic }, /* HNoPurge */
    { _MixedModeMagic }, /* SetGrowZone */
    { _MixedModeMagic }, /* CompactMem */
    { _MixedModeMagic }, /* PurgeMem */
    { _MixedModeMagic }, /* AddDrive */
    { _MixedModeMagic }, /* RDrvrInstall */
    { _MixedModeMagic }, /* RelString */
    { _MixedModeMagic }, /* ReadXPRam */
    { _MixedModeMagic }, /* WriteXPRam */
    { _MixedModeMagic }, /* ClkNoMem */
    { _MixedModeMagic }, /* UprString */
    { _MixedModeMagic, C14MM_L_L_D0_D0, (ProcPtr)&C14StripAddress }, /* StripAddress */
    { _MixedModeMagic }, /* LwrString */
    { _MixedModeMagic }, /* SetAppBase */
    { _MixedModeMagic }, /* InsTime */
    { _MixedModeMagic }, /* RmvTime */
    { _MixedModeMagic }, /* PrimeTime */
    { _MixedModeMagic }, /* PowerOFF */
    { _MixedModeMagic }, /* VM_Dispatch */
    { _MixedModeMagic }, /* SwapMMUMode */
    { _MixedModeMagic }, /* NMInstall */
    { _MixedModeMagic }, /* NMRemove */
    { _MixedModeMagic }, /* HFsDispatch */
    { _MixedModeMagic }, /* MaxBlock */
    { _MixedModeMagic, C14MM_REGS, (ProcPtr)&C14PurgeSpaceM68K }, /* PurgeSpace */
    { _MixedModeMagic, C14MM_L_V_D0, (ProcPtr)&C14MaxApplZone }, /* MaxApplZone */
    { _MixedModeMagic }, /* MoveHHi */
    { _MixedModeMagic }, /* StackSpace */
    { _MixedModeMagic }, /* NewEmptyHandle */
    { _MixedModeMagic }, /* HSetRBit */
    { _MixedModeMagic }, /* HClrRBit */
    { _MixedModeMagic }, /* HGetState */
    { _MixedModeMagic }, /* HSetState */
    { _MixedModeMagic }, /* SI_TestMgr */
    { _MixedModeMagic }, /* InitFs */
    { _MixedModeMagic }, /* InitEvents */
    { _MixedModeMagic }, /* SlotManager */
    { _MixedModeMagic }, /* SlotVInstall */
    { _MixedModeMagic }, /* SlotVRemove */
    { _MixedModeMagic }, /* AttachVBL */
    { _MixedModeMagic }, /* DoVBLTask */
    { _MixedModeMagic }, /* Reserved4 */
    { _MixedModeMagic }, /* CacheMgr */
    { _MixedModeMagic }, /* SIntInstall */
    { _MixedModeMagic }, /* SIntRemove */
    { _MixedModeMagic }, /* CountADBs */
    { _MixedModeMagic }, /* GetIndADB */
    { _MixedModeMagic }, /* GetADBInfo */
    { _MixedModeMagic }, /* SetADBInfo */
    { _MixedModeMagic }, /* ADBReInit */
    { _MixedModeMagic }, /* ADBOp */
    { _MixedModeMagic }, /* GetDefaultStartup */
    { _MixedModeMagic }, /* SetDefaultStartup */
    { _MixedModeMagic }, /* InternalWait */
    { _MixedModeMagic }, /* GetVideoDefault */
    { _MixedModeMagic }, /* SetVideoDefault */
    { _MixedModeMagic }, /* DTInstall */
    { _MixedModeMagic }, /* SetOSDefault */
    { _MixedModeMagic }, /* GetOSDefault */
    { _MixedModeMagic }, /* PowerMgr */
    { _MixedModeMagic }, /* IOPInfoAccess */
    { _MixedModeMagic }, /* IOPMsgRequest */
    { _MixedModeMagic }, /* IOPMoveData */
    { _MixedModeMagic }, /* SCSIAtomic */
    { _MixedModeMagic }, /* Sleep */
    { _MixedModeMagic }, /* CommToolboxDispatch */
    { _MixedModeMagic }, /* Wakeup */
    { _MixedModeMagic }, /* DebugUtil */
    { _MixedModeMagic }, /* BTreeDispatch */
    { _MixedModeMagic }, /* DeferUserFn */
    { _MixedModeMagic, C14MM_W_WL_D0_D0_A0, (ProcPtr)&C14SysEnvirons }, /* SysEnvirons */
    { _MixedModeMagic }, /* Xlate24to32 */
    { _MixedModeMagic }, /* EgretDispatch */
    { _MixedModeMagic }, /* MicroTickCount */
    { _MixedModeMagic }, /* ServerDispatch */
    { _MixedModeMagic }, /* Myst_95 */
    { _MixedModeMagic }, /* Myst_96 */
    { _MixedModeMagic }, /* FPPriv */
    { _MixedModeMagic, C14MM_V_V_DESC, (ProcPtr)&C14Unimplemented }, /* HWPriv -- for cache control? See segment.c in Executor source. */
    { _MixedModeMagic }, /* XToolTable */
    { _MixedModeMagic }, /* vProcHelper */
    { _MixedModeMagic }, /* Messager */
    { _MixedModeMagic }, /* NewPtrStartup */
    { _MixedModeMagic }, /* MoveHLow */
    { _MixedModeMagic }, /* PowerDispatchPublic */
    { _MixedModeMagic }, /* Power */
    { _MixedModeMagic }, /* Myst_A0 */
    { _MixedModeMagic }, /* Myst_A1 */
    { _MixedModeMagic }, /* Myst_A2 */
    { _MixedModeMagic }, /* Myst_A3 */
    { _MixedModeMagic }, /* HeapDispatch */
    { _MixedModeMagic }, /* VisRegionChanged */
    { _MixedModeMagic }, /* Myst_A6 */
    { _MixedModeMagic }, /* Myst_A7 */
    { _MixedModeMagic }, /* Myst_A8 */
    { _MixedModeMagic }, /* Myst_A9 */
    { _MixedModeMagic }, /* Myst_AA */
    { _MixedModeMagic }, /* Myst_AB */
    { _MixedModeMagic }, /* FileSysMgr */
    { _MixedModeMagic }, /* Gestalt */
    { _MixedModeMagic }, /* Myst_AE */
    { _MixedModeMagic }, /* Myst_AF */
    { _MixedModeMagic }, /* Myst_B0 */
    { _MixedModeMagic }, /* Myst_B1 */
    { _MixedModeMagic }, /* Myst_B2 */
    { _MixedModeMagic }, /* Myst_B3 */
    { _MixedModeMagic }, /* Myst_B4 */
    { _MixedModeMagic }, /* jGoDriver */
    { _MixedModeMagic }, /* jWaitUntil */
    { _MixedModeMagic }, /* jSyncWait */
    { _MixedModeMagic }, /* jSoundDead */
    { _MixedModeMagic }, /* jDisptch */
    { _MixedModeMagic }, /* jIAZInit */
    { _MixedModeMagic }, /* jIAZPostInit */
    { _MixedModeMagic }, /* jLaunchInit */
    { _MixedModeMagic }, /* jCacheFlush */
    { _MixedModeMagic }, /* jSysUtil */
    { _MixedModeMagic }, /* jLg2Phys */
    { _MixedModeMagic }, /* jFlushCache */
    { _MixedModeMagic }, /* jGetBlock */
    { _MixedModeMagic }, /* jMarkBlock */
    { _MixedModeMagic }, /* jRelBlock */
    { _MixedModeMagic }, /* jTrashBlocks */
    { _MixedModeMagic }, /* jTrashVBlks */
    { _MixedModeMagic }, /* jCacheWrIP */
    { _MixedModeMagic }, /* jCacheRdIP */
    { _MixedModeMagic }, /* jBasicIO */
    { _MixedModeMagic }, /* jRdBlocks */
    { _MixedModeMagic }, /* jWrBlocks */
    { _MixedModeMagic }, /* jSetUpTags */
    { _MixedModeMagic }, /* jBTClose */
    { _MixedModeMagic }, /* jBTDelete */
    { _MixedModeMagic }, /* jBTFlush */
    { _MixedModeMagic }, /* jBTGetRecord */
    { _MixedModeMagic }, /* jBTInsert */
    { _MixedModeMagic }, /* jBTOpen */
    { _MixedModeMagic }, /* jBTSearch */
    { _MixedModeMagic }, /* jBTUpdate */
    { _MixedModeMagic }, /* jGetNode */
    { _MixedModeMagic }, /* jRelNode */
    { _MixedModeMagic }, /* jAllocNode */
    { _MixedModeMagic }, /* jFreeNode */
    { _MixedModeMagic }, /* jExtBTFile */
    { _MixedModeMagic }, /* jDeallocFile */
    { _MixedModeMagic }, /* jExtendFile */
    { _MixedModeMagic }, /* jTruncateFile */
    { _MixedModeMagic }, /* jCMSetUp */
    { _MixedModeMagic }, /* PPCToolBoxMgr */
    { _MixedModeMagic }, /* jDtrmV1 */
    { _MixedModeMagic }, /* jBlkAlloc */
    { _MixedModeMagic }, /* jBlkDeAlloc */
    { _MixedModeMagic }, /* jFileOpen */
    { _MixedModeMagic }, /* jPermssnChk */
    { _MixedModeMagic }, /* jFndFilName */
    { _MixedModeMagic }, /* jRfNCall */
    { _MixedModeMagic }, /* jAdjEOF */
    { _MixedModeMagic }, /* jPixel2Char */
    { _MixedModeMagic }, /* jChar2Pixel */
    { _MixedModeMagic }, /* jHiliteText */
    { _MixedModeMagic }, /* jFileClose */
    { _MixedModeMagic }, /* jFileRead */
    { _MixedModeMagic }, /* jFileWrite */
    { _MixedModeMagic }, /* jDispatchHelper */
    { _MixedModeMagic }, /* jUpdAldMDB */
    { _MixedModeMagic }, /* jCkExtFs */
    { _MixedModeMagic }, /* jDTrmV3 */
    { _MixedModeMagic }, /* jBMChk */
    { _MixedModeMagic }, /* jTstMod */
    { _MixedModeMagic }, /* jLocCRec */
    { _MixedModeMagic }, /* jTreeSearch */
    { _MixedModeMagic }, /* jMapFBlock */
    { _MixedModeMagic }, /* jXFSearch */
    { _MixedModeMagic }, /* jReadBM */
    { _MixedModeMagic }, /* jDoEject */
    { _MixedModeMagic }, /* jSegStack */
    { _MixedModeMagic }, /* jSuperLoad */
    { _MixedModeMagic }, /* jCmpFrm */
    { _MixedModeMagic }, /* jNewMap */
    { _MixedModeMagic }, /* jCheckLoad */
    { _MixedModeMagic }, /* TETrimMeasure */
    { _MixedModeMagic }, /* TEFindWord */
    { _MixedModeMagic }  /* TEFindLine */
};


static C14RoutineDescriptor tbTrap[0x400] = {
    { _MixedModeMagic }, /* SoundDispatch */
    { _MixedModeMagic }, /* SndDisposeChannel */
    { _MixedModeMagic }, /* SndAddModifier */
    { _MixedModeMagic }, /* SndDoCommand */
    { _MixedModeMagic }, /* SndDoImmediate */
    { _MixedModeMagic }, /* SndPlay */
    { _MixedModeMagic }, /* SndControl */
    { _MixedModeMagic }, /* SndNewChannel */
    { _MixedModeMagic }, /* InitProcMenu */
    { _MixedModeMagic }, /* GetCVariant */
    { _MixedModeMagic }, /* GetWVariant */
    { _MixedModeMagic }, /* PopUpMenuSelect */
    { _MixedModeMagic }, /* RGetResource */
    { _MixedModeMagic }, /* Count1Resources */
    { _MixedModeMagic }, /* Get1IxResource */
    { _MixedModeMagic }, /* Get1IxType */
    { _MixedModeMagic }, /* Unique1ID */
    { _MixedModeMagic }, /* TeSelView */
    { _MixedModeMagic }, /* TePinScroll */
    { _MixedModeMagic }, /* TeAutoView */
    { _MixedModeMagic }, /* SetFractEnable */
    { _MixedModeMagic }, /* SCSiDispatch */
    { _MixedModeMagic }, /* Pack8 */
    { _MixedModeMagic }, /* CopyMask */
    { _MixedModeMagic }, /* FixAtan2 */
    { _MixedModeMagic }, /* XMunger */
    { _MixedModeMagic }, /* HOpenResFile */
    { _MixedModeMagic }, /* HCreateResFile */
    { _MixedModeMagic }, /* Count1Types */
    { _MixedModeMagic }, /* InvalMenuBar */
    { _MixedModeMagic }, /* SaveRestoreBits */
    { _MixedModeMagic }, /* Get1Resource */
    { _MixedModeMagic }, /* Get1NamedResource */
    { _MixedModeMagic }, /* MaxSizeRsrc */
    { _MixedModeMagic }, /* RsrcMgr */
    { _MixedModeMagic }, /* AliasMgr */
    { _MixedModeMagic }, /* HFSUtils */
    { _MixedModeMagic }, /* MenuDispatch */
    { _MixedModeMagic }, /* InsMenuItem */
    { _MixedModeMagic }, /* HideD_Item */
    { _MixedModeMagic }, /* ShowD_Item */
    { _MixedModeMagic }, /* LayerDispatch */
    { _MixedModeMagic }, /* ComponentDispatch */
    { _MixedModeMagic }, /* PPCBrowser */
    { _MixedModeMagic }, /* Pack10 */
    { _MixedModeMagic }, /* DataPubMgr */
    { _MixedModeMagic }, /* Pack12 */
    { _MixedModeMagic }, /* DatBaseMgr */
    { _MixedModeMagic }, /* Help_Pack */
    { _MixedModeMagic }, /* Pack15 */
    { _MixedModeMagic }, /* QD_GX_Mgr */
    { _MixedModeMagic }, /* ScrnBitMap */
    { _MixedModeMagic }, /* SetFScaleDisable */
    { _MixedModeMagic }, /* FontMetrics */
    { _MixedModeMagic }, /* GetMaskTable */
    { _MixedModeMagic }, /* MeasureText */
    { _MixedModeMagic }, /* CalcMask */
    { _MixedModeMagic }, /* SeedFill */
    { _MixedModeMagic }, /* ZoomWindow */
    { _MixedModeMagic }, /* TrackBox */
    { _MixedModeMagic }, /* TEGetOffset */
    { _MixedModeMagic }, /* TEDispatch */
    { _MixedModeMagic }, /* TEStyleNew */
    { _MixedModeMagic }, /* Long2Fix */
    { _MixedModeMagic }, /* Fix2Long */
    { _MixedModeMagic }, /* Fix2Frac */
    { _MixedModeMagic }, /* Fract2Fix */
    { _MixedModeMagic }, /* Fix2X */
    { _MixedModeMagic }, /* X2Fix */
    { _MixedModeMagic }, /* Frac2X */
    { _MixedModeMagic }, /* X2Frac */
    { _MixedModeMagic }, /* FracCos */
    { _MixedModeMagic }, /* FracSin */
    { _MixedModeMagic }, /* FracSqrt */
    { _MixedModeMagic }, /* FracMul */
    { _MixedModeMagic }, /* FracDiv */
    { _MixedModeMagic }, /* ScrollDelay */
    { _MixedModeMagic }, /* FixDiv */
    { _MixedModeMagic }, /* GetItmCmdkey */
    { _MixedModeMagic }, /* SetItmCmdkey */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&InitCursor }, /* InitCursor */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&SetCursor }, /* SetCursor */
    { _MixedModeMagic }, /* HideCursor */
    { _MixedModeMagic }, /* ShowCursor */
    { _MixedModeMagic }, /* FontDispatch */
    { _MixedModeMagic }, /* ShieldCursor */
    { _MixedModeMagic }, /* ObscureCursor */
    { _MixedModeMagic }, /* Myst_857 */
    { _MixedModeMagic }, /* BitAnd */
    { _MixedModeMagic }, /* BitXOr */
    { _MixedModeMagic }, /* BitNot */
    { _MixedModeMagic }, /* BitOr */
    { _MixedModeMagic }, /* BitShift */
    { _MixedModeMagic }, /* BitTst */
    { _MixedModeMagic }, /* BitSet */
    { _MixedModeMagic }, /* BitClr */
    { _MixedModeMagic, C14MM_B_WLLL, (ProcPtr)&C14WaitNextEvent }, /* WaitNextEvent */
    { _MixedModeMagic }, /* Random */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&ForeColor }, /* ForeColor */
    { _MixedModeMagic }, /* BackColor */
    { _MixedModeMagic }, /* ColorBit */
    { _MixedModeMagic }, /* GetPixel */
    { _MixedModeMagic }, /* StuffHex */
    { _MixedModeMagic }, /* LongMul */
    { _MixedModeMagic }, /* FixMul */
    { _MixedModeMagic }, /* FixRatio */
    { _MixedModeMagic }, /* HiWord */
    { _MixedModeMagic }, /* LoWord */
    { _MixedModeMagic }, /* FixRound */
    { _MixedModeMagic }, /* InitPort */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&C14InitGraf }, /* InitGraf */
    { _MixedModeMagic }, /* OpenPort */
    { _MixedModeMagic }, /* LocalToGlobal */
    { _MixedModeMagic }, /* GlobalToLocal */
    { _MixedModeMagic }, /* GrafDevice */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&C14SetPort }, /* SetPort */
    { _MixedModeMagic }, /* GetPort */
    { _MixedModeMagic }, /* SetPBits */
    { _MixedModeMagic }, /* PortSize */
    { _MixedModeMagic }, /* MovePortTo */
    { _MixedModeMagic, C14MM_V_WW, (ProcPtr)&C14SetOrigin }, /* SetOrigin */
    { _MixedModeMagic }, /* SetClip */
    { _MixedModeMagic }, /* GetClip */
    { _MixedModeMagic }, /* ClipRect */
    { _MixedModeMagic }, /* BackPat */
    { _MixedModeMagic }, /* ClosePort */
    { _MixedModeMagic }, /* AddPt */
    { _MixedModeMagic }, /* SubPt */
    { _MixedModeMagic }, /* SetPt */
    { _MixedModeMagic }, /* EqualPt */
    { _MixedModeMagic }, /* StdText */
    { _MixedModeMagic }, /* DrawChar */
    { _MixedModeMagic }, /* DrawString */
    { _MixedModeMagic }, /* DrawText */
    { _MixedModeMagic }, /* TextWidth */
    { _MixedModeMagic }, /* TextFont */
    { _MixedModeMagic }, /* TextFace */
    { _MixedModeMagic }, /* TextMode */
    { _MixedModeMagic }, /* TextSize */
    { _MixedModeMagic }, /* GetFontInfo */
    { _MixedModeMagic }, /* StringWidth */
    { _MixedModeMagic }, /* CharWidth */
    { _MixedModeMagic }, /* SpaceExtra */
    { _MixedModeMagic }, /* OSDispatch */
    { _MixedModeMagic }, /* StdLine */
    { _MixedModeMagic }, /* LineTo */
    { _MixedModeMagic }, /* Line */
    { _MixedModeMagic }, /* MoveTo */
    { _MixedModeMagic }, /* Move */
    { _MixedModeMagic }, /* ShutDown */
    { _MixedModeMagic }, /* HidePen */
    { _MixedModeMagic }, /* ShowPen */
    { _MixedModeMagic }, /* GetPenState */
    { _MixedModeMagic }, /* SetPenState */
    { _MixedModeMagic }, /* GetPen */
    { _MixedModeMagic }, /* PenSize */
    { _MixedModeMagic }, /* PenMode */
    { _MixedModeMagic }, /* PenPat */
    { _MixedModeMagic }, /* PenNormal */
    { _MixedModeMagic, C14MM_V_V_DESC, (ProcPtr)&C14Unimplemented }, /* UnimplTrap */
    { _MixedModeMagic }, /* StdRect */
    { _MixedModeMagic }, /* FrameRect */
    { _MixedModeMagic }, /* PaintRect */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&EraseRect }, /* EraseRect */
    { _MixedModeMagic }, /* InverRect */
    { _MixedModeMagic }, /* FillRect */
    { _MixedModeMagic }, /* EqualRect */
    { _MixedModeMagic }, /* SetRect */
    { _MixedModeMagic }, /* OffSetRect */
    { _MixedModeMagic }, /* InSetRect */
    { _MixedModeMagic }, /* SectRect */
    { _MixedModeMagic }, /* UnionRect */
    { _MixedModeMagic }, /* Pt2Rect */
    { _MixedModeMagic }, /* PtInRect */
    { _MixedModeMagic }, /* EmptyRect */
    { _MixedModeMagic }, /* StdRRect */
    { _MixedModeMagic }, /* FrameRoundRect */
    { _MixedModeMagic }, /* PaintRoundRect */
    { _MixedModeMagic }, /* EraseRoundRect */
    { _MixedModeMagic }, /* InverRoundRect */
    { _MixedModeMagic }, /* FillRoundRect */
    { _MixedModeMagic }, /* ScriptUtil */
    { _MixedModeMagic }, /* StdOval */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&FrameOval }, /* FrameOval */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&PaintOval }, /* PaintOval */
    { _MixedModeMagic }, /* EraseOval */
    { _MixedModeMagic }, /* InvertOval */
    { _MixedModeMagic }, /* FillOval */
    { _MixedModeMagic }, /* SlopeFromAngle */
    { _MixedModeMagic }, /* StdArc */
    { _MixedModeMagic }, /* FrameArc */
    { _MixedModeMagic }, /* PaintArc */
    { _MixedModeMagic }, /* EraseArc */
    { _MixedModeMagic }, /* InvertArc */
    { _MixedModeMagic }, /* FillArc */
    { _MixedModeMagic }, /* PtToAngle */
    { _MixedModeMagic }, /* AngleFromSlope */
    { _MixedModeMagic }, /* StdPoly */
    { _MixedModeMagic }, /* FramePoly */
    { _MixedModeMagic }, /* PaintPoly */
    { _MixedModeMagic }, /* ErasePoly */
    { _MixedModeMagic }, /* InvertPoly */
    { _MixedModeMagic }, /* FillPoly */
    { _MixedModeMagic }, /* OpenPoly */
    { _MixedModeMagic }, /* ClosePgon */
    { _MixedModeMagic }, /* KillPoly */
    { _MixedModeMagic }, /* OffSetPoly */
    { _MixedModeMagic }, /* PackBits */
    { _MixedModeMagic }, /* UnpackBits */
    { _MixedModeMagic }, /* StdRgn */
    { _MixedModeMagic }, /* FrameRgn */
    { _MixedModeMagic }, /* PaintRgn */
    { _MixedModeMagic }, /* EraseRgn */
    { _MixedModeMagic }, /* InverRgn */
    { _MixedModeMagic }, /* FillRgn */
    { _MixedModeMagic }, /* BitmapToRegion */
    { _MixedModeMagic, C14MM_L_V, (ProcPtr)&C14NewRgn }, /* NewRgn */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&C14DisposeRgn }, /* DisposRgn */
    { _MixedModeMagic }, /* OpenRgn */
    { _MixedModeMagic }, /* CloseRgn */
    { _MixedModeMagic, C14MM_V_LL, (ProcPtr)&C14CopyRgn }, /* CopyRgn */
    { _MixedModeMagic }, /* SetEmptyRgn */
    { _MixedModeMagic, C14MM_V_LWWWW, (ProcPtr)&C14SetRectRgn }, /* SetRecRgn */
    { _MixedModeMagic, C14MM_V_LL, (ProcPtr)&C14RectRgn }, /* RectRgn */
    { _MixedModeMagic }, /* OfSetRgn */
    { _MixedModeMagic }, /* InSetRgn */
    { _MixedModeMagic, C14MM_B_L, (ProcPtr)&C14EmptyRgn }, /* EmptyRgn */
    { _MixedModeMagic }, /* EqualRgn */
    { _MixedModeMagic, C14MM_V_LLL, (ProcPtr)&C14SectRgn }, /* SectRgn */
    { _MixedModeMagic }, /* UnionRgn */
    { _MixedModeMagic, C14MM_V_LLL, (ProcPtr)&C14DiffRgn }, /* DiffRgn */
    { _MixedModeMagic }, /* XOrRgn */
    { _MixedModeMagic, C14MM_B_LL, (ProcPtr)&C14PtInRgn }, /* PtInRgn */
    { _MixedModeMagic }, /* RectInRgn */
    { _MixedModeMagic }, /* SetStdProcs */
    { _MixedModeMagic }, /* StdBits */
    { _MixedModeMagic }, /* CopyBits */
    { _MixedModeMagic }, /* StdTxMeas */
    { _MixedModeMagic }, /* StdGetPic */
    { _MixedModeMagic }, /* ScrollRect */
    { _MixedModeMagic }, /* StdPutPic */
    { _MixedModeMagic }, /* StdComment */
    { _MixedModeMagic }, /* PicComment */
    { _MixedModeMagic }, /* OpenPicture */
    { _MixedModeMagic }, /* ClosePicture */
    { _MixedModeMagic }, /* KillPicture */
    { _MixedModeMagic }, /* DrawPicture */
    { _MixedModeMagic }, /* Reserved11 */
    { _MixedModeMagic }, /* ScalePt */
    { _MixedModeMagic }, /* MapPt */
    { _MixedModeMagic }, /* MapRect */
    { _MixedModeMagic }, /* MapRgn */
    { _MixedModeMagic }, /* MapPoly */
    { _MixedModeMagic }, /* PrintMgr */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&C14InitFonts }, /* InitFonts */
    { _MixedModeMagic }, /* GetFName */
    { _MixedModeMagic }, /* GetFNum */
    { _MixedModeMagic }, /* FMSwapFont */
    { _MixedModeMagic }, /* RealFont */
    { _MixedModeMagic }, /* SetFontLock */
    { _MixedModeMagic }, /* DrawGrowIcon */
    { _MixedModeMagic }, /* DragGrayRgn */
    { _MixedModeMagic }, /* NewString */
    { _MixedModeMagic }, /* SetString */
    { _MixedModeMagic }, /* ShowHide */
    { _MixedModeMagic }, /* CalcVis */
    { _MixedModeMagic }, /* CalcVBehind */
    { _MixedModeMagic }, /* ClipAbove */
    { _MixedModeMagic }, /* PaintOne */
    { _MixedModeMagic }, /* PaintBehind */
    { _MixedModeMagic }, /* SaveOld */
    { _MixedModeMagic }, /* DrawNew */
    { _MixedModeMagic }, /* GetWMgrPort */
    { _MixedModeMagic }, /* CheckUpDate */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&C14InitWindows }, /* InitWindows */
    { _MixedModeMagic }, /* NewWindow */
    { _MixedModeMagic }, /* DisposWindow */
    { _MixedModeMagic }, /* ShowWindow */
    { _MixedModeMagic }, /* HideWindow */
    { _MixedModeMagic }, /* GetWRefCon */
    { _MixedModeMagic }, /* SetWRefCon */
    { _MixedModeMagic }, /* GetWTitle */
    { _MixedModeMagic }, /* SetWTitle */
    { _MixedModeMagic }, /* MoveWindow */
    { _MixedModeMagic }, /* HiliteWindow */
    { _MixedModeMagic }, /* SizeWindow */
    { _MixedModeMagic }, /* TrackGoAway */
    { _MixedModeMagic }, /* SelectWindow */
    { _MixedModeMagic }, /* BringToFront */
    { _MixedModeMagic }, /* SendBehind */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&C14BeginUpdate }, /* BeginUpdate */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&C14EndUpdate }, /* EndUpdate */
    { _MixedModeMagic, C14MM_L_V, (ProcPtr)&C14FrontWindow }, /* FrontWindow */
    { _MixedModeMagic, C14MM_V_LLL, (ProcPtr)&C14DragWindow }, /* DragWindow */
    { _MixedModeMagic }, /* DragTheRgn */
    { _MixedModeMagic }, /* InvalRgn */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&C14InvalRect }, /* InvalRect */
    { _MixedModeMagic }, /* ValidRgn */
    { _MixedModeMagic }, /* ValidRect */
    { _MixedModeMagic }, /* GrowWindow */
    { _MixedModeMagic, C14MM_W_LL, (ProcPtr)&C14FindWindow }, /* FindWindow */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&C14CloseWindow }, /* CloseWindow */
    { _MixedModeMagic }, /* SetWindowPic */
    { _MixedModeMagic }, /* GetWindowPic */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&C14InitMenus }, /* InitMenus */
    { _MixedModeMagic }, /* NewMenu */
    { _MixedModeMagic }, /* DisposMenu */
    { _MixedModeMagic }, /* AppendMenu */
    { _MixedModeMagic }, /* ClearMenuBar */
    { _MixedModeMagic }, /* InsertMenu */
    { _MixedModeMagic }, /* DeleteMenu */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&DrawMenuBar }, /* DrawMenuBar */
    { _MixedModeMagic, C14MM_V_W, (ProcPtr)&HiliteMenu }, /* HiliteMenu */
    { _MixedModeMagic, C14MM_V_LW, (ProcPtr)&C14EnableItem }, /* EnableItem */
    { _MixedModeMagic, C14MM_V_LW, (ProcPtr)&C14DisableItem }, /* DisableItem */
    { _MixedModeMagic }, /* GetMenuBar */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&SetMenuBar }, /* SetMenuBar */
    { _MixedModeMagic, C14MM_L_L, (ProcPtr)&MenuSelect }, /* MenuSelect */
    { _MixedModeMagic, C14MM_L_W, (ProcPtr)&MenuKey }, /* MenuKey */
    { _MixedModeMagic }, /* GetItemIcon */
    { _MixedModeMagic }, /* SetItemIcon */
    { _MixedModeMagic }, /* GetItemStyle */
    { _MixedModeMagic }, /* SetItemStyle */
    { _MixedModeMagic }, /* GetItemMark */
    { _MixedModeMagic }, /* SetItemMark */
    { _MixedModeMagic, C14MM_V_LWB, (ProcPtr)&C14CheckItem }, /* CheckItem */
    { _MixedModeMagic }, /* GetItem */
    { _MixedModeMagic }, /* SetItem */
    { _MixedModeMagic }, /* CalcMenuSize */
    { _MixedModeMagic, C14MM_L_W, (ProcPtr)&GetMenuHandle }, /* GetMHandle */
    { _MixedModeMagic }, /* SetMFlash */
    { _MixedModeMagic }, /* PlotIcon */
    { _MixedModeMagic }, /* FlashMenuBar */
    { _MixedModeMagic, C14MM_V_LL, (ProcPtr)&AppendResMenu }, /* AddResMenu */
    { _MixedModeMagic }, /* PinRect */
    { _MixedModeMagic }, /* DeltaPoint */
    { _MixedModeMagic }, /* CountMItems */
    { _MixedModeMagic }, /* InsertResMenu */
    { _MixedModeMagic }, /* DelMenuItem */
    { _MixedModeMagic }, /* UpdateControl */
    { _MixedModeMagic }, /* NewControl */
    { _MixedModeMagic }, /* DisposControl */
    { _MixedModeMagic }, /* KillControls */
    { _MixedModeMagic }, /* ShowControl */
    { _MixedModeMagic }, /* HideControl */
    { _MixedModeMagic }, /* MoveControl */
    { _MixedModeMagic }, /* GetCRefCon */
    { _MixedModeMagic }, /* SetCRefCon */
    { _MixedModeMagic }, /* SizeControl */
    { _MixedModeMagic }, /* HiliteControl */
    { _MixedModeMagic }, /* GetCTitle */
    { _MixedModeMagic }, /* SetCTitle */
    { _MixedModeMagic }, /* GetCtlValue */
    { _MixedModeMagic }, /* GetMinCtl */
    { _MixedModeMagic }, /* GetMaxCtl */
    { _MixedModeMagic }, /* SetCtlValue */
    { _MixedModeMagic }, /* SetMinCtl */
    { _MixedModeMagic }, /* SetMaxCtl */
    { _MixedModeMagic }, /* TestControl */
    { _MixedModeMagic }, /* DragControl */
    { _MixedModeMagic }, /* TrackControl */
    { _MixedModeMagic }, /* DrawControls */
    { _MixedModeMagic }, /* GetCtlAction */
    { _MixedModeMagic }, /* SetCtlAction */
    { _MixedModeMagic }, /* FindControl */
    { _MixedModeMagic }, /* Draw1Control */
    { _MixedModeMagic }, /* DeQueue */
    { _MixedModeMagic }, /* EnQueue */
    { _MixedModeMagic }, /* GetNextEvent */
    { _MixedModeMagic, C14MM_B_WL, (ProcPtr)&EventAvail }, /* EventAvail */
    { _MixedModeMagic }, /* GetMouse */
    { _MixedModeMagic }, /* StillDown */
    { _MixedModeMagic }, /* Button */
    { _MixedModeMagic }, /* TickCount */
    { _MixedModeMagic }, /* GetKeys */
    { _MixedModeMagic }, /* WaitMouseUp */
    { _MixedModeMagic }, /* UpdtDialog */
    { _MixedModeMagic }, /* CouldDialog */
    { _MixedModeMagic }, /* FreeDialog */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&C14InitDialogs }, /* InitDialogs */
    { _MixedModeMagic }, /* GetNewDialog */
    { _MixedModeMagic }, /* NewDialog */
    { _MixedModeMagic }, /* SelIText */
    { _MixedModeMagic }, /* IsDialogEvent */
    { _MixedModeMagic }, /* DialogSelect */
    { _MixedModeMagic }, /* DrawDialog */
    { _MixedModeMagic }, /* CloseDialog */
    { _MixedModeMagic }, /* DisposDialog */
    { _MixedModeMagic }, /* FindD_Item */
    { _MixedModeMagic, C14MM_W_WL, (ProcPtr)&C14Alert }, /* Alert */
    { _MixedModeMagic }, /* StopAlert */
    { _MixedModeMagic }, /* NoteAlert */
    { _MixedModeMagic }, /* CautionAlert */
    { _MixedModeMagic }, /* CouldAlert */
    { _MixedModeMagic }, /* FreeAlert */
    { _MixedModeMagic }, /* ParamText */
    { _MixedModeMagic }, /* ErrorSound */
    { _MixedModeMagic }, /* GetDItem */
    { _MixedModeMagic }, /* SetDItem */
    { _MixedModeMagic }, /* SetIText */
    { _MixedModeMagic }, /* GetIText */
    { _MixedModeMagic }, /* ModalDialog */
    { _MixedModeMagic }, /* DetachResource */
    { _MixedModeMagic }, /* SetResPurge */
    { _MixedModeMagic }, /* CurResFile */
    { _MixedModeMagic }, /* InitResources */
    { _MixedModeMagic }, /* RsrcZoneInit */
    { _MixedModeMagic }, /* OpenResFile */
    { _MixedModeMagic }, /* UseResFile */
    { _MixedModeMagic }, /* UpdateResFile */
    { _MixedModeMagic }, /* CloseResFile */
    { _MixedModeMagic }, /* SetResLoad */
    { _MixedModeMagic }, /* CountResources */
    { _MixedModeMagic }, /* GetIndResource */
    { _MixedModeMagic }, /* CountTypes */
    { _MixedModeMagic }, /* GetIndType */
    { _MixedModeMagic, C14MM_L_LW, (ProcPtr)&GetResource }, /* GetResource */
    { _MixedModeMagic }, /* GetNamedResource */
    { _MixedModeMagic }, /* LoadResource */
    { _MixedModeMagic, C14MM_V_L, (ProcPtr)&ReleaseResource }, /* ReleaseResource */
    { _MixedModeMagic }, /* HomeResFile */
    { _MixedModeMagic }, /* SizeRsrc */
    { _MixedModeMagic }, /* GetResAttrs */
    { _MixedModeMagic }, /* SetResAttrs */
    { _MixedModeMagic }, /* GetResInfo */
    { _MixedModeMagic }, /* SetResInfo */
    { _MixedModeMagic }, /* ChangedResource */
    { _MixedModeMagic }, /* AddResource */
    { _MixedModeMagic }, /* AddReference */
    { _MixedModeMagic }, /* RmveResource */
    { _MixedModeMagic }, /* RmveReference */
    { _MixedModeMagic }, /* ResError */
    { _MixedModeMagic }, /* WriteResource */
    { _MixedModeMagic }, /* CreateResFile */
    { _MixedModeMagic }, /* SystemEvent */
    { _MixedModeMagic }, /* SystemClick */
    { _MixedModeMagic }, /* SystemTask */
    { _MixedModeMagic }, /* SystemMenu */
    { _MixedModeMagic }, /* OpenDeskAcc */
    { _MixedModeMagic }, /* CloseDeskAcc */
    { _MixedModeMagic }, /* GetPattern */
    { _MixedModeMagic, C14MM_L_W, (ProcPtr)&GetCursor }, /* GetCursor */
    { _MixedModeMagic }, /* GetString */
    { _MixedModeMagic }, /* GetIcon */
    { _MixedModeMagic }, /* GetPicture */
    { _MixedModeMagic, C14MM_L_WLL, (ProcPtr)&C14GetNewWindow }, /* GetNewWindow */
    { _MixedModeMagic }, /* GetNewControl */
    { _MixedModeMagic }, /* GetMenu */
    { _MixedModeMagic, C14MM_L_W, (ProcPtr)&GetNewMBar }, /* GetNewMBar */
    { _MixedModeMagic }, /* UniqueID */
    { _MixedModeMagic }, /* SysEdit */
    { _MixedModeMagic }, /* KeyTrans */
    { _MixedModeMagic }, /* OpenRFPerm */
    { _MixedModeMagic }, /* RsrcMapEntry */
    { _MixedModeMagic }, /* Secs2Date */
    { _MixedModeMagic }, /* Date2Secs */
    { _MixedModeMagic }, /* SysBeep */
    { _MixedModeMagic }, /* SysError */
    { _MixedModeMagic }, /* SI_PutIcon */
    { _MixedModeMagic }, /* TeGetText */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&C14TEInit }, /* TeInit */
    { _MixedModeMagic }, /* TeDispose */
    { _MixedModeMagic }, /* TextBox */
    { _MixedModeMagic }, /* TeSetText */
    { _MixedModeMagic }, /* TeCalText */
    { _MixedModeMagic }, /* TeSetSelect */
    { _MixedModeMagic }, /* TeNew */
    { _MixedModeMagic }, /* TeUpdate */
    { _MixedModeMagic }, /* TeClick */
    { _MixedModeMagic }, /* TeCopy */
    { _MixedModeMagic }, /* TeCut */
    { _MixedModeMagic }, /* TeDelete */
    { _MixedModeMagic }, /* TeActivate */
    { _MixedModeMagic }, /* TeDeactivate */
    { _MixedModeMagic }, /* TeIdle */
    { _MixedModeMagic }, /* TePaste */
    { _MixedModeMagic }, /* TeKey */
    { _MixedModeMagic }, /* TeScroll */
    { _MixedModeMagic }, /* TeInsert */
    { _MixedModeMagic }, /* TeSetJust */
    { _MixedModeMagic }, /* Munger */
    { _MixedModeMagic }, /* HandToHand */
    { _MixedModeMagic }, /* PtrToXHand */
    { _MixedModeMagic }, /* PtrToHand */
    { _MixedModeMagic }, /* HandAndHand */
    { _MixedModeMagic }, /* InitPack */
    { _MixedModeMagic }, /* InitAllPacks */
    { _MixedModeMagic }, /* Pack0 */
    { _MixedModeMagic }, /* Pack1 */
    { _MixedModeMagic }, /* Pack2 */
    { _MixedModeMagic }, /* Pack3 */
    { _MixedModeMagic }, /* Pack4 */
    { _MixedModeMagic }, /* Pack5 */
    { _MixedModeMagic }, /* Pack6 */
    { _MixedModeMagic }, /* Pack7 */
    { _MixedModeMagic }, /* PtrAndHand */
    { _MixedModeMagic, C14MM_V_W_CALLER, (ProcPtr)&C14LoadSeg }, /* LoadSeg */
    { _MixedModeMagic }, /* UnLoadSeg */
    { _MixedModeMagic }, /* Launch */
    { _MixedModeMagic }, /* Chain */
    { _MixedModeMagic, C14MM_V_V, (ProcPtr)&ExitToShell }, /* ExitToShell */
    { _MixedModeMagic }, /* GetAppParms */
    { _MixedModeMagic }, /* GetResFileAttrs */
    { _MixedModeMagic }, /* SetResFileAttrs */
    { _MixedModeMagic }, /* MethodDispatch */
    { _MixedModeMagic }, /* InfoScrap */
    { _MixedModeMagic }, /* UnlodeScrap */
    { _MixedModeMagic }, /* LodeScrap */
    { _MixedModeMagic }, /* ZeroScrap */
    { _MixedModeMagic }, /* GetScrap */
    { _MixedModeMagic }, /* PutScrap */
    { _MixedModeMagic }, /* Debugger */
    { _MixedModeMagic }, /* XOpenCPort */
    { _MixedModeMagic }, /* InitCPort */
    { _MixedModeMagic }, /* CloseCPort */
    { _MixedModeMagic }, /* NewPixMap */
    { _MixedModeMagic }, /* DisposPixMap */
    { _MixedModeMagic }, /* CopyPixMap */
    { _MixedModeMagic }, /* SetPortPix */
    { _MixedModeMagic }, /* NewPixPat */
    { _MixedModeMagic }, /* DisposPixPat */
    { _MixedModeMagic }, /* CopyPixPat */
    { _MixedModeMagic }, /* PenPixPat */
    { _MixedModeMagic }, /* BackPixPat */
    { _MixedModeMagic }, /* GetPixPat */
    { _MixedModeMagic }, /* MakeRGBPat */
    { _MixedModeMagic }, /* FillCRect */
    { _MixedModeMagic }, /* FillCOval */
    { _MixedModeMagic }, /* FillCRoundRect */
    { _MixedModeMagic }, /* FillCArc */
    { _MixedModeMagic }, /* FillCRgn */
    { _MixedModeMagic }, /* FillCPoly */
    { _MixedModeMagic }, /* RGBForeColor */
    { _MixedModeMagic }, /* RGBBackColor */
    { _MixedModeMagic }, /* SetCPixel */
    { _MixedModeMagic }, /* GetCPixel */
    { _MixedModeMagic }, /* GetCTable */
    { _MixedModeMagic }, /* GetForeColor */
    { _MixedModeMagic }, /* GetBackColor */
    { _MixedModeMagic }, /* GetCCursor */
    { _MixedModeMagic }, /* SetCCursor */
    { _MixedModeMagic }, /* AllocCursor */
    { _MixedModeMagic }, /* GetCIcon */
    { _MixedModeMagic }, /* PlotCIcon */
    { _MixedModeMagic }, /* OpenCPicture */
    { _MixedModeMagic }, /* OpColor */
    { _MixedModeMagic }, /* HiliteColor */
    { _MixedModeMagic }, /* CharExtra */
    { _MixedModeMagic }, /* DisposCTable */
    { _MixedModeMagic }, /* DisposCIcon */
    { _MixedModeMagic }, /* DisposCCursor */
    { _MixedModeMagic }, /* GetMaxDevice */
    { _MixedModeMagic }, /* GetCTSeed */
    { _MixedModeMagic }, /* GetDeviceList */
    { _MixedModeMagic }, /* GetMainDevice */
    { _MixedModeMagic }, /* GetNextDevice */
    { _MixedModeMagic }, /* TestDeviceAttribute */
    { _MixedModeMagic }, /* SetDeviceAttribute */
    { _MixedModeMagic }, /* InitGDevice */
    { _MixedModeMagic }, /* NewGDevice */
    { _MixedModeMagic }, /* DisposGDevice */
    { _MixedModeMagic }, /* SetGDevice */
    { _MixedModeMagic }, /* GetGDevice */
    { _MixedModeMagic }, /* Color2Index */
    { _MixedModeMagic }, /* Index2Color */
    { _MixedModeMagic }, /* InvertColor */
    { _MixedModeMagic }, /* RealColor */
    { _MixedModeMagic }, /* GetSubTable */
    { _MixedModeMagic }, /* UpdatePixMap */
    { _MixedModeMagic }, /* MakeITable */
    { _MixedModeMagic }, /* AddSearch */
    { _MixedModeMagic }, /* AddComp */
    { _MixedModeMagic }, /* SetClientID */
    { _MixedModeMagic }, /* ProtectEntry */
    { _MixedModeMagic }, /* ReserveEntry */
    { _MixedModeMagic }, /* SetEntries */
    { _MixedModeMagic }, /* QDError */
    { _MixedModeMagic }, /* SetWinColor */
    { _MixedModeMagic }, /* GetAuxWin */
    { _MixedModeMagic }, /* SetCtlColor */
    { _MixedModeMagic }, /* GetAuxCtl */
    { _MixedModeMagic }, /* NewCWindow */
    { _MixedModeMagic }, /* GetNewCWindow */
    { _MixedModeMagic }, /* SetDeskCPat */
    { _MixedModeMagic }, /* GetCWMgrPort */
    { _MixedModeMagic }, /* SaveEntries */
    { _MixedModeMagic }, /* RestoreEntries */
    { _MixedModeMagic }, /* NewCDialog */
    { _MixedModeMagic }, /* DelSearch */
    { _MixedModeMagic }, /* DelComp */
    { _MixedModeMagic }, /* SetStdCProcs */
    { _MixedModeMagic }, /* CalcCMask */
    { _MixedModeMagic }, /* SeedCFill */
    { _MixedModeMagic }, /* CopyDeepMask */
    { _MixedModeMagic }, /* HighLvlFSDispatch */
    { _MixedModeMagic }, /* DictionaryDispatch */
    { _MixedModeMagic }, /* TextServDispatch */
    { _MixedModeMagic }, /* KobeDispatch */
    { _MixedModeMagic }, /* PlainTalk */
    { _MixedModeMagic }, /* DockingDispatch */
    { _MixedModeMagic }, /* NuKernelDispatch */
    { _MixedModeMagic }, /* MixedModeDispatch */
    { _MixedModeMagic }, /* CodeFragDispatch */
    { _MixedModeMagic }, /* RemoteAccess */
    { _MixedModeMagic }, /* OCE_DigSig */
    { _MixedModeMagic }, /* OCE_LetterPacking */
    { _MixedModeMagic }, /* OCE_MailMessage */
    { _MixedModeMagic }, /* OCE_Authentication */
    { _MixedModeMagic }, /* DelMCEntries */
    { _MixedModeMagic }, /* GetMCInfo */
    { _MixedModeMagic }, /* SetMCInfo */
    { _MixedModeMagic }, /* DispMCInfo */
    { _MixedModeMagic }, /* GetMCEntry */
    { _MixedModeMagic }, /* SetMCEntries */
    { _MixedModeMagic }, /* MenuChoice */
    { _MixedModeMagic }, /* ModalDialogMenuSetup */
    { _MixedModeMagic }, /* DialogDispatch */
    { _MixedModeMagic }, /* UserNameNotification */
    { _MixedModeMagic }, /* DeviceDispatch */
    { _MixedModeMagic }, /* PowerPCFuture */
    { _MixedModeMagic }, /* PenMacDispatch */
    { _MixedModeMagic }, /* LanguageMgrDispatch */
    { _MixedModeMagic }, /* AppleGuideDispatch */
    { _MixedModeMagic }, /* Myst_A6F */
    { _MixedModeMagic }, /* Myst_A70 */
    { _MixedModeMagic }, /* Myst_A71 */
    { _MixedModeMagic }, /* Myst_A72 */
    { _MixedModeMagic }, /* ControlDispatch */
    { _MixedModeMagic }, /* AppearanceDispatch */
    { _MixedModeMagic }, /* IconServicesDispatch */
    { _MixedModeMagic }, /* Myst_A76 */
    { _MixedModeMagic }, /* Myst_A77 */
    { _MixedModeMagic }, /* Myst_A78 */
    { _MixedModeMagic }, /* Myst_A79 */
    { _MixedModeMagic }, /* Myst_A7A */
    { _MixedModeMagic }, /* Myst_A7B */
    { _MixedModeMagic }, /* Myst_A7C */
    { _MixedModeMagic }, /* Myst_A7D */
    { _MixedModeMagic }, /* Myst_A7E */
    { _MixedModeMagic }, /* Myst_A7F */
    { _MixedModeMagic }, /* Myst_A80 */
    { _MixedModeMagic }, /* Myst_A81 */
    { _MixedModeMagic }, /* Myst_A82 */
    { _MixedModeMagic }, /* Myst_A83 */
    { _MixedModeMagic }, /* Myst_A84 */
    { _MixedModeMagic }, /* Myst_A85 */
    { _MixedModeMagic }, /* Myst_A86 */
    { _MixedModeMagic }, /* Myst_A87 */
    { _MixedModeMagic }, /* Myst_A88 */
    { _MixedModeMagic }, /* Myst_A89 */
    { _MixedModeMagic }, /* Myst_A8A */
    { _MixedModeMagic }, /* Myst_A8B */
    { _MixedModeMagic }, /* Myst_A8C */
    { _MixedModeMagic }, /* Myst_A8D */
    { _MixedModeMagic }, /* Myst_A8E */
    { _MixedModeMagic }, /* Myst_A8F */
    { _MixedModeMagic }, /* InitPalettes */
    { _MixedModeMagic }, /* NewPalette */
    { _MixedModeMagic }, /* GetNewPalette */
    { _MixedModeMagic }, /* DisposPalette */
    { _MixedModeMagic }, /* ActivatePalette */
    { _MixedModeMagic }, /* SetPalette */
    { _MixedModeMagic }, /* GetPalette */
    { _MixedModeMagic }, /* PmForeColor */
    { _MixedModeMagic }, /* PmBackColor */
    { _MixedModeMagic }, /* AnimateEntry */
    { _MixedModeMagic }, /* AnimatePalette */
    { _MixedModeMagic }, /* GetEntryColor */
    { _MixedModeMagic }, /* SetEntryColor */
    { _MixedModeMagic }, /* GetEntryUsage */
    { _MixedModeMagic }, /* SetEntryUsage */
    { _MixedModeMagic }, /* CTab2Palette */
    { _MixedModeMagic }, /* Palette2CTab */
    { _MixedModeMagic }, /* CopyPalette */
    { _MixedModeMagic }, /* PaletteDispatch */
    { _MixedModeMagic }, /* CodecDispatch */
    { _MixedModeMagic }, /* Myst_AA4 */
    { _MixedModeMagic }, /* Myst_AA5 */
    { _MixedModeMagic }, /* Myst_AA6 */
    { _MixedModeMagic }, /* Myst_AA7 */
    { _MixedModeMagic }, /* Myst_AA8 */
    { _MixedModeMagic }, /* Myst_AA9 */
    { _MixedModeMagic }, /* QTDispatch */
    { _MixedModeMagic }, /* Myst_AAB */
    { _MixedModeMagic }, /* Myst_AAC */
    { _MixedModeMagic }, /* Myst_AAD */
    { _MixedModeMagic }, /* Myst_AAE */
    { _MixedModeMagic }, /* Myst_AAF */
    { _MixedModeMagic }, /* Myst_AB0 */
    { _MixedModeMagic }, /* Myst_AB1 */
    { _MixedModeMagic }, /* Myst_AB2 */
    { _MixedModeMagic }, /* Myst_AB3 */
    { _MixedModeMagic }, /* Myst_AB4 */
    { _MixedModeMagic }, /* Myst_AB5 */
    { _MixedModeMagic }, /* Myst_AB6 */
    { _MixedModeMagic }, /* Myst_AB7 */
    { _MixedModeMagic }, /* Myst_AB8 */
    { _MixedModeMagic }, /* Myst_AB9 */
    { _MixedModeMagic }, /* Myst_ABA */
    { _MixedModeMagic }, /* Myst_ABB */
    { _MixedModeMagic }, /* Myst_ABC */
    { _MixedModeMagic }, /* Myst_ABD */
    { _MixedModeMagic }, /* Myst_ABE */
    { _MixedModeMagic }, /* Myst_ABF */
    { _MixedModeMagic }, /* Myst_AC0 */
    { _MixedModeMagic }, /* Myst_AC1 */
    { _MixedModeMagic }, /* Myst_AC2 */
    { _MixedModeMagic }, /* Myst_AC3 */
    { _MixedModeMagic }, /* Myst_AC4 */
    { _MixedModeMagic }, /* Myst_AC5 */
    { _MixedModeMagic }, /* Myst_AC6 */
    { _MixedModeMagic }, /* Myst_AC7 */
    { _MixedModeMagic }, /* Myst_AC8 */
    { _MixedModeMagic }, /* Myst_AC9 */
    { _MixedModeMagic }, /* Myst_ACA */
    { _MixedModeMagic }, /* Myst_ACB */
    { _MixedModeMagic }, /* Myst_ACC */
    { _MixedModeMagic }, /* Myst_ACD */
    { _MixedModeMagic }, /* Myst_ACE */
    { _MixedModeMagic }, /* Myst_ACF */
    { _MixedModeMagic }, /* Myst_AD0 */
    { _MixedModeMagic }, /* Myst_AD1 */
    { _MixedModeMagic }, /* Myst_AD2 */
    { _MixedModeMagic }, /* Myst_AD3 */
    { _MixedModeMagic }, /* Myst_AD4 */
    { _MixedModeMagic }, /* Myst_AD5 */
    { _MixedModeMagic }, /* Myst_AD6 */
    { _MixedModeMagic }, /* Myst_AD7 */
    { _MixedModeMagic }, /* Myst_AD8 */
    { _MixedModeMagic }, /* Myst_AD9 */
    { _MixedModeMagic }, /* Myst_ADA */
    { _MixedModeMagic }, /* CursorADBDispatch */
    { _MixedModeMagic }, /* Myst_ADC */
    { _MixedModeMagic }, /* HumanInterfaceUtils */
    { _MixedModeMagic }, /* Myst_ADE */
    { _MixedModeMagic }, /* Myst_ADF */
    { _MixedModeMagic }, /* Myst_AE0 */
    { _MixedModeMagic }, /* Myst_AE1 */
    { _MixedModeMagic }, /* Myst_AE2 */
    { _MixedModeMagic }, /* Myst_AE3 */
    { _MixedModeMagic }, /* Myst_AE4 */
    { _MixedModeMagic }, /* Myst_AE5 */
    { _MixedModeMagic }, /* Myst_AE6 */
    { _MixedModeMagic }, /* Myst_AE7 */
    { _MixedModeMagic }, /* Myst_AE8 */
    { _MixedModeMagic }, /* Myst_AE9 */
    { _MixedModeMagic }, /* Myst_AEA */
    { _MixedModeMagic }, /* Myst_AEB */
    { _MixedModeMagic }, /* Myst_AEC */
    { _MixedModeMagic }, /* Myst_AED */
    { _MixedModeMagic }, /* AppleScript */
    { _MixedModeMagic }, /* Myst_AEF */
    { _MixedModeMagic }, /* Myst_AF0 */
    { _MixedModeMagic }, /* ATAManager */
    { _MixedModeMagic }, /* ControlStripMgr */
    { _MixedModeMagic }, /* ExpansionBusDispatch */
    { _MixedModeMagic }, /* InterruptMgrDispatch */
    { _MixedModeMagic }, /* Myst_AF5 */
    { _MixedModeMagic }, /* Myst_AF6 */
    { _MixedModeMagic }, /* Myst_AF7 */
    { _MixedModeMagic }, /* Myst_AF8 */
    { _MixedModeMagic }, /* Myst_AF9 */
    { _MixedModeMagic }, /* InitApplication */
    { _MixedModeMagic }, /* CleanupApplication */
    { _MixedModeMagic }, /* Myst_AFC */
    { _MixedModeMagic }, /* Myst_AFD */
    { _MixedModeMagic }, /* MixedModeMagic */
    { _MixedModeMagic }, /* Myst_AFF */
    { _MixedModeMagic }, /* qd_BitBlt */
    { _MixedModeMagic }, /* qd_BitsToMap */
    { _MixedModeMagic }, /* qd_BitsToPix */
    { _MixedModeMagic }, /* QD32Trap */
    { _MixedModeMagic }, /* qd_ColorMap */
    { _MixedModeMagic }, /* qd_CopyHandle */
    { _MixedModeMagic }, /* qd_CullPoints */
    { _MixedModeMagic }, /* qd_DputpicByte */
    { _MixedModeMagic }, /* qd_DputPicop */
    { _MixedModeMagic }, /* qd_DrawArc */
    { _MixedModeMagic }, /* qd_DrawLine */
    { _MixedModeMagic }, /* qd_DrawSlab */
    { _MixedModeMagic }, /* qd_FastSlabMode */
    { _MixedModeMagic }, /* qd_GetSeek */
    { _MixedModeMagic }, /* qd_MakeScaleTbl */
    { _MixedModeMagic }, /* qd_Checkpic */
    { _MixedModeMagic }, /* qd_Line */
    { _MixedModeMagic }, /* qd_OldPatToNew */
    { _MixedModeMagic }, /* qd_PackRgn */
    { _MixedModeMagic }, /* qd_PatConvert */
    { _MixedModeMagic }, /* qd_PatDither */
    { _MixedModeMagic }, /* qd_PatExpand */
    { _MixedModeMagic }, /* qd_Pinit */
    { _MixedModeMagic }, /* qd_PortToMap */
    { _MixedModeMagic }, /* qd_PushVerb */
    { _MixedModeMagic }, /* qd_PutLine */
    { _MixedModeMagic }, /* qd_PutOval */
    { _MixedModeMagic }, /* qd_PutRgn */
    { _MixedModeMagic }, /* NewTempBuffer */
    { _MixedModeMagic }, /* QDOffscreen */
    { _MixedModeMagic }, /* DisposeTempBuffer */
    { _MixedModeMagic }, /* qd_RgnBlt */
    { _MixedModeMagic }, /* qd_RgnOp */
    { _MixedModeMagic }, /* qd_Rsect */
    { _MixedModeMagic }, /* qd_SeekRgn */
    { _MixedModeMagic }, /* qd_SetFillPat */
    { _MixedModeMagic }, /* qd_SetupStretch */
    { _MixedModeMagic }, /* qd_SlabMode */
    { _MixedModeMagic }, /* qd_SortPoints */
    { _MixedModeMagic }, /* qd_StretchBits */
    { _MixedModeMagic }, /* qd_StdDevLoop */
    { _MixedModeMagic }, /* qd_TrimRect */
    { _MixedModeMagic }, /* qd_XorSlab */
    { _MixedModeMagic }, /* Myst_B2B */
    { _MixedModeMagic }, /* Myst_B2C */
    { _MixedModeMagic }, /* NewTempHandle */
    { _MixedModeMagic }, /* Myst_B2E */
    { _MixedModeMagic }, /* Myst_B2F */
    { _MixedModeMagic }, /* qd_BMain0 */
    { _MixedModeMagic }, /* qd_BMain1 */
    { _MixedModeMagic }, /* qd_BMain2 */
    { _MixedModeMagic }, /* qd_BMain3 */
    { _MixedModeMagic }, /* qd_Bsetup8 */
    { _MixedModeMagic }, /* qd_BMain9 */
    { _MixedModeMagic }, /* qd_Bsetup10 */
    { _MixedModeMagic }, /* qd_BMain11 */
    { _MixedModeMagic }, /* qd_BXMain8 */
    { _MixedModeMagic }, /* qd_BXMain9 */
    { _MixedModeMagic }, /* qd_BXMain10 */
    { _MixedModeMagic }, /* qd_Bxmain11 */
    { _MixedModeMagic }, /* qd_BCMain0 */
    { _MixedModeMagic }, /* qd_BCMain1 */
    { _MixedModeMagic }, /* qd_BHilite */
    { _MixedModeMagic }, /* qd_BCMain3 */
    { _MixedModeMagic }, /* qd_BEnd0 */
    { _MixedModeMagic }, /* qd_BEnd1 */
    { _MixedModeMagic }, /* qd_BEnd2 */
    { _MixedModeMagic }, /* qd_BEnd3 */
    { _MixedModeMagic }, /* qd_BLong8 */
    { _MixedModeMagic }, /* qd_BEnd9 */
    { _MixedModeMagic }, /* qd_BEnd10 */
    { _MixedModeMagic }, /* qd_BEnd11 */
    { _MixedModeMagic }, /* qd_BXLong8 */
    { _MixedModeMagic }, /* qd_BXEnd9 */
    { _MixedModeMagic }, /* qd_BXEnd10 */
    { _MixedModeMagic }, /* qd_BXEnd11 */
    { _MixedModeMagic }, /* qd_BCEnd0 */
    { _MixedModeMagic }, /* qd_BCEnd1 */
    { _MixedModeMagic }, /* qd_BSloHilite */
    { _MixedModeMagic }, /* qd_BCEnd3 */
    { _MixedModeMagic }, /* qd_BAvg */
    { _MixedModeMagic }, /* qd_BAddPin */
    { _MixedModeMagic }, /* qd_BAddOver */
    { _MixedModeMagic }, /* qd_BSubPin */
    { _MixedModeMagic }, /* qd_BTransparent */
    { _MixedModeMagic }, /* qd_BMax */
    { _MixedModeMagic }, /* qd_BSubOver */
    { _MixedModeMagic }, /* qd_BMin */
    { _MixedModeMagic }, /* qd_Bsetup0 */
    { _MixedModeMagic }, /* qd_BLeft0 */
    { _MixedModeMagic }, /* qd_Rmask0 */
    { _MixedModeMagic }, /* qd_Rmask1 */
    { _MixedModeMagic }, /* qd_Rmask2 */
    { _MixedModeMagic }, /* qd_Rmask3 */
    { _MixedModeMagic }, /* qd_Rmask8 */
    { _MixedModeMagic }, /* qd_Rmask9 */
    { _MixedModeMagic }, /* qd_Rmask10 */
    { _MixedModeMagic }, /* qd_Rmask11 */
    { _MixedModeMagic }, /* qd_RXmask8 */
    { _MixedModeMagic }, /* qd_RXmask9 */
    { _MixedModeMagic }, /* qd_RXmask10 */
    { _MixedModeMagic }, /* qd_RXmask11 */
    { _MixedModeMagic }, /* qd_RAvg */
    { _MixedModeMagic }, /* qd_RAddPin */
    { _MixedModeMagic }, /* qd_RAddOver */
    { _MixedModeMagic }, /* qd_RSubPin */
    { _MixedModeMagic }, /* qd_RTransparent */
    { _MixedModeMagic }, /* qd_RMax */
    { _MixedModeMagic }, /* qd_RSubOver */
    { _MixedModeMagic }, /* qd_RMin */
    { _MixedModeMagic }, /* qd_RCmask0 */
    { _MixedModeMagic }, /* qd_RCmask1 */
    { _MixedModeMagic }, /* qd_RSlowHilite */
    { _MixedModeMagic }, /* qd_RCmask3 */
    { _MixedModeMagic }, /* qd_RHilite */
    { _MixedModeMagic }, /* qd_StMask0 */
    { _MixedModeMagic }, /* qd_StMask1 */
    { _MixedModeMagic }, /* qd_StMask2 */
    { _MixedModeMagic }, /* qd_StMask3 */
    { _MixedModeMagic }, /* qd_StAvg */
    { _MixedModeMagic }, /* qd_StAddPin */
    { _MixedModeMagic }, /* qd_StAddOver */
    { _MixedModeMagic }, /* qd_StSubPin */
    { _MixedModeMagic }, /* qd_StTransparent */
    { _MixedModeMagic }, /* qd_StMax */
    { _MixedModeMagic }, /* qd_StSubOver */
    { _MixedModeMagic }, /* qd_StMin */
    { _MixedModeMagic }, /* qd_StHilite */
    { _MixedModeMagic }, /* qd_SlMask8 */
    { _MixedModeMagic }, /* qd_SlMask9 */
    { _MixedModeMagic }, /* qd_SlMask10 */
    { _MixedModeMagic }, /* qd_SlMask11 */
    { _MixedModeMagic }, /* qd_SlxMask8 */
    { _MixedModeMagic }, /* qd_SlxMask9 */
    { _MixedModeMagic }, /* qd_SlxMask10 */
    { _MixedModeMagic }, /* qd_SlxMask11 */
    { _MixedModeMagic }, /* qd_SlAvg */
    { _MixedModeMagic }, /* qd_SlAddPin */
    { _MixedModeMagic }, /* qd_SlAddOver */
    { _MixedModeMagic }, /* qd_SlSubPin */
    { _MixedModeMagic }, /* qd_SlTransparent */
    { _MixedModeMagic }, /* qd_SlMax */
    { _MixedModeMagic }, /* qd_SlSubOver */
    { _MixedModeMagic }, /* qd_SlMin */
    { _MixedModeMagic }, /* qd_SlHilite */
    { _MixedModeMagic }, /* qd_ItabMatch */
    { _MixedModeMagic }, /* qd_Colorthing2Index */
    { _MixedModeMagic }, /* Myst_B93 */
    { _MixedModeMagic }, /* qd_AllocRunbuf */
    { _MixedModeMagic }, /* qd_InitRgn */
    { _MixedModeMagic }, /* qd_Scaleblt */
    { _MixedModeMagic }, /* qd_StnoStack */
    { _MixedModeMagic }, /* qd_Blitcase */
    { _MixedModeMagic }, /* qd_Stscanloop */
    { _MixedModeMagic }, /* qd_Picitem1 */
    { _MixedModeMagic }, /* qd_MakeGrayItab */
    { _MixedModeMagic }, /* qd_FastLine */
    { _MixedModeMagic }, /* qd_FastSlant */
    { _MixedModeMagic }, /* qd_BitsDevloop */
    { _MixedModeMagic }, /* Myst_B9F */
    { _MixedModeMagic }, /* Myst_BA0 */
    { _MixedModeMagic }, /* Myst_BA1 */
    { _MixedModeMagic }, /* Myst_BA2 */
    { _MixedModeMagic }, /* Myst_BA3 */
    { _MixedModeMagic }, /* Myst_BA4 */
    { _MixedModeMagic }, /* Myst_BA5 */
    { _MixedModeMagic }, /* Myst_BA6 */
    { _MixedModeMagic }, /* Myst_BA7 */
    { _MixedModeMagic }, /* qd_Stcolortab */
    { _MixedModeMagic }, /* qd_Stgraytab */
    { _MixedModeMagic }, /* qd_StSearchtab */
    { _MixedModeMagic }, /* qd_ScaleIndxd2Indxd */
    { _MixedModeMagic }, /* Myst_BAC */
    { _MixedModeMagic }, /* Myst_BAD */
    { _MixedModeMagic }, /* Myst_BAE */
    { _MixedModeMagic }, /* Myst_BAF */
    { _MixedModeMagic }, /* Myst_BB0 */
    { _MixedModeMagic }, /* Myst_BB1 */
    { _MixedModeMagic }, /* Myst_BB2 */
    { _MixedModeMagic }, /* Myst_BB3 */
    { _MixedModeMagic }, /* Myst_BB4 */
    { _MixedModeMagic }, /* Myst_BB5 */
    { _MixedModeMagic }, /* Myst_BB6 */
    { _MixedModeMagic }, /* Myst_BB7 */
    { _MixedModeMagic }, /* Myst_BB8 */
    { _MixedModeMagic }, /* Myst_BB9 */
    { _MixedModeMagic }, /* Myst_BBA */
    { _MixedModeMagic }, /* qd_Barith16Setup */
    { _MixedModeMagic }, /* qd_Barith32Setup */
    { _MixedModeMagic }, /* Myst_BBD */
    { _MixedModeMagic }, /* Myst_BBE */
    { _MixedModeMagic }, /* Myst_BBF */
    { _MixedModeMagic }, /* qd_AlphaDispatch */
    { _MixedModeMagic }, /* qd_StreamToMask */
    { _MixedModeMagic }, /* QT_MatrixMathDispatch */
    { _MixedModeMagic }, /* NQD_Misc */
    { _MixedModeMagic }, /* qd_GetpmData */
    { _MixedModeMagic }, /* Myst_BC5 */
    { _MixedModeMagic }, /* Myst_BC6 */
    { _MixedModeMagic }, /* Myst_BC7 */
    { _MixedModeMagic }, /* Myst_BC8 */
    { _MixedModeMagic }, /* IconDispatch */
    { _MixedModeMagic }, /* DeviceLoop */
    { _MixedModeMagic }, /* Myst_BCB */
    { _MixedModeMagic }, /* Myst_BCC */
    { _MixedModeMagic }, /* Myst_BCD */
    { _MixedModeMagic }, /* UnicodeDispatch */
    { _MixedModeMagic }, /* ProcessMgrDispatch */
    { _MixedModeMagic }, /* Myst_BD0 */
    { _MixedModeMagic }, /* Myst_BD1 */
    { _MixedModeMagic }, /* Myst_BD2 */
    { _MixedModeMagic }, /* Myst_BD3 */
    { _MixedModeMagic }, /* Myst_BD4 */
    { _MixedModeMagic }, /* Myst_BD5 */
    { _MixedModeMagic }, /* Myst_BD6 */
    { _MixedModeMagic }, /* Myst_BD7 */
    { _MixedModeMagic }, /* Myst_BD8 */
    { _MixedModeMagic }, /* Myst_BD9 */
    { _MixedModeMagic }, /* Myst_BDA */
    { _MixedModeMagic }, /* Myst_BDB */
    { _MixedModeMagic }, /* Myst_BDC */
    { _MixedModeMagic }, /* Myst_BDD */
    { _MixedModeMagic }, /* Myst_BDE */
    { _MixedModeMagic }, /* Myst_BDF */
    { _MixedModeMagic }, /* Myst_BE0 */
    { _MixedModeMagic }, /* Myst_BE1 */
    { _MixedModeMagic }, /* Myst_BE2 */
    { _MixedModeMagic }, /* Myst_BE3 */
    { _MixedModeMagic }, /* Myst_BE4 */
    { _MixedModeMagic }, /* Myst_BE5 */
    { _MixedModeMagic }, /* Myst_BE6 */
    { _MixedModeMagic }, /* Myst_BE7 */
    { _MixedModeMagic }, /* Myst_BE8 */
    { _MixedModeMagic }, /* Myst_BE9 */
    { _MixedModeMagic }, /* ModemMgr */
    { _MixedModeMagic }, /* DisplayMgr */
    { _MixedModeMagic }, /* ButtonMgr */
    { _MixedModeMagic }, /* DragDispatch */
    { _MixedModeMagic }, /* ColorMatch */
    { _MixedModeMagic }, /* TTSMgr */
    { _MixedModeMagic }, /* AROSE */
    { _MixedModeMagic }, /* GestaltValueDispatch */
    { _MixedModeMagic }, /* ThreadDispatch */
    { _MixedModeMagic }, /* EddyTrap */
    { _MixedModeMagic }, /* XTNDMgr */
    { _MixedModeMagic }, /* DSPManager */
    { _MixedModeMagic }, /* CollectionMgr */
    { _MixedModeMagic }, /* SynchIdleTime */
    { _MixedModeMagic }, /* StdOpcodeProc */
    { _MixedModeMagic }, /* AuxDispatch */
    { _MixedModeMagic }, /* Myst_BFA */
    { _MixedModeMagic }, /* GX_Message_Mgr */
    { _MixedModeMagic }, /* TranslationMgr */
    { _MixedModeMagic }, /* Myst_BFD */
    { _MixedModeMagic }  /* GXPrint_Mgr */
};


/* 0x400 */
static ProcPtr OSTable[0x100];

/* 0xE00 */
static ProcPtr TBTrapTbl[0x400];


void
C14InitTraps(void)
{
    UInt16 trapNum;
    
    for (trapNum = 0; trapNum < 0x100; ++trapNum) {
        if (osTrap[trapNum].routine == (ProcPtr)&C14Unimplemented) {
            OSTable[trapNum] = (ProcPtr)&tbTrap[_Unimplemented & 0x3ff].goMixedModeTrap;
        } else {
            OSTable[trapNum] = (ProcPtr)&osTrap[trapNum].goMixedModeTrap;
            if (!osTrap[trapNum].routine) {
                osTrap[trapNum].flags = C14MM_V_V_DESC;
                osTrap[trapNum].routine = (ProcPtr)&C14Unimplemented;
            }
        }
    }
    
    for (trapNum = 0; trapNum < 0x400; ++trapNum) {
        if (tbTrap[trapNum].routine == (ProcPtr)&C14Unimplemented) {
            TBTrapTbl[trapNum] = (ProcPtr)&tbTrap[_Unimplemented & 0x3ff].goMixedModeTrap;
        } else {
            TBTrapTbl[trapNum] = (ProcPtr)&tbTrap[trapNum].goMixedModeTrap;
            if (!tbTrap[trapNum].routine) {
                tbTrap[trapNum].flags = C14MM_V_V_DESC;
                tbTrap[trapNum].routine = (ProcPtr)&C14Unimplemented;
            }
        }
    }
}


UniversalProcPtr
C14GetTrapAddress(UInt16 trapWord)
{
    UInt16 trapNum;
    ProcPtr routine;
    
    if (trapWord & 0x800) {
        trapNum = trapWord & 0x3ff;
        routine = TBTrapTbl[trapNum];
    } else {
        trapNum = trapWord & 0xff;
        routine = OSTable[trapNum];
    }
    
    return (UniversalProcPtr)routine;
}


void
C14SetTrapAddress(
  UniversalProcPtr   trapAddr,
  UInt16             trapWord)
{
    UInt16 trapNum;
    
    if (trapWord & 0x800) {
        trapNum = trapWord & 0x3ff;
        TBTrapTbl[trapNum] = (ProcPtr)trapAddr;
    } else {
        trapNum = trapWord & 0xff;
        OSTable[trapNum] = (ProcPtr)trapAddr;
    }
}


static void
findRoutineDescriptor(C14RoutineDescriptor *desc,
                      UInt16 *pTrapNum, const char **pTrapName)
{
    UInt16 trapNum;
    
    *pTrapNum = 0;
    *pTrapName = "???";
    
    if (&osTrap[0] <= desc && desc < &osTrap[0x100]) {
        trapNum = desc - &osTrap[0];
        *pTrapNum = trapNum;
        *pTrapName = osTrapName[trapNum];
        return;
    }
    
    if (&tbTrap[0] <= desc && desc < &tbTrap[0x400]) {
        trapNum = desc - &tbTrap[0];
        *pTrapNum = trapNum;
        *pTrapName = tbTrapName[trapNum];
        return;
    }
}

void
C14Unimplemented(C14RoutineDescriptor *desc)
{
    UInt16 trapNum = 0; const char *trapName = "xxx";
    static Str255 explanation;
    char *buffer;
    SInt16 itemHit;

    findRoutineDescriptor(desc, &trapNum, &trapName);
    
    buffer = (char *)&explanation[1];
    sprintf(buffer,
            "C-14 encountered an unimplemented trap. "
            "The application must now quit."
            "\n\n"
            "%lx %s", trapNum, trapName);
    explanation[0] = strlen(buffer);
    
    StandardAlert(
        kAlertDefaultOKText,
        "\pUnimplemented Trap",
        explanation,
        nil,
        &itemHit);
    ExitToShell();
}


void
C14PrivateTrapDispatcher(UInt16 trapWord, UInt32 regs[16])
{
    UInt16 trapNum;
    UInt32 pc; UInt16 *sp;
    C14RoutineDescriptor *desc;
    ProcPtr routine;
    const char *trapName; /* XXX: debug */
    
    pc = C14M68KGetPC();
    
    if (trapWord == _MixedModeMagic) {
        desc = (C14RoutineDescriptor *)(pc - 2);
        C14MixedModeMagic(desc, regs);
        return;
    }

    /* push return address */
#define SP regs[8+7]
    SP -= 4;
    sp = (UInt16 *)SP;
    sp[0] = HiWord(pc);
    sp[1] = LoWord(pc);
    
    /* jump to the routine */
    if (trapWord & 0x800) {
        trapNum = trapWord & 0x3ff;
        routine = TBTrapTbl[trapNum];
        trapName = tbTrapName[trapNum];
    } else {
        trapNum = trapWord & 0xff;
        routine = OSTable[trapNum];
        trapName = osTrapName[trapNum];
    }
    C14M68KSetPC((UInt32)routine);
}
