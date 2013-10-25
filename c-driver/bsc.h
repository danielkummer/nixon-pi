
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// bsc.h
//
// BCM2835 BSC driver.
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


#ifndef __BSC_H__
#define __BSC_H__


#include <sys/mman.h>

#include "i2cbus.h"


typedef struct  _BSC_MEMIO_CTX          BSC_MEMIO_CTX;


struct  _BSC_MEMIO_CTX
{
    off_t   pha;
    size_t  len;

    volatile    U32    *va;
};


VOID  bsc_memio_deinit (BSC_MEMIO_CTX *ctx);

FLAG  bsc_memio_init (BSC_MEMIO_CTX *ctx, int fd);

VOID  bsc_dump_status (BSC_MEMIO_CTX *p);

VOID  bsc_dump_regs (BSC_MEMIO_CTX *p);

FLAG  bsc_write (BSC_MEMIO_CTX *p, I2CBUS_RW_IO *io);

FLAG  bsc_read (BSC_MEMIO_CTX *p, I2CBUS_RW_IO *io);

FLAG  bsc_write_reg (BSC_MEMIO_CTX *p, U8 slave_ad, U8 reg);

FLAG  bsc_read_reg (BSC_MEMIO_CTX *p, U8 slave_ad, U8 *reg);


#endif  // __BSC_H__

