
#include "C14Memory.h"


DEFINE_API( OSErr )
C14MaxApplZone(void)
{
    /* no-op */
    return noErr;
}


DEFINE_API( void )
C14PurgeSpaceM68K(UInt32 regs[16])
{
    regs[0] = PurgeSpaceContiguous();
    regs[8+0] = PurgeSpaceTotal();
}


DEFINE_API( void )
C14SetApplLimit(void * zoneLimit)
{
    /* no-op */
}


DEFINE_API( Ptr )
C14StripAddress(void * theAddress)
{
    return (Ptr)theAddress;
}
