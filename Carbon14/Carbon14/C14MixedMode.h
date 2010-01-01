
#ifndef __C14MixedMode_h__
#define __C14MixedMode_h__


enum {
    /* stack-based routines */
    C14MM_V_V,
    C14MM_V_V_DESC,
    C14MM_V_W,
    C14MM_V_W_CALLER,
    C14MM_V_WW,
    C14MM_V_L,
    C14MM_V_LW,
    C14MM_V_LWB,
    C14MM_V_LWWWW,
    C14MM_V_LL,
    C14MM_V_LLL,
    C14MM_W_V,      C14MM_B_V,
    C14MM_W_L,      C14MM_B_L,
    C14MM_W_WL,     C14MM_B_WL,
    C14MM_W_WLLL,   C14MM_B_WLLL,
    C14MM_W_LL,     C14MM_B_LL,
    C14MM_W_LLL,
    C14MM_L_V,
    C14MM_L_W,
    C14MM_L_WLL,
    C14MM_L_L,
    C14MM_L_LW,
    
    /* register-based routines */
    C14MM_V_L_A0,
    C14MM_V_LW_A0_D0,
    C14MM_V_LLL_A0_A1_D0,
    C14MM_W_WL_D0_D0_A0,
    C14MM_W_LL_D0_D0_A1,
    C14MM_L_V_D0,
    C14MM_L_W_A0_D0,
    C14MM_L_L_A0_D0,
    C14MM_L_L_D0_A0,
    C14MM_L_L_D0_D0,
    
    C14MM_REGS
};


typedef struct C14RoutineDescriptor {
    UInt16 goMixedModeTrap; /* _MixedModeMagic */
    unsigned int flags;
    ProcPtr routine;
} C14RoutineDescriptor;


void
C14MixedModeMagic(C14RoutineDescriptor *desc, UInt32 regs[16]);


#endif /* __C14MixedMode_h__ */
