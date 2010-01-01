
#include "C14OSUtils.h"


DEFINE_API( OSErr )
SysEnvirons(
  short        versionRequested,
  SysEnvRec *  theWorld)
{
    return C14SysEnvirons(versionRequested, theWorld);
}
