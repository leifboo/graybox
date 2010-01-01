
#include "C14SegLoad.h"

#include "C14LowMem.h"
#include "C14M68K.h"
#include "C14Traps.h"


#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif


/*
 * Jump Table Entry
 * IM II-61
 */
typedef struct JumpTableEntry {
    UInt16 codeOffsetOrSegNum;
    UInt16 code[3];
} JumpTableEntry;


/*
 * Code Segment
 * IM II-61
 */
typedef struct CodeSegment {
    UInt16 firstJTEOffset;
    UInt16 numEntries;
    UInt16 code[1];
} CodeSegment;


/*
 * CODE segment 0
 * IM II-62
 */
typedef struct CODE0 {
    UInt32 aboveA5;
    UInt32 belowA5;
    UInt32 jtLength;
    UInt32 jtOffset;
    JumpTableEntry jumpTable[1];
} CODE0;


#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif



static JumpTableEntry *jumpTable;


void
C14LoadSeg(UInt16 segNum, UInt32 *caller)
{
    CodeSegment **segHandle, *seg;
    JumpTableEntry *jt;
    UInt16 i;
    Ptr base, routine;
    
    /* load the code segment */
    segHandle = (CodeSegment **)GetResource('CODE', segNum);
    seg = *segHandle;
    
    /* change its jump table entries to the "loaded" state */
    jt = (JumpTableEntry *)((Ptr)jumpTable + seg->firstJTEOffset);
    base = (Ptr)seg + offsetof(CodeSegment, code);
    for (i = 0; i < seg->numEntries; ++i) {
        routine = base + jt[i].codeOffsetOrSegNum;
        jt[i].codeOffsetOrSegNum = segNum;
        jt[i].code[0] = 0x4EF9; /* JMP absolute */
        jt[i].code[1] = HiWord(routine);
        jt[i].code[2] = LoWord(routine);
    }
    
    /* jump to the target routine
       by tweaking the return address */
    *caller -= 6;
}


DEFINE_API( void )
C14ClassicM68KApplicationMain(void)
{
    CODE0 **code0Handle, *code0;
    Ptr a5World, a5, sp; UInt32 stackSize;
    
    code0Handle = (CODE0 **)GetResource('CODE', 0);
    code0 = *code0Handle;
    
    /*
     * Create the application's A5 world.
     * See "Memory Organization", IM II-19
     * and "The Jump Table", IM II pp. 60-61.
     *
     * +------------------------+ <- A5 + aboveA5 == A5 + jtOffset + jtLength
     * | jump table             |
     * +------------------------+ <- A5 + jtOffset == A5 + 32
     * | application parameters |
     * +------------------------+ <- A5
     * | application globals    |
     * +------------------------+
     * | QuickDraw globals      |
     * +------------------------+ <- initial SP (A7) == A5 - belowA5
     * | stack                  |
     *
     */

    stackSize = 0x10000; /* 64K -- XXX: allow it to grow */
    a5World = NewPtr(stackSize + code0->belowA5 + code0->aboveA5);
    sp = a5World + stackSize;
    a5 = sp + code0->belowA5;
    jumpTable = (JumpTableEntry *)(a5 + code0->jtOffset);
    
    /*
     * Initialize the jump table.
     */
    BlockMove(&code0->jumpTable, jumpTable, code0->jtLength);
    
    /*
     * Initialize low memory & the Trap Dispatcher.
     */
    C14InitLowMem(a5);
    C14InitTraps();
    
    /*
     * Start the M68K emulator.
     */
	C14M68KStart((Ptr)&jumpTable[0].code, sp, a5);
}
