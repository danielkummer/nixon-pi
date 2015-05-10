
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// osrtl.h
//
// Run-time library functions specific to the operating system.
//
// Language: GCC4 gnu89, MSVC60
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


#ifndef __OSRTL_H__
#define __OSRTL_H__


#include "platform.h"


#define GetContAd(var_ad,cont_type_name,var_name)           \
    (cont_type_name*)                                       \
    ((U8*)(var_ad) - (U8*)&((cont_type_name*)0)->var_name)


VOID  OSRTL_SleepMs (U32 ms);

U32  OSRTL_GetTickCount (VOID);

VOID  OSRTL_Yield (VOID);


#endif  // __OSRTL_H__
