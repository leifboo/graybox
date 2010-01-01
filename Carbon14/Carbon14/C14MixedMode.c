
#include "C14MixedMode.h"

#include "C14M68K.h"


typedef UInt16 W;
typedef UInt32 L;

typedef void (*V_V)(void);
typedef void (*V_V_DESC)(C14RoutineDescriptor *);
typedef void (*V_W)(W);
typedef void (*V_W_C)(W, L *);
typedef void (*V_WW)(W, W);
typedef void (*V_L)(L);
typedef void (*V_LW)(L, W);
typedef void (*V_LWW)(L, W, W);
typedef void (*V_LWWWW)(L, W, W, W, W);
typedef void (*V_LL)(L, L);
typedef void (*V_LLL)(L, L, L);
typedef W (*W_V)(void);
typedef W (*W_WL)(W, L);
typedef W (*W_WLLL)(W, L, L, L);
typedef W (*W_L)(L);
typedef W (*W_LL)(L, L);
typedef W (*W_LLL)(L, L, L);
typedef L (*L_V)(void);
typedef L (*L_W)(W);
typedef L (*L_WLL)(W, L, L);
typedef L (*L_L)(L);
typedef L (*L_LW)(L, W);

typedef void (*REGS)(UInt32 regs[16]);


void
C14MixedModeMagic(C14RoutineDescriptor *desc, UInt32 regs[16])
{
#define SP regs[8+7]
#define FetchBoolean(i) (sp[i] >> 8)
#define FetchLong(i) (((L)sp[i] << 16) | (L)sp[i+1])
    W *sp = (W *)SP;
    L caller = FetchLong(0);
    
    sp += 2; /* skip return address */
    
    switch (desc->flags) {
    
    /*
     * stack-based routines
     */
    case C14MM_V_V: {
        V_V routine = (V_V)desc->routine;
        (*routine)();
        break; }
        
    case C14MM_V_V_DESC: {
        V_V_DESC routine = (V_V_DESC)desc->routine;
        (*routine)(desc);
        break; }
        
    case C14MM_V_W: {
        V_W routine = (V_W)desc->routine;
        W arg0 = sp[0];
        (*routine)(arg0);
        SP += 2;
        break; }
        
    case C14MM_V_W_CALLER: {
        V_W_C routine = (V_W_C)desc->routine;
        W arg0 = sp[0];
        (*routine)(arg0, &caller);
        SP += 2;
        break; }
        
    case C14MM_V_WW: {
        V_WW routine = (V_WW)desc->routine;
        W arg0 = sp[1];
        W arg1 = sp[0];
        (*routine)(arg0, arg1);
        SP += 4;
        break; }
    
    case C14MM_V_L: {
        V_L routine = (V_L)desc->routine;
        L arg0 = FetchLong(0);
        (*routine)(arg0);
        SP += 4;
        break; }
        
    case C14MM_V_LW: {
        V_LW routine = (V_LW)desc->routine;
        L arg0 = FetchLong(1);
        W arg1 = sp[0];
        (*routine)(arg0, arg1);
        SP += 6;
        break; }
    
    case C14MM_V_LWB: {
        V_LWW routine = (V_LWW)desc->routine;
        L arg0 = FetchLong(2);
        W arg1 = sp[1];
        W arg2 = FetchBoolean(0);
        (*routine)(arg0, arg1, arg2);
        SP += 8;
        break; }
    
    case C14MM_V_LWWWW: {
        V_LWWWW routine = (V_LWWWW)desc->routine;
        L arg0 = FetchLong(4);
        W arg1 = sp[3];
        W arg2 = sp[2];
        W arg3 = sp[1];
        W arg4 = sp[0];
        (*routine)(arg0, arg1, arg2, arg3, arg4);
        SP += 12;
        break; }
    
    case C14MM_V_LL: {
        V_LL routine = (V_LL)desc->routine;
        L arg0 = FetchLong(2);
        L arg1 = FetchLong(0);
        (*routine)(arg0, arg1);
        SP += 8;
        break; }
    
    case C14MM_V_LLL: {
        V_LLL routine = (V_LLL)desc->routine;
        L arg0 = FetchLong(4);
        L arg1 = FetchLong(2);
        L arg2 = FetchLong(0);
        (*routine)(arg0, arg1, arg2);
        SP += 12;
        break; }
    
    case C14MM_W_V:
    case C14MM_B_V: {
        W_V routine = (W_V)desc->routine;
        sp[0] = (*routine)();
        if (desc->flags == C14MM_B_V)
            sp[0] <<= 8;
        break; }
    
    case C14MM_W_WL:
    case C14MM_B_WL: {
        W_WL routine = (W_WL)desc->routine;
        W arg0 = sp[2];
        L arg1 = FetchLong(0);
        sp[3] = (*routine)(arg0, arg1);
        if (desc->flags == C14MM_B_WL)
            sp[3] <<= 8;
        SP += 6;
        break; }
    
    case C14MM_W_WLLL:
    case C14MM_B_WLLL: {
        W_WLLL routine = (W_WLLL)desc->routine;
        W arg0 = sp[6];
        L arg1 = FetchLong(4);
        L arg2 = FetchLong(2);
        L arg3 = FetchLong(0);
        sp[7] = (*routine)(arg0, arg1, arg2, arg3);
        if (desc->flags == C14MM_B_WLLL)
            sp[7] <<= 8;
        SP += 14;
        break; }
    
    case C14MM_W_L:
    case C14MM_B_L: {
        W_L routine = (W_L)desc->routine;
        L arg0 = FetchLong(0);
        sp[2] = (*routine)(arg0);
        if (desc->flags == C14MM_B_L)
            sp[2] <<= 8;
        SP += 4;
        break; }
    
    case C14MM_W_LL:
    case C14MM_B_LL: {
        W_LL routine = (W_LL)desc->routine;
        L arg0 = FetchLong(2);
        L arg1 = FetchLong(0);
        sp[4] = (*routine)(arg0, arg1);
        if (desc->flags == C14MM_B_LL)
            sp[4] <<= 8;
        SP += 8;
        break; }
    
    case C14MM_W_LLL: {
        W_LLL routine = (W_LLL)desc->routine;
        L arg0 = FetchLong(4);
        L arg1 = FetchLong(2);
        L arg2 = FetchLong(0);
        sp[6] = (*routine)(arg0, arg1, arg2);
        SP += 12;
        break; }
    
    case C14MM_L_V: {
        L_V routine = (L_V)desc->routine;
        L result = (*routine)();
        sp[0] = HiWord(result);
        sp[1] = LoWord(result);
        break; }
    
    case C14MM_L_W: {
        L_W routine = (L_W)desc->routine;
        W arg0 = sp[0];
        L result = (*routine)(arg0);
        sp[1] = HiWord(result);
        sp[2] = LoWord(result);
        SP += 2;
        break; }
    
    case C14MM_L_WLL: {
        L_WLL routine = (L_WLL)desc->routine;
        W arg0 = sp[4];
        L arg1 = FetchLong(2);
        L arg2 = FetchLong(0);
        L result = (*routine)(arg0, arg1, arg2);
        sp[5] = HiWord(result);
        sp[6] = LoWord(result);
        SP += 10;
        break; }
    
    case C14MM_L_L: {
        L_L routine = (L_L)desc->routine;
        L arg0 = FetchLong(0);
        L result = (*routine)(arg0);
        sp[2] = HiWord(result);
        sp[3] = LoWord(result);
        SP += 4;
        break; }
    
    case C14MM_L_LW: {
        L_LW routine = (L_LW)desc->routine;
        L arg0 = FetchLong(1);
        W arg1 = sp[0];
        L result = (*routine)(arg0, arg1);
        sp[3] = HiWord(result);
        sp[4] = LoWord(result);
        SP += 6;
        break; }
    
    /*
     * register-based routines
     */
    /*
     * XXX: set condition codes based upon D0 -- see IM I-94
     */
    case C14MM_V_L_A0: {
        V_L routine = (V_L)desc->routine;
        (*routine)(regs[8+0]);
        break; }
        
    case C14MM_V_LW_A0_D0: {
        V_LW routine = (V_LW)desc->routine;
        (*routine)(regs[8+0], regs[0] & 0xffff);
        break; }
    
    case C14MM_V_LLL_A0_A1_D0: {
        V_LLL routine = (V_LLL)desc->routine;
        (*routine)(regs[8+0], regs[8+1], regs[0]);
        break; }
    
    case C14MM_W_WL_D0_D0_A0: {
        W_WL routine = (W_WL)desc->routine;
        regs[0] = (*routine)(regs[0] & 0xffff, regs[8+0]);
        break; }
    
    case C14MM_W_LL_D0_D0_A1: {
        W_LL routine = (W_LL)desc->routine;
        regs[0] = (*routine)(regs[0], regs[8+1]);
        break; }
    
    case C14MM_L_V_D0: {
        L_V routine = (L_V)desc->routine;
        regs[0] = (*routine)();
        break; }
    
    case C14MM_L_W_A0_D0: {
        L_W routine = (L_W)desc->routine;
        regs[8+0] = (*routine)(regs[0] & 0xffff);
        break; }
        
    case C14MM_L_L_A0_D0: {
        L_L routine = (L_L)desc->routine;
        regs[8+0] = (*routine)(regs[0]);
        break; }
    
    case C14MM_L_L_D0_A0: {
        L_L routine = (L_L)desc->routine;
        regs[0] = (*routine)(regs[8+0]);
        break; }
    
    case C14MM_L_L_D0_D0: {
        L_L routine = (L_L)desc->routine;
        regs[0] = (*routine)(regs[0]);
        break; }
    
    case C14MM_REGS: {
        /* This path is for odd ducks. */
        REGS routine = (REGS)desc->routine;
        (*routine)(regs);
        break; }
    }
    
    /* return to caller */
    C14M68KSetPC(caller);
    SP += 4;
}
