
#include "C14Dialogs.h"


DEFINE_API( DialogItemIndex )
C14Alert(
  SInt16           alertID,
  ModalFilterUPP   modalFilter)
{
    /* XXX: deal with M68K 'modalFilter' */
    return Alert(alertID, modalFilter);
}


DEFINE_API( void )
C14InitDialogs(void * ignored)
{
    /* no-op */
}
