
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// serial.h
//
// Definition of a serial port interface.
//
// Language: GCC4 gnu89
//
// History:
//
//   2014-02-12  Peter S'heeren, Axiris
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


#ifndef __SERIAL_H__
#define __SERIAL_H__


#include "platform.h"


// Forward declarations

typedef struct  _SERIAL_SETTINGS        SERIAL_SETTINGS;
typedef struct  _SERIAL_FN_TABLE        SERIAL_FN_TABLE;
typedef struct  _SERIAL_INTF            SERIAL_INTF;


struct  _SERIAL_SETTINGS
{
    U32         baudrate;               // [IN]  1..
    U8          data_bits;              // [IN]  5, 6, 7, 8
    U8          stop_bits;              // [IN]  * =0: 1 stop bit
                                        //       * =1: 1.5 stop bits
                                        //       * =2: 2 stop bits
    U8          parity;                 // [IN]  * =0: No parity
                                        //       * =1: Odd parity
                                        //       * =2: Even parity
    U8          handshake;              // [IN]  * =0: No handshaking
};


// Non-blocking write

typedef FLAG    SERIAL_WRITE_FN (SERIAL_INTF *intf, U8 *buf, U32 len, U32 *xfrd);

// Non-blocking read

typedef FLAG    SERIAL_READ_FN (SERIAL_INTF *intf, U8 *buf, U32 len, U32 *xfrd);

typedef FLAG    SERIAL_SET_SETTINGS_FN (SERIAL_INTF *intf, SERIAL_SETTINGS *settings);

typedef FLAG    SERIAL_SET_BAUDRATE_FN (SERIAL_INTF *intf, U32 baudrate);

typedef FLAG    SERIAL_PURGE_RX_FN (SERIAL_INTF *intf);

typedef FLAG    SERIAL_SET_DTR_FN (SERIAL_INTF *intf);

typedef FLAG    SERIAL_CLEAR_DTR_FN (SERIAL_INTF *intf);

typedef FLAG    SERIAL_SET_RTS_FN (SERIAL_INTF *intf);

typedef FLAG    SERIAL_CLEAR_RTS_FN (SERIAL_INTF *intf);


// Function call table for I2C bus interface

struct  _SERIAL_FN_TABLE
{
    SERIAL_WRITE_FN            *write_fn;
    SERIAL_READ_FN             *read_fn;
    SERIAL_SET_SETTINGS_FN     *set_settings_fn;
    SERIAL_SET_BAUDRATE_FN     *set_baudrate_fn;
    SERIAL_PURGE_RX_FN         *purge_rx_fn;
    SERIAL_SET_DTR_FN          *set_dtr_fn;
    SERIAL_CLEAR_DTR_FN        *clear_dtr_fn;
    SERIAL_SET_RTS_FN          *set_rts_fn;
    SERIAL_CLEAR_RTS_FN        *clear_rts_fn;
};


// Embed this structure in the context of the owner (creator) of the serial
// interface

struct  _SERIAL_INTF
{
    SERIAL_FN_TABLE        *fn_table;
};


#endif  // __SERIAL_H__
