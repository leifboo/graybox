
#ifndef __C14M68K_h__
#define __C14M68K_h__


void
C14M68KMapMemory(UInt32 virtualPtr, Ptr ptr, Size size);

void
C14M68KStart(Ptr pc, Ptr sp);

void
C14M68KStop(void);

UInt32 *
C14M68KRegisters(void);

UInt32
C14M68KGetPC(void);

void
C14M68KSetPC(UInt32 newPC);


#endif /* __C14M68K_h__ */
