
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// ft245_serial.h
//
// Serial interface for FT245 handle.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-08-25  Peter S'heeren, Axiris
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


#ifndef __FT245_SERIAL_H__
#define __FT245_SERIAL_H__


#include "ft245.h"
#include "serial.h"


SERIAL_INTF  *FT245_Serial_Create (FT245_HANDLE handle, FLAG verbose);

VOID  FT245_Serial_Destroy (SERIAL_INTF *intf);


#endif  // __FT245_SERIAL_H__
