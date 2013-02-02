
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// i2c.h
//
// Definition of a simple I2C interface.
//
// Language: GCC4 gnu89
//
// History:
//
//   2013-01-20  Peter S'heeren, Axiris
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


#ifndef __I2C_H__
#define __I2C_H__


#include "platform.h"


typedef struct  _I2C_WRITE_IO           I2C_WRITE_IO;
typedef struct  _I2C_READ_IO            I2C_READ_IO;
typedef struct  _I2C_CB_TABLE           I2C_CB_TABLE;


struct  _I2C_WRITE_IO
{
    U8     *buf;
    U16     len;
    U16     xfrd;
    U8      slave_ad;
};


struct  _I2C_READ_IO
{
    U8     *buf;
    U16     len;
    U16     xfrd;
    U8      slave_ad;
};


typedef FLAG    I2C_WRITE_FN (POINTER *ctx, I2C_WRITE_IO *io);
typedef FLAG    I2C_READ_FN (POINTER *ctx, I2C_READ_IO *io);
typedef FLAG    I2C_WRITE_REG_FN (POINTER *ctx, U8 slave_ad, U8 reg);
typedef FLAG    I2C_READ_REG_FN (POINTER *ctx, U8 slave_ad, U8 *reg);


// Callback table for I2C interface

struct  _I2C_CB_TABLE
{
    I2C_WRITE_FN       *write_fn;
    I2C_READ_FN        *read_fn;
    I2C_WRITE_REG_FN   *write_reg_fn;
    I2C_READ_REG_FN    *read_reg_fn;
};


#endif  // __I2C_H__

