
#include "C14OSUtils.h"


DEFINE_API( OSErr )
C14SysEnvirons(
  short        versionRequested,
  SysEnvRec *  theWorld)
{
    /* XXX: not all will be set, depending upon 'versionRequested' */
    theWorld->environsVersion   = curSysEnvVers;
    theWorld->machineType       = envMacIIfx;
    theWorld->systemVersion     = 0; /* XXX */
    theWorld->processor         = env68000; /* XXX */
    theWorld->hasFPU            = false; /* XXX */
    theWorld->hasColorQD        = true;
    theWorld->keyBoardType      = envExtISOADBKbd;
    theWorld->atDrvrVersNum     = 0; /* XXX */
    theWorld->sysVRefNum        = 0; /* XXX */
    return noErr;
}
