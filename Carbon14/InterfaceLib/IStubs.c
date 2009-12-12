
DEFINE_API( Ptr )
GetApplLimit(void)
{
    return (Ptr)0x400000;
}

DEFINE_API( void )
MaxApplZone(void)
{
    return;
}

DEFINE_API( UniversalProcPtr )
NGetTrapAddress(
  UInt16     trapNum,
  TrapType   tTyp)
{
    return (UniversalProcPtr)trapNum;
}

DEFINE_API( void )
EnableItem(
  MenuRef   theMenu,
  short     item)
{
    EnableMenuItem(theMenu, item);
}

DEFINE_API( void )
DisableItem(
  MenuRef   theMenu,
  short     item)
{
    DisableMenuItem(theMenu, item);
}

DEFINE_API( void )
SystemClick(
  const EventRecord *  theEvent,
  WindowRef            theWindow)
{
}

DEFINE_API( void )
InitFonts(void)
{
}

DEFINE_API( void )
TEInit(void)
{
}

DEFINE_API( THz )
ApplicationZone(void)
{
    return (THz)0;
}

DEFINE_API( void )
InitDialogs(void * ignored)
{
}

DEFINE_API( void )
InitMenus(void)
{
}

DEFINE_API( void )
InitWindows(void)
{
}

DEFINE_API( void )
CheckItem(
  MenuRef   theMenu,
  short     item,
  Boolean   checked)
{
#undef CheckMenuItem
    CheckMenuItem(theMenu, item, checked);
}

DEFINE_API( void )
SystemTask(void)
{
}

DEFINE_API( OSErr )
SysEnvirons(
  short        versionRequested,
  SysEnvRec *  theWorld)
{
    theWorld->environsVersion = versionRequested;
    theWorld->machineType = envMacIIfx;
    /*...*/
}

DEFINE_API( short )
DIBadMount(
  Point    where,
  UInt32   evtMessage)
{
    return noErr;
}

DEFINE_API( Boolean )
OSEventAvail(
  EventMask      mask,
  EventRecord *  theEvent)
{
    /* XXX: adjust 'mask' for OS events */
    return EventAvail(mask, theEvent);
}

DEFINE_API( Boolean )
SystemEdit(short editCmd)
{
    return FALSE;
}

DEFINE_API( short )
OpenDeskAcc(ConstStr255Param deskAccName)
{
    return 0;
}

DEFINE_API( void )
CloseDeskAcc(short refNum)
{
}
