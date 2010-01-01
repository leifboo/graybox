
#ifndef __C14Dialogs_h__
#define __C14Dialogs_h__


#include "C14Macros.h"


C14_API( DialogItemIndex )
C14Alert(
  SInt16           alertID,
  ModalFilterUPP   modalFilter);

C14_API( void )
C14InitDialogs(void * ignored);


#endif /* __C14Dialogs_h__ */
