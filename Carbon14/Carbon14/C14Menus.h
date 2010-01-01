
#ifndef __C14Menus_h__
#define __C14Menus_h__


#include "C14Macros.h"


C14_API( void )
C14CheckItem(
  MenuRef   theMenu,
  short     item,
  Boolean   checked);

C14_API( void )
C14DisableItem(
  MenuRef   theMenu,
  short     item);

C14_API( void )
C14EnableItem(
  MenuRef   theMenu,
  short     item);

C14_API( void )
C14InitMenus(void);


#endif /* __C14Menus_h__ */
