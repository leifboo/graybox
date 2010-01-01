
#ifndef __C14M68K_h__
#define __C14M68K_h__


#include "C14Macros.h"
#include "C14Types.h"


void
C14M68KStart(Ptr pc, Ptr sp, Ptr a5);

UInt32
C14M68KGetPC(void);

void
C14M68KSetPC(UInt32 newPC);


#endif /* __C14M68K_h__ */
