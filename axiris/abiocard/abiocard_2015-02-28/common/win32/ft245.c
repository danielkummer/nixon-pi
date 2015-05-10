
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// ft245.c
//
// Interface for FT245 attached to serial device path.
//
// Language: MSVC60
//
// History:
//
//   2014-07-04  Peter S'heeren, Axiris
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


#include "ft245.h"

#include <windows.h>


// I/O control codes

#define IOCTL_SET_BAUD_RATE         0x001B0004
#define IOCTL_SET_QUEUE_SIZE        0x001B0008
#define IOCTL_SET_LINE_CTRL         0x001B000C
#define IOCTL_SET_TIMEOUTS          0x001B001C
#define IOCTL_PURGE                 0x001B004C
#define IOCTL_SET_HANDFLOW          0x001B0064


typedef struct  _IO_BAUD_RATE
{
    U32     rate;
}
    IO_BAUD_RATE;


typedef struct  _IO_QUEUE_SIZE
{
    U32     rx_buf_len;
    U32     tx_buf_len;
}
    IO_QUEUE_SIZE;


typedef struct  _IO_LINE_CTRL
{
    U8      stop_bits;          // stop_bits:  0 : 1 stop bit
                                //             1 : 1.5 stop bits
                                //             2 : 2 stop bits
    U8      parity;             // parity:     0 : No parity
                                //             1 : Odd parity
                                //             2 : Even parity
                                //             3 : Mark parity
                                //             4 : Space parity
    U8      word_len;           // word_len:   5 : 5 bits
                                //             6 : 6 bits
                                //             7 : 7 bits
                                //             8 : 8 bits
}
    IO_LINE_CTRL;


typedef struct  _IO_TIMEOUTS
{
    U32     rd_intv;
    U32     rd_total_mult;
    U32     rd_total_cst;
    U32     wr_total_mult;
    U32     wr_total_cst;
}
    IO_TIMEOUTS;


typedef struct  _IO_PURGE
{
    U32     tx_req  :  1;       // Abort transmit (write) requests
    U32     rx_req  :  1;       // Abort receive (read) requests
    U32     tx_buf  :  1;       // Clear transmit (write) buffers
    U32     rx_buf  :  1;       // Clear receive (read) buffers
    U32             : 28;
}
    IO_PURGE;


typedef struct  _IO_HANDFLOW
{
    U32     ctrl_handshake;
    U32     flow_replace;
    U32     xon_limit;
    U32     xoff_limit;
}
    IO_HANDFLOW;


FLAG  FT245_Write (FT245_HANDLE handle, U8 *buf, U32 len, U32 *xfrd)
{
    if (len == 0)
    {
        *xfrd = 0;
        return 1;
    }

    return WriteFile((HANDLE)handle,buf,len,(DWORD*)xfrd,0);
}


FLAG  FT245_Read (FT245_HANDLE handle, U8 *buf, U32 len, U32 *xfrd)
{
    if (len == 0)
    {
        *xfrd = 0;
        return 1;
    }

    return ReadFile((HANDLE)handle,buf,len,(DWORD*)xfrd,0);
}


VOID  FT245_Close (FT245_HANDLE handle)
{
    if (!handle) return;

    CloseHandle((HANDLE)handle);
}


FT245_HANDLE  FT245_Open (U8 *path)
{
    HANDLE          handle;
    IO_QUEUE_SIZE   io_queue_size;
    IO_TIMEOUTS     io_timeouts;
    IO_PURGE        io_purge;
    IO_BAUD_RATE    io_baud_rate;
    IO_LINE_CTRL    io_line_ctrl;
    IO_HANDFLOW     io_handflow;
    BOOL            ok;
    DWORD           xfrd;

    //
    // Set up the serial I/O
    //

    handle = CreateFileA
        (
            path,
            GENERIC_READ | GENERIC_WRITE,
            0,
            0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0
        );
    if (handle == INVALID_HANDLE_VALUE) goto Err;

    // Receive buffer
    io_queue_size.rx_buf_len = 65536;
    io_queue_size.tx_buf_len = 0;
    ok = DeviceIoControl(handle,IOCTL_SET_QUEUE_SIZE,&io_queue_size,sizeof(IO_QUEUE_SIZE),0,0,&xfrd,0);
    //if (!ok) goto Err; <- Don't check. Not sure if all serial drivers will succeed...

    // Timeouts
    io_timeouts.rd_intv       = 0xFFFFFFFF;    // - No timeout, read operation returns immediately
    io_timeouts.rd_total_mult = 0;             // /
    io_timeouts.rd_total_cst  = 0;             // /
    io_timeouts.wr_total_mult = 0;
    io_timeouts.wr_total_cst  = 0;
    ok = DeviceIoControl(handle,IOCTL_SET_TIMEOUTS,&io_timeouts,sizeof(IO_TIMEOUTS),0,0,&xfrd,0);
    if (!ok) goto Err;

    // Purge Tx and Rx buffers
    memset(&io_purge,0,sizeof(IO_PURGE));
    io_purge.rx_buf = 1;
    io_purge.tx_buf = 1;
    ok = DeviceIoControl(handle,IOCTL_PURGE,&io_purge,sizeof(IO_PURGE),0,0,&xfrd,0);
    if (!ok) goto Err;

    // Baud rate
    io_baud_rate.rate = 9600;
    ok = DeviceIoControl(handle,IOCTL_SET_BAUD_RATE,&io_baud_rate,sizeof(IO_BAUD_RATE),0,0,&xfrd,0);
    if (!ok) goto Err;

    // Line control
    io_line_ctrl.stop_bits = 0;
    io_line_ctrl.parity    = 0;
    io_line_ctrl.word_len  = 8;
    ok = DeviceIoControl(handle,IOCTL_SET_LINE_CTRL,&io_line_ctrl,sizeof(IO_LINE_CTRL),0,0,&xfrd,0);
    if (!ok) goto Err;

    // Hand flow
    io_handflow.ctrl_handshake = 0;
    io_handflow.flow_replace   = 0;
    io_handflow.xon_limit      = 0;
    io_handflow.xoff_limit     = 0;
    ok = DeviceIoControl(handle,IOCTL_SET_HANDFLOW,&io_handflow,sizeof(IO_HANDFLOW),0,0,&xfrd,0);
    if (!ok) goto Err;

    // Return successfully
    return (FT245_HANDLE)handle;

  Err:

    CloseHandle(handle);
    return 0;
}
