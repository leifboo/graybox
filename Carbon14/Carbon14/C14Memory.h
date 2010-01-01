
#ifndef __C14Memory_h__
#define __C14Memory_h__


#include "C14Macros.h"


C14_API( OSErr )
C14MaxApplZone(void);

C14_API( void )
C14PurgeSpaceM68K(UInt32 regs[16]);

C14_API( void )
C14SetApplLimit(void * zoneLimit);

C14_API( Ptr )
C14StripAddress(void * theAddress);


#endif /* __C14Memory_h__ */
