
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_i2cbus.h
//
// I2C bus interface for AxiCat.
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


#ifndef __AXICAT_I2CBUS_H__
#define __AXICAT_I2CBUS_H__


#include "i2cbus.h"


// Forward declarations

typedef struct  _AXICAT_I2CBUS_CREATE_IO    AXICAT_I2CBUS_CREATE_IO;


typedef U8      AXICAT_I2CBUS_CREATE_ERR;

#define AXICAT_I2CBUS_CREATE_ERR_NONE       0
#define AXICAT_I2CBUS_CREATE_ERR_OOM        1
#define AXICAT_I2CBUS_CREATE_ERR_FT245      2
#define AXICAT_I2CBUS_CREATE_ERR_AL         3


struct  _AXICAT_I2CBUS_CREATE_IO
{
    CHAR                       *path;           // [IN]  Serial path
    FLAG                        verbose;        // [IN]  Verbose mode y/n

    // Non-zero when AXICAT_I2CBus_Create() returns 0
    //
    AXICAT_I2CBUS_CREATE_ERR    err;            // [OUT] Error code
};


I2CBUS_INTF  *AXICAT_I2CBus_Create (AXICAT_I2CBUS_CREATE_IO *io);

VOID  AXICAT_I2CBus_Destroy (I2CBUS_INTF *intf);


#endif  // __AXICAT_I2CBUS_H__
