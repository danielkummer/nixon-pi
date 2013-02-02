
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// debug.c
//
// Debug functions.
//
// Language: GCC4 gnu89
//
// History:
//
//   2012-10-24  Peter S'heeren, Axiris
//
//      * Created.
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


#include <stdio.h>

#include "debug.h"


VOID  dbg_hex_dump (U8 *buf, U32 len, U8 bytes_per_row)
{
    U8      u;
    U8      v;

    u = 0;
    v = 0;
    do
    {
        printf("%02Xh ",buf[u]);
        u++;
        v++;
        if (v == 8) { printf("\n"); v = 0; }
    }
    while (u < len);
    if (v != 0) printf("\n");
}

