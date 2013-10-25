
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// i2cbus.h
//
// Definition of a simple I2C bus (adapter) interface.
//
// Language: GCC4 gnu89
//
// History:
//
//   2013-03-02  Peter S'heeren, Axiris
//
//      * Created.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2013  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#ifndef __I2CBUS_H__
#define __I2CBUS_H__


#include "platform.h"


typedef struct  _I2CBUS_RW_IO           I2CBUS_RW_IO;
typedef struct  _I2CBUS_CB_TABLE        I2CBUS_CB_TABLE;
typedef struct  _I2CBUS_INTF            I2CBUS_INTF;


// I/O for read and write functions

struct  _I2CBUS_RW_IO
{
    U8             *buf;                // [IN]  Data buffer
    U16             len;                // [IN]  Bytes in data buffer
    U16             xfrd;               // [OUT] Bytes transferred
    U8              slave_ad;           // [IN]  I2C slave address
};


typedef FLAG    I2CBUS_WRITE_FN (I2CBUS_INTF *intf, I2CBUS_RW_IO *io);
typedef FLAG    I2CBUS_READ_FN (I2CBUS_INTF *intf, I2CBUS_RW_IO *io);
typedef FLAG    I2CBUS_WRITE_REG_FN (I2CBUS_INTF *intf, U8 slave_ad, U8 reg);
typedef FLAG    I2CBUS_READ_REG_FN (I2CBUS_INTF *intf, U8 slave_ad, U8 *reg);


// Callback table for I2C bus interface

struct  _I2CBUS_CB_TABLE
{
    I2CBUS_WRITE_FN        *write_fn;
    I2CBUS_READ_FN         *read_fn;
    I2CBUS_WRITE_REG_FN    *write_reg_fn;
    I2CBUS_READ_REG_FN     *read_reg_fn;
};


struct  _I2CBUS_INTF
{
    I2CBUS_CB_TABLE        *cb_table;
};


#endif  // __I2CBUS_H__

