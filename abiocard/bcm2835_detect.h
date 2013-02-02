
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// bcm2835_detect.h
//
// Detect the BCM2835 hardware.
//
// Language: GCC4 gnu89
//
// History:
//
//   2012-07-30  Peter S'heeren, Axiris
//
//      * Created.
//
//   2012-08-18  Peter S'heeren, Axiris
//
//      * Released.
//
//   2012-09-10  Peter S'heeren, Axiris
//
//      * Added detection of revision number.
//      * Released.
//
//   2012-12-10  Peter S'heeren, Axiris
//
//      * Added support for the warranty void bit in the revision code.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2012-2013  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#ifndef __BCM2835_DETECT_H__
#define __BCM2835_DETECT_H__


#include "platform.h"


typedef struct  _BCM2835_DETECT_IO      BCM2835_DETECT_IO;

struct  _BCM2835_DETECT_IO
{
    FLAG    res_detected;

    union
    {
        U32     res_word;

        struct
        {
            U32     res_revision  : 16;
            U32                   :  8;
            U32     warranty_void :  1;
            U32                   :  7;
        };
    };
};


FLAG  bcm2835_detect (BCM2835_DETECT_IO *io);


#endif  // __BCM2835_DETECT_H__

