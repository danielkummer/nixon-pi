
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// ft245_serial.c
//
// Serial interface for FT245 handle.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-08-25  Peter S'heeren, Axiris
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


#include "ft245_serial.h"
#include "osrtl.h"

#include <stdio.h>
#include <stdlib.h>


// Forward declarations

typedef struct  _CTX            CTX;


struct  _CTX
{
    SERIAL_INTF         serial_intf;
    FT245_HANDLE        ft245_handle;
};


static
FLAG  SERIAL_Write_CB (SERIAL_INTF *intf, U8 *buf, U32 len, U32 *xfrd)
{
    CTX    *ctx;

    // Init
    ctx = GetContAd(intf,CTX,serial_intf);

    return FT245_Write(ctx->ft245_handle,buf,len,xfrd);
}


static
FLAG  SERIAL_Read_CB (SERIAL_INTF *intf, U8 *buf, U32 len, U32 *xfrd)
{
    CTX    *ctx;

    // Init
    ctx = GetContAd(intf,CTX,serial_intf);

    return FT245_Read(ctx->ft245_handle,buf,len,xfrd);
}


static
FLAG  SERIAL_Set_Settings_CB (SERIAL_INTF *intf, SERIAL_SETTINGS *settings)
{
    return 1;
}


static
FLAG  SERIAL_Set_Baudrate_CB (SERIAL_INTF *intf, U32 baudrate)
{
    return 1;
}


static
FLAG  SERIAL_Purge_Rx_CB (SERIAL_INTF *intf)
{
    CTX    *ctx;
    U8      buf[32];
    U32     xfrd;
    FLAG    ok;

    // Init
    ctx = GetContAd(intf,CTX,serial_intf);

    for (;;)
    {
        ok = FT245_Read(ctx->ft245_handle,buf,sizeof(buf),&xfrd);
        if (!ok) return 0;

        if (xfrd == 0) break;
    }

    return 1;
}


static
FLAG  SERIAL_Set_DTR_CB (SERIAL_INTF *intf)
{
    return 1;
}


static
FLAG  SERIAL_Clear_DTR_CB (SERIAL_INTF *intf)
{
    return 1;
}


static
FLAG  SERIAL_Set_RTS_CB (SERIAL_INTF *intf)
{
    return 1;
}


static
FLAG  SERIAL_Clear_RTS_CB (SERIAL_INTF *intf)
{
    return 1;
}


static  SERIAL_FN_TABLE     serial_fn_table =
{
    SERIAL_Write_CB,            // SERIAL_WRITE_FN
    SERIAL_Read_CB,             // SERIAL_READ_FN
    SERIAL_Set_Settings_CB,     // SERIAL_SET_SETTINGS_FN
    SERIAL_Set_Baudrate_CB,     // SERIAL_SET_BAUDRATE_FN
    SERIAL_Purge_Rx_CB,         // SERIAL_PURGE_RX_FN
    SERIAL_Set_DTR_CB,          // SERIAL_SET_DTR_FN
    SERIAL_Clear_DTR_CB,        // SERIAL_CLEAR_DTR_FN
    SERIAL_Set_RTS_CB,          // SERIAL_SET_RTS_FN
    SERIAL_Clear_RTS_CB         // SERIAL_CLEAR_RTS_FN
};


VOID  FT245_Serial_Destroy (SERIAL_INTF *intf)
{
    CTX    *ctx;

    if (!intf) return;

    // Get the containing context
    ctx = GetContAd(intf,CTX,serial_intf);

    free(ctx);
}


SERIAL_INTF  *FT245_Serial_Create (FT245_HANDLE handle, FLAG verbose)
{
    CTX    *ctx;


    // Allocate
    ctx = malloc(sizeof(CTX));
    if (!ctx)
    {
        if (verbose) printf("Error: out of memory\n");
        return 0;
    }

    // Set up
    ctx->serial_intf.fn_table = &serial_fn_table;
    ctx->ft245_handle         = handle;

    return &ctx->serial_intf;
}
