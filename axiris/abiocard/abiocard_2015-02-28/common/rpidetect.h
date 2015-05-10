
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// rpidetect.h
//
// Detect the presence of the RPi SoC.
//
// Language: GCC4 gnu89
//
// History:
//
//   2015-02-22  Peter S'heeren, Axiris
//
//      * Created.
//
//   2015-02-23  Peter S'heeren, Axiris
//
//      * Released.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2015  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#ifndef __RPIDETECT_H__
#define __RPIDETECT_H__


#include "platform.h"


// Forward declarations

typedef struct  _RPIDETECT_IO           RPIDETECT_IO;


typedef U8      RPIDETECT_CHIP;

#define RPIDETECT_CHIP_UNKNOWN    0     // RPi not detected
#define RPIDETECT_CHIP_BCM2835    1
#define RPIDETECT_CHIP_BCM2836    2


struct  _RPIDETECT_IO
{
    RPIDETECT_CHIP      chip;

    U8                  bsc_index;

    FLAG                warranty_void;
};


VOID  rpidetect (RPIDETECT_IO *io);


#endif  // __RPIDETECT_H__

