
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// i2cbus.h
//
// Definition of a simple non-blocking I2C bus (adapter) interface.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2013-03-02  Peter S'heeren, Axiris
//
//      * Created.
//
//   2014-05-14  Peter S'heeren, Axiris
//
//      * Added non-blocking transfers.
//      * Removed blocking transfers.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2012-2014  Peter S'heeren, Axiris
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


// Forward declarations

typedef struct  _I2CBUS_XFR             I2CBUS_XFR;
typedef struct  _I2CBUS_FN_TABLE        I2CBUS_FN_TABLE;
typedef struct  _I2CBUS_INTF            I2CBUS_INTF;


// I2C data transfer direction

#define I2CBUS_DIR_WRITE                0   // Master -> slave
#define I2CBUS_DIR_READ                 1   // Slave -> master


// Transfer status

typedef U8  I2CBUS_XFR_STATUS;

#define I2CBUS_XFR_STATUS_SUCCESS       0   // Completed: success
#define I2CBUS_XFR_STATUS_CANCELLED     1   // Completed: cancelled
#define I2CBUS_XFR_STATUS_ERROR         2   // Completed: other error
#define I2CBUS_XFR_STATUS_BUS_ERROR     3   // Completed: bus error
#define I2CBUS_XFR_STATUS_ARB_LOST      4   // Completed: arbitration lost
#define I2CBUS_XFR_STATUS_SCHEDULED    15   // Scheduled


// I2C transfer object
//
// I/O non-for blocking I2C master read and write functions.

struct  _I2CBUS_XFR
{
    U8             *buf;                // [IN]  Read/write buffer
    U16             sc;                 // [IN]  Store count: bytes to read/write
    U16             xfrd;               // [OUT] Bytes transferred
    U8              slave_ad;           // [IN]  Slave address
    FLAG            dir;                // [IN]  Read or write (I2CBUS_DIR_Xxx constant)
    FLAG            force_stop;         // [IN]  Force stop (1) or allow restart (0)
    FLAG            nack_rsp;           // [OUT] NACK (1) or ACK (0) response on SLA+W, SLA+R or last data byte
};


// Set the bus speed (Hz). The actual speed must be equal or less than the
// given value.

typedef VOID  I2CBUS_SET_SPEED_FN (I2CBUS_INTF *intf, U32 speed);


// Create a transfer object.

typedef I2CBUS_XFR  *I2CBUS_XFR_CREATE_FN (I2CBUS_INTF *intf);


// Destroy the given transfer object.
//
// If the transfer is scheduled, it'll be cancelled first.
//
// The given pointer may be zero.

typedef VOID  I2CBUS_XFR_DESTROY_FN (I2CBUS_INTF *intf, I2CBUS_XFR *xfr);


// Schedule the given transfer.

typedef FLAG  I2CBUS_XFR_SCHEDULE_FN (I2CBUS_INTF *intf, I2CBUS_XFR *xfr);


// Immediately complete the given transfer as cancelled. The transfer object
// becomes available for scheduling a new transfer.

typedef VOID  I2CBUS_XFR_CANCEL_FN (I2CBUS_INTF *intf, I2CBUS_XFR *xfr);


// Get the status of the transfer.
//
// If the transfer has completed, the transfer object will become available for
// scheduling a new transfer.

typedef I2CBUS_XFR_STATUS  I2CBUS_XFR_GET_STATUS_FN (I2CBUS_INTF *intf, I2CBUS_XFR *xfr);


// Function call table for I2C bus interface

struct  _I2CBUS_FN_TABLE
{
    I2CBUS_SET_SPEED_FN        *set_speed_fn;
    I2CBUS_XFR_CREATE_FN       *xfr_create_fn;
    I2CBUS_XFR_DESTROY_FN      *xfr_destroy_fn;
    I2CBUS_XFR_SCHEDULE_FN     *xfr_schedule_fn;
    I2CBUS_XFR_CANCEL_FN       *xfr_cancel_fn;
    I2CBUS_XFR_GET_STATUS_FN   *xfr_get_status_fn;
};


// Embed this structure in the context of the owner (creator) of the I2C bus
// interface

struct  _I2CBUS_INTF
{
    I2CBUS_FN_TABLE        *fn_table;
};


#endif  // __I2CBUS_H__

