
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// i2cdev_i2cbus.h
//
// I2C bus interface for a i2c-dev file.
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


#ifndef __I2CDEV_I2CBUS_H__
#define __I2CDEV_I2CBUS_H__


#include "i2cbus.h"


// Forward declarations

typedef struct  _I2CDEV_I2CBUS_CREATE_IO    I2CDEV_I2CBUS_CREATE_IO;


typedef U8      I2CDEV_I2CBUS_CREATE_ERR;

#define I2CDEV_I2CBUS_CREATE_ERR_NONE       0
#define I2CDEV_I2CBUS_CREATE_ERR_PARAM      1
#define I2CDEV_I2CBUS_CREATE_ERR_OOM        2
#define I2CDEV_I2CBUS_CREATE_ERR_NO_DEV     3
#define I2CDEV_I2CBUS_CREATE_ERR_NO_PERM    4
#define I2CDEV_I2CBUS_CREATE_ERR_OTHER      5


struct  _I2CDEV_I2CBUS_CREATE_IO
{
    CHAR                       *i2cdev_name;    // [IN]  Device path
    FLAG                        verbose;        // [IN]  Verbose mode y/n

    // Non-zero when I2CDev_I2CBus_Create() returns 0
    //
    I2CDEV_I2CBUS_CREATE_ERR    err;            // [OUT] Error code
};


I2CBUS_INTF  *I2CDev_I2CBus_Create (I2CDEV_I2CBUS_CREATE_IO *io);

VOID  I2CDev_I2CBus_Destroy (I2CBUS_INTF *intf);


#endif  // __I2CDEV_I2CBUS_H__

