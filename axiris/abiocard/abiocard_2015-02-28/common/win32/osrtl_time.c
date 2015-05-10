
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// osrtl.c
//
// Run-time library functions specific to the operating system.
//
// Language: MSVC60
//
// History:
//
//   2014-07-05  Peter S'heeren, Axiris
//
//      * Created.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2014  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#include "osrtl.h"

#include <windows.h>


VOID  OSRTL_SleepMs (U32 ms)
{
    Sleep(ms);
}


U32  OSRTL_GetTickCount (VOID)
{
    return GetTickCount();
}


VOID  OSRTL_Yield (VOID)
{
    Sleep(0);
}
