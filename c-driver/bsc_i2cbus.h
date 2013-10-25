
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// bsc_i2cbus.h
//
// I2C bus interface for BCM2835 BSC controller.
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


#ifndef __BSC_I2CBUS_H__
#define __BSC_I2CBUS_H__


#include "i2cbus.h"


// Forward declarations

typedef struct  _BSC_I2CBUS_CREATE_IO   BSC_I2CBUS_CREATE_IO;


typedef U8      BSC_I2CBUS_CREATE_ERR;

#define BSC_I2CBUS_CREATE_ERR_NONE      0
#define BSC_I2CBUS_CREATE_ERR_PARAM     1
#define BSC_I2CBUS_CREATE_ERR_OOM       2
#define BSC_I2CBUS_CREATE_ERR_LOCKED    3
#define BSC_I2CBUS_CREATE_ERR_NO_PERM   4
#define BSC_I2CBUS_CREATE_ERR_OTHER     5


struct  _BSC_I2CBUS_CREATE_IO
{
    U8                      bsc_index;  // [IN]  Index of the BSC controller (0, 1)
    FLAG                    verbose;    // [IN]  Verbose mode y/n
    
    // Non-zero when BSC_I2CBus_Create() returns 0
    //
    BSC_I2CBUS_CREATE_ERR   err;        // [OUT] Error code
};


I2CBUS_INTF  *BSC_I2CBus_Create (BSC_I2CBUS_CREATE_IO *io);

VOID  BSC_I2CBus_Destroy (I2CBUS_INTF *intf);


#endif  // __BSC_I2CBUS_H__

