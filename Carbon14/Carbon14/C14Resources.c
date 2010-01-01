
#include "C14Resources.h"


DEFINE_API( void )
C14Get1IndType(
  ResType *  theType,
  short      index)
{
    /* Finder 6.1.8 calls this with theType == null. */
    if (theType)
        Get1IndType(theType, index);
}
