/* make sure this is correct CNFGGLOB */
#ifndef __MWERKS__
#error "wrong CNFGGLOB.h"
#endif
#ifndef __POWERPC__
#error "wrong CNFGGLOB.h"
#endif

#define MyCompilerMetrowerks 1
#define BigEndianUnaligned 1
#define MayInline __inline__

#define Debug 1

#define MacTarget 1
#define MySoundEnabled 1

#define kStrAppName "Mini vMac"
#define kAppVariationStr "minivmac-2.4.1-mw8carbd"
#define kStrCopyrightYear "2004"
#define kMaintainerName "Paul C. Pratt"
#define kStrHomePage "http://www.gryphel.com/c/minivmac"

