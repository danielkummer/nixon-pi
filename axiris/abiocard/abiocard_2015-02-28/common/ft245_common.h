
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// ft245_common.h
//
// Interface for FT245 attached to serial device path.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-07-04  Peter S'heeren, Axiris
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


#ifndef __FT245_COMMON_H__
#define __FT245_COMMON_H__


#include "platform.h"


FT245_HANDLE  FT245_Open (U8 *path);

VOID  FT245_Close (FT245_HANDLE handle);

FLAG  FT245_Write (FT245_HANDLE handle, U8 *buf, U32 len, U32 *xfrd);

FLAG  FT245_Read (FT245_HANDLE handle, U8 *buf, U32 len, U32 *xfrd);


#endif  // __FT245_COMMON_H__
