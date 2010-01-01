
#include "C14Menus.h"


DEFINE_API( void )
CheckItem(
  MenuRef   theMenu,
  short     item,
  Boolean   checked)
{
    C14CheckItem(theMenu, item, checked);
}


DEFINE_API( void )
DisableItem(
  MenuRef   theMenu,
  short     item)
{
    C14DisableItem(theMenu, item);
}


DEFINE_API( void )
EnableItem(
  MenuRef   theMenu,
  short     item)
{
    C14EnableItem(theMenu, item);
}


DEFINE_API( void )
InitMenus(void)
{
    C14InitMenus();
}
