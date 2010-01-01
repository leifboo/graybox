
DEFINE_API( Ptr )
GetApplLimit(void)
{
    return (Ptr)0x400000;
}

DEFINE_API( UniversalProcPtr )
NGetTrapAddress(
  UInt16     trapNum,
  TrapType   tTyp)
{
    return (UniversalProcPtr)trapNum;
}

DEFINE_API( void )
SystemClick(
  const EventRecord *  theEvent,
  WindowRef            theWindow)
{
}

DEFINE_API( THz )
ApplicationZone(void)
{
    return (THz)0;
}

DEFINE_API( void )
SystemTask(void)
{
}

DEFINE_API( short )
DIBadMount(
  Point    where,
  UInt32   evtMessage)
{
    return noErr;
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
