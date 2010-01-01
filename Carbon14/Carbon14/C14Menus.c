
#include "C14Menus.h"


DEFINE_API( void )
C14CheckItem(
  MenuRef   theMenu,
  short     item,
  Boolean   checked)
{
#undef CheckMenuItem
    CheckMenuItem(theMenu, item, checked);
}


DEFINE_API( void )
C14DisableItem(
  MenuRef   theMenu,
  short     item)
{
    DisableMenuItem(theMenu, item);
}


DEFINE_API( void )
C14EnableItem(
  MenuRef   theMenu,
  short     item)
{
    EnableMenuItem(theMenu, item);
}


DEFINE_API( void )
C14InitMenus(void)
{
    /* no-op */
}
