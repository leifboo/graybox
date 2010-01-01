
#ifndef __C14LowMem_h__
#define __C14LowMem_h__


void
C14InitLowMem(Ptr a5);

UInt32
C14LowMemAccess(UInt32 data, Boolean write, Boolean byte, UInt32 addr);


#endif /* __C14LowMem_h__ */
