
#ifndef __C14Traps_h__
#define __C14Traps_h__


struct C14RoutineDescriptor;


void
C14InitTraps(void);

UniversalProcPtr
C14GetTrapAddress(UInt16 trapNum);

void
C14SetTrapAddress(
  UniversalProcPtr   trapAddr,
  UInt16             trapNum);

void
C14Unimplemented(struct C14RoutineDescriptor *desc);

void
C14PrivateTrapDispatcher(UInt16 trapWord, UInt32 regs[16]);


#endif /* __C14Traps_h__ */
