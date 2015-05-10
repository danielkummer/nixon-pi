
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_i2cbus.c
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


#include "axicat_i2cbus.h"

#include "osrtl.h"
#include "ft245.h"
#include "axicat_al.h"
#include "serial.h"

#include <stdio.h>
#include <stdlib.h>


// Forward declarations

typedef struct  _CTX            CTX;
typedef struct  _BRIDGE_XFR     BRIDGE_XFR;


struct  _CTX
{
    I2CBUS_INTF         i2cbus_intf;

    SERIAL_INTF         serial_intf;

    FT245_HANDLE        ft245_handle;
    AXICAT_AL_HANDLE    axicat_al_handle;
};


struct  _BRIDGE_XFR
{
    AXICAT_AL_TWI_XFR      *axicat_xfr; // Transfer object in the AxiCat AL
    I2CBUS_XFR              user_xfr;   // Public area for user
    U8                      status;     // Most recently produced status (I2CBUS_XFR_STATUS_Xxx)
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


static
VOID  I2CBUS_Set_Speed_CB (I2CBUS_INTF *intf, U32 speed)
{
    // <>
}


static
VOID  I2CBUS_Xfr_Destroy_CB (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    BRIDGE_XFR     *bridge_xfr;

    // Get the containing context
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    AXICAT_AL_TWI_Xfr_Destroy(bridge_xfr->axicat_xfr);
    free(bridge_xfr);
}


static
I2CBUS_XFR  *I2CBUS_Xfr_Create_CB (I2CBUS_INTF *intf)
{
    CTX            *ctx;
    BRIDGE_XFR     *bridge_xfr;

    // Get the containing context
    ctx = GetContAd(intf,CTX,i2cbus_intf);

    // Allocate 
    bridge_xfr = calloc(sizeof(BRIDGE_XFR),1);
    if (!bridge_xfr) return 0;

    return &bridge_xfr->user_xfr;
}


static
FLAG  I2CBUS_Xfr_Schedule_CB (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    CTX            *ctx;
    BRIDGE_XFR     *bridge_xfr;
    FLAG            ok;

    // Get the containing contexts
    ctx        = GetContAd(intf,CTX,i2cbus_intf);
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    // Can't schedule if a transfer is already scheduled
    if (bridge_xfr->axicat_xfr) return 0;

    // Allocate a transfer object
    bridge_xfr->axicat_xfr = AXICAT_AL_TWI_Xfr_Create(ctx->axicat_al_handle);
    if (!bridge_xfr->axicat_xfr) goto Err;

    // Set up he transfer object
    bridge_xfr->axicat_xfr->buf        = user_xfr->buf;
    bridge_xfr->axicat_xfr->sc         = user_xfr->sc;
    bridge_xfr->axicat_xfr->dir        = (user_xfr->dir == I2CBUS_DIR_WRITE)
                                            ? AXICAT_AL_TWI_DIR_WRITE
                                            : AXICAT_AL_TWI_DIR_READ;
    bridge_xfr->axicat_xfr->force_stop = user_xfr->force_stop;
    bridge_xfr->axicat_xfr->slave_ad   = user_xfr->slave_ad;

    // Schedule the transfer
    ok = AXICAT_AL_TWI_Xfr_Master_Schedule(bridge_xfr->axicat_xfr);
    if (!ok) goto Err;

    bridge_xfr->status = I2CBUS_XFR_STATUS_SCHEDULED;

    return 1;

  Err:

    AXICAT_AL_TWI_Xfr_Destroy(bridge_xfr->axicat_xfr);
    bridge_xfr->axicat_xfr = 0;

    return 0;
}


static
VOID  I2CBUS_Xfr_Cancel_CB (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    BRIDGE_XFR     *bridge_xfr;

    // Get the containing context
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    // Can't cancel if a transfer isn't scheduled
    if (!bridge_xfr->axicat_xfr) return;

    // Request cancellation
    AXICAT_AL_TWI_Xfr_Cancel(bridge_xfr->axicat_xfr);
}


static
I2CBUS_XFR_STATUS  I2CBUS_Xfr_Get_Status_CB
(
    I2CBUS_INTF    *intf,
    I2CBUS_XFR     *user_xfr
)
{
    CTX            *ctx;
    BRIDGE_XFR     *bridge_xfr;

    // Get the containing context
    ctx        = GetContAd(intf,CTX,i2cbus_intf);
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    if (bridge_xfr->axicat_xfr)
    {
        // The transfer is marked as scheduled

        U8      st;

        AXICAT_AL_Step(ctx->axicat_al_handle,0);

        st = bridge_xfr->axicat_xfr->status;

        if (st != AXICAT_AL_TWI_ST_SCHEDULED)
        {
            // The transfer has completed

            // Translate the resulting status code
            if (st == AXICAT_AL_TWI_ST_SUCCESS)   bridge_xfr->status = I2CBUS_XFR_STATUS_SUCCESS; else
            if (st == AXICAT_AL_TWI_ST_CANCELLED) bridge_xfr->status = I2CBUS_XFR_STATUS_CANCELLED; else
            if (st == AXICAT_AL_TWI_ST_BUS_ERROR) bridge_xfr->status = I2CBUS_XFR_STATUS_BUS_ERROR; else
            if (st == AXICAT_AL_TWI_ST_ARB_LOST)  bridge_xfr->status = I2CBUS_XFR_STATUS_ARB_LOST; else
                bridge_xfr->status = I2CBUS_XFR_STATUS_ERROR;

            // Report back the transfer results
            bridge_xfr->user_xfr.xfrd     = bridge_xfr->axicat_xfr->xfrd;
            bridge_xfr->user_xfr.nack_rsp = bridge_xfr->axicat_xfr->nack_rsp;

            AXICAT_AL_TWI_Xfr_Destroy(bridge_xfr->axicat_xfr);
            bridge_xfr->axicat_xfr = 0;
        }
        // else: The transfer is still scheduled
    }

    return bridge_xfr->status;
}


static
I2CBUS_FN_TABLE     i2cbus_fn_table =
{
    I2CBUS_Set_Speed_CB,        // I2CBUS_SET_SPEED_FN
    I2CBUS_Xfr_Create_CB,       // I2CBUS_XFR_CREATE_FN
    I2CBUS_Xfr_Destroy_CB,      // I2CBUS_XFR_DESTROY_FN
    I2CBUS_Xfr_Schedule_CB,     // I2CBUS_XFR_SCHEDULE_FN
    I2CBUS_Xfr_Cancel_CB,       // I2CBUS_XFR_CANCEL_FN
    I2CBUS_Xfr_Get_Status_CB    // I2CBUS_XFR_GET_STATUS_FN
};


VOID  AXICAT_I2CBus_Destroy (I2CBUS_INTF *intf)
{
    CTX    *ctx;

    if (!intf) return;

    // Get the containing context
    ctx = GetContAd(intf,CTX,i2cbus_intf);

    AXICAT_AL_Destroy(ctx->axicat_al_handle);
    FT245_Close(ctx->ft245_handle);
    free(ctx);
}


I2CBUS_INTF  *AXICAT_I2CBus_Create (AXICAT_I2CBUS_CREATE_IO *io)
{
    CTX    *ctx;


    // Allocate a cleared context
    ctx = calloc(sizeof(CTX),1);
    if (!ctx)
    {
        if (io->verbose) printf("Error: out of memory\n");
        io->err = AXICAT_I2CBUS_CREATE_ERR_OOM;
        goto Err;
    }

    // Set up the context
    ctx->i2cbus_intf.fn_table = &i2cbus_fn_table;


    ctx->ft245_handle = FT245_Open(io->path);
    if (!ctx->ft245_handle)
    {
        if (io->verbose) printf("Error: can't open serial path %s\n",io->path);

        io->err = AXICAT_I2CBUS_CREATE_ERR_FT245;

        goto Err;
    }


    ctx->serial_intf.fn_table = &serial_fn_table;

    ctx->axicat_al_handle = AXICAT_AL_Create(&ctx->serial_intf);
    if (!ctx->axicat_al_handle)
    {
        if (io->verbose) printf("Error: can't create AL\n");

        io->err = AXICAT_I2CBUS_CREATE_ERR_AL;

        goto Err;
    }


    // Wait for the end of the connecting state.
    for (;;)
    {
        AXICAT_AL_CONN_STATE    conn_state;
        FLAG                    ok;

        // Step
        ok = AXICAT_AL_Step(ctx->axicat_al_handle,0);
        if (!ok) goto Err;

        conn_state = AXICAT_AL_Get_Conn_State(ctx->axicat_al_handle);

        if (conn_state == AXICAT_AL_CONN_STATE_DISCONNECTED) goto Err;

        if (conn_state != AXICAT_AL_CONN_STATE_CONNECTING) break;

        OSRTL_Yield();
    }


    AXICAT_AL_TWI_Enable(ctx->axicat_al_handle);


    return &ctx->i2cbus_intf;


  Err:

    AXICAT_I2CBus_Destroy(&ctx->i2cbus_intf);
    return 0;
}

