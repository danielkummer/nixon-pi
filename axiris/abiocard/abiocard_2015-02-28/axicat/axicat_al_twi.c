
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al_twi.c
//
// AxiCat AL.
//
// This is an include file.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-04-30  Peter S'heeren, Axiris
//
//      * Created.
//
//   2014-07-07  Peter S'heeren, Axiris
//
//      * Released.
//
//   2014-09-01  Peter S'heeren, Axiris
//
//      * Data buffer of transfer isn't accessed when the transfer has been
//        detached from the user.
//      * Added reporting of skipped status for slave Tx and Rx transfers.
//      * Released v1.0.0.
//
//   2014-09-17  Peter S'heeren, Axiris
//
//      * Released v1.1.0.
//
//   2014-09-28  Peter S'heeren, Axiris
//
//      * Released v1.2.0.
//
//   2014-12-12  Peter S'heeren, Axiris
//
//      * Released v1.3.0.
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


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Enable (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Enable (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_ENABLE)) goto Err;

    // Mark as enabled
    ctx->twi.hw_enabled = 1;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Disable (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Disable (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_DISABLE)) goto Err;

    // Mark as disabled
    ctx->twi.hw_enabled = 0;

    ctx->twi.bus_error = 0;
    ctx->twi.arb_lost  = 0;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Slave_Enable
(
    AXICAT_AL_HANDLE    handle,
    U8                  slave_ad,
    FLAG                gen_call
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Slave_Enable
(
    AXICAT_AL_HANDLE    handle,
    U8                  slave_ad,
    FLAG                gen_call
)
{
    CTX    *ctx;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (slave_ad > 127) goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;


    // Send command packet

    u = slave_ad << 1;
    if (gen_call) u |= 0x01;

    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_SLAVE_ENABLE)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;


    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Slave_Disable (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Slave_Disable (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_SLAVE_DISABLE)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Set_Speed (AXICAT_AL_HANDLE handle, U32 speed)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Set_Speed (AXICAT_AL_HANDLE handle, U32 speed)
{
    CTX    *ctx;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    if (speed == 50000)  u = AXICAT_TWI_SPEED_50000; else
    if (speed == 100000) u = AXICAT_TWI_SPEED_100000; else
    if (speed == 200000) u = AXICAT_TWI_SPEED_200000; else
    if (speed == 400000) u = AXICAT_TWI_SPEED_400000; else
        goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_SET_SPEED)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Set_Speed_Raw
(
    AXICAT_AL_HANDLE    handle,
    U8                  twbr,
    U8                  twps
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Set_Speed_Raw
(
    AXICAT_AL_HANDLE    handle,
    U8                  twbr,
    U8                  twps
)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (twps > 3) goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_SET_SPEED_RAW)) goto Err;
    if (!Tx_Byte(ctx,twbr)) goto Err;
    if (!Tx_Byte(ctx,twps)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
TWI_XFR  *TWI_Xfr_Get_Next (TWI_XFR *twi_xfr)
-----------------------------------------------------------------------------*/


static
TWI_XFR  *TWI_Xfr_Get_Next (TWI_XFR *twi_xfr)
{
    LDLL_ITEM  *item;

    item = LDLLGetNextItem(&twi_xfr->list_item);
    if (item)
        return GetContAd(item,TWI_XFR,list_item);
    else
        return 0;
}


/*-----------------------------------------------------------------------------
VOID  TWI_Xfr_Uncache (TWI_XFR *twi_xfr)

Remove the given transfer from the list of cached transfer objects and destroy
it.
-----------------------------------------------------------------------------*/


static
VOID  TWI_Xfr_Uncache (TWI_XFR *twi_xfr)
{
    TWI_CTX    *twi;

    // Init
    twi = &twi_xfr->ctx->twi;

    // Unlink from the list of cached transfer objects
    LDLLUnlinkItem(&twi->cached_xfr_list_base,&twi_xfr->list_item);
    twi->cached_xfr_cnt--;

    // Clean up
    AXICAT_AL_Free(twi_xfr);
}


/*-----------------------------------------------------------------------------
VOID  TWI_Xfr_Unassign (TWI_XFR *twi_xfr)
-----------------------------------------------------------------------------*/


static
VOID  TWI_Xfr_Unassign (TWI_XFR *twi_xfr)
{
    TWI_CTX    *twi;

    // Init
    twi = &twi_xfr->ctx->twi;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&twi->assigned_xfr_list_base,&twi_xfr->list_item);
    twi->assigned_xfr_cnt--;

    // TODO:
    // * Decide whether to cache or destroy the transfer
    if (1)
    {
        // Link to the list of cached transfer objects
        LDLLLinkTailItem(&twi->cached_xfr_list_base,&twi_xfr->list_item);
        twi->cached_xfr_cnt++;
        twi_xfr->loc = TWI_XFR_LOC_CACHED;
    }
}


/*-----------------------------------------------------------------------------
VOID  TWI_Xfr_Master_Complete (TWI_XFR *twi_xfr, U8 status)

Complete a scheduled TWI master transfer with the given status.

The transfer is moved to the list of assigned transfers. If the detached flag
is set, the transfer is unassigned as well.
-----------------------------------------------------------------------------*/


static
VOID  TWI_Xfr_Master_Complete (TWI_XFR *twi_xfr, U8 status)
{
    TWI_CTX   *twi;

    // Init
    twi = &twi_xfr->ctx->twi;

    if (twi->cur_m_tx_xfr == twi_xfr)
    {
        twi->cur_m_tx_xfr = TWI_Xfr_Get_Next(twi->cur_m_tx_xfr);
    }

    // Unlink from the list of scheduled master transfer objects
    LDLLUnlinkItem(&twi->sch_m_xfr_list_base,&twi_xfr->list_item);
    twi->sch_m_xfr_cnt--;

    // Link to the list of assigned transfer objects
    LDLLLinkTailItem(&twi->assigned_xfr_list_base,&twi_xfr->list_item);
    twi->assigned_xfr_cnt++;
    twi_xfr->loc = TWI_XFR_LOC_ASSIGNED;

    // Report completion information
    twi_xfr->user.xfrd     = twi_xfr->xfrd;
    twi_xfr->user.nack_rsp = twi_xfr->nack_rsp;
    twi_xfr->user.status   = status;

    // Unassign the transfer object if the user has "destroyed" it earlier
    if (twi_xfr->detached)
    {
        TWI_Xfr_Unassign(twi_xfr);
    }
}


/*-----------------------------------------------------------------------------
VOID  TWI_Xfr_Slave_Tx_Complete (TWI_XFR *twi_xfr, U8 status)

Complete a scheduled TWI slave Tx transfer with the given status.

The transfer is moved to the list of assigned transfers. If the detached flag
is set, the transfer is unassigned as well.
-----------------------------------------------------------------------------*/


static
VOID  TWI_Xfr_Slave_Tx_Complete (TWI_XFR *twi_xfr, U8 status)
{
    TWI_CTX   *twi;

    // Init
    twi = &twi_xfr->ctx->twi;

    if (twi->cur_stx_tx_xfr == twi_xfr)
    {
        twi->cur_stx_tx_xfr = TWI_Xfr_Get_Next(twi->cur_stx_tx_xfr);
    }

    // Unlink from the list of scheduled slave Tx transfer objects
    LDLLUnlinkItem(&twi->sch_stx_xfr_list_base,&twi_xfr->list_item);
    twi->sch_stx_xfr_cnt--;

    // Link to the list of assigned transfer objects
    LDLLLinkTailItem(&twi->assigned_xfr_list_base,&twi_xfr->list_item);
    twi->assigned_xfr_cnt++;
    twi_xfr->loc = TWI_XFR_LOC_ASSIGNED;

    // Report completion information
    twi_xfr->user.xfrd     = twi_xfr->xfrd;
    twi_xfr->user.nack_rsp = twi_xfr->nack_rsp;
    twi_xfr->user.status   = status;

    // Unassign the transfer object if the user has "destroyed" it earlier
    if (twi_xfr->detached)
    {
        TWI_Xfr_Unassign(twi_xfr);
    }
}


/*-----------------------------------------------------------------------------
VOID  TWI_Xfr_Slave_Rx_Complete (TWI_XFR *twi_xfr, U8 status)

Complete a scheduled TWI slave Rx transfer with the given status.

The transfer is moved to the list of assigned transfers. If the detached flag
is set, the transfer is unassigned as well.
-----------------------------------------------------------------------------*/


static
VOID  TWI_Xfr_Slave_Rx_Complete (TWI_XFR *twi_xfr, U8 status)
{
    TWI_CTX   *twi;

    // Init
    twi = &twi_xfr->ctx->twi;

    if (twi->cur_srx_tx_xfr == twi_xfr)
    {
        twi->cur_srx_tx_xfr = TWI_Xfr_Get_Next(twi->cur_srx_tx_xfr);
    }

    // Unlink from the list of scheduled slave Tx transfer objects
    LDLLUnlinkItem(&twi->sch_srx_xfr_list_base,&twi_xfr->list_item);
    twi->sch_srx_xfr_cnt--;

    // Link to the list of assigned transfer objects
    LDLLLinkTailItem(&twi->assigned_xfr_list_base,&twi_xfr->list_item);
    twi->assigned_xfr_cnt++;
    twi_xfr->loc = TWI_XFR_LOC_ASSIGNED;

    // Report completion information
    twi_xfr->user.xfrd   = twi_xfr->xfrd;
    twi_xfr->user.status = status;

    // Unassign the transfer object if the user has "destroyed" it earlier
    if (twi_xfr->detached)
    {
        TWI_Xfr_Unassign(twi_xfr);
    }
}


/*-----------------------------------------------------------------------------
VOID  TWI_Complete_All_Xfrs (TWI_CTX *twi)

Complete all scheduled transfers forcibly, nomatter they're active or not.

This function is called when the AxiCat is deemed detached.
-----------------------------------------------------------------------------*/


static
VOID  TWI_Complete_All_Xfrs (TWI_CTX *twi)
{
    TWI_XFR    *twi_xfr;
    LDLL_ITEM  *item;

    // Iterate all scheduled master transfers until the list has depleted
    while ((item = LDLLGetHeadItem(&twi->sch_m_xfr_list_base)) != 0)
    {
        twi_xfr = GetContAd(item,TWI_XFR,list_item);

        // Complete as cancelled
        TWI_Xfr_Master_Complete(twi_xfr,AXICAT_AL_TWI_ST_CANCELLED);
    }

    // Iterate all scheduled slave Tx transfers until the list has depleted
    while ((item = LDLLGetHeadItem(&twi->sch_stx_xfr_list_base)) != 0)
    {
        twi_xfr = GetContAd(item,TWI_XFR,list_item);

        // Complete as cancelled
        TWI_Xfr_Slave_Tx_Complete(twi_xfr,AXICAT_AL_TWI_ST_CANCELLED);
    }

    // Iterate all scheduled slave Rx transfers until the list has depleted
    while ((item = LDLLGetHeadItem(&twi->sch_srx_xfr_list_base)) != 0)
    {
        twi_xfr = GetContAd(item,TWI_XFR,list_item);

        // Complete as cancelled
        TWI_Xfr_Slave_Rx_Complete(twi_xfr,AXICAT_AL_TWI_ST_CANCELLED);
    }
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Xfr_Cancel (TWI_XFR *twi_xfr)

Try to cancel the given transfer. If the transfer can be cancelled, it's
completed with status CANCELLED.

The transfer can only be cancelled when it's scheduled and non-active.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Xfr_Cancel (TWI_XFR *twi_xfr)
{
    TWI_CTX   *twi;

    // Get the TWI context
    twi = &twi_xfr->ctx->twi;

    if (twi_xfr->loc == TWI_XFR_LOC_SCHEDULED_M)
    {
        if (twi_xfr->active)
        {
            // Can't cancel. One or more command packets have been transmitted,
            // and we're waiting for the response packet(s).
        }
        else
        {
            // Complete as cancelled
            TWI_Xfr_Master_Complete(twi_xfr,AXICAT_AL_TWI_ST_CANCELLED);
        }
    }
    else
    if (twi_xfr->loc == TWI_XFR_LOC_SCHEDULED_STX)
    {
        if (twi_xfr->active)
        {
            // Can't cancel. One or more command packets have been transmitted,
            // and we're waiting for the response packet(s).
        }
        else
        {
            // Complete as cancelled
            TWI_Xfr_Slave_Tx_Complete(twi_xfr,AXICAT_AL_TWI_ST_CANCELLED);
        }
    }
    else
    if (twi_xfr->loc == TWI_XFR_LOC_SCHEDULED_SRX)
    {
        if (twi_xfr->active)
        {
            // Can't cancel. One or more command packets have been transmitted,
            // and we're waiting for the response packet(s).
        }
        else
        {
            // Complete as cancelled
            TWI_Xfr_Slave_Rx_Complete(twi_xfr,AXICAT_AL_TWI_ST_CANCELLED);
        }
    }
    else
    {
        // Can't cancel a transfer that's not scheduled
        return 0;
    }

    return 1;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_TWI_Xfr_Cancel (AXICAT_AL_TWI_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_TWI_Xfr_Cancel (AXICAT_AL_TWI_XFR *xfr)
{
    TWI_XFR   *twi_xfr;

    // Get the containing transfer context
    twi_xfr = GetContAd(xfr,TWI_XFR,user);

    TWI_Xfr_Cancel(twi_xfr);
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_TWI_Xfr_Destroy (AXICAT_AL_TWI_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_TWI_Xfr_Destroy (AXICAT_AL_TWI_XFR *xfr)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;

    if (!xfr) return;

    // Init
    twi_xfr = GetContAd(xfr,TWI_XFR,user);
    twi     = &twi_xfr->ctx->twi;

    // Cancel the transfer
    TWI_Xfr_Cancel(twi_xfr);

    if (twi_xfr->loc == TWI_XFR_LOC_ASSIGNED)
    {
        // The transfer was cancelled. Unassign the object (cache or destroy).
        TWI_Xfr_Unassign(twi_xfr);
    }
    else
    {
        // The transfer wasn't cancelled. Mark as detached and unassign later,
        // upon completion.
        twi_xfr->detached = 1;
    }
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
AXICAT_AL_TWI_XFR *
DynllCall  AXICAT_AL_TWI_Xfr_Create (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
AXICAT_AL_TWI_XFR *
DynllCall  AXICAT_AL_TWI_Xfr_Create (AXICAT_AL_HANDLE handle)
{
    CTX        *ctx;
    TWI_CTX    *twi;
    TWI_XFR    *twi_xfr;
    LDLL_ITEM  *item;

    // Init
    ctx = (CTX*)handle;
    twi = &ctx->twi;

    if (twi->cached_xfr_cnt > 0)
    {
        // Take a TWI transfer object from the pool of cached transfers

        // Unlink from the list of cached transfer objects
        item = LDLLUnlinkHeadItem(&twi->cached_xfr_list_base);
        twi->cached_xfr_cnt--;

        twi_xfr = GetContAd(item,TWI_XFR,list_item);
    }
    else
    {
        // Allocate a TWI transfer object
        twi_xfr = AXICAT_AL_Alloc_Clear(sizeof(TWI_XFR));
        if (!twi_xfr) return 0;

        // Set up
        twi_xfr->ctx = ctx;
    }

    // Move the transfer object to the list of assigned transfer objects
    LDLLLinkTailItem(&twi->assigned_xfr_list_base,&twi_xfr->list_item);
    twi->assigned_xfr_cnt++;
    twi_xfr->loc = TWI_XFR_LOC_ASSIGNED;

    return &twi_xfr->user;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Xfr_Master_Schedule (AXICAT_AL_TWI_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Xfr_Master_Schedule (AXICAT_AL_TWI_XFR *xfr)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;

    // Check arguments
    if ((xfr->dir == AXICAT_AL_TWI_DIR_READ) && (xfr->sc == 0)) goto Err;

    // Get the containing transfer context
    twi_xfr = GetContAd(xfr,TWI_XFR,user);

    if (twi_xfr->loc != TWI_XFR_LOC_ASSIGNED) goto Err;

    // Get the TWI context
    twi = &twi_xfr->ctx->twi;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&twi->assigned_xfr_list_base,&twi_xfr->list_item);
    twi->assigned_xfr_cnt--;

    // Link to the list of scheduled master transfer objects
    LDLLLinkTailItem(&twi->sch_m_xfr_list_base,&twi_xfr->list_item);
    twi->sch_m_xfr_cnt++;
    twi_xfr->loc = TWI_XFR_LOC_SCHEDULED_M;

    // Set as the current Tx transfer if none is present
    if (!twi->cur_m_tx_xfr) twi->cur_m_tx_xfr = twi_xfr;

    // Set up for the scheduled state
    twi_xfr->active       = 0;
    twi_xfr->detached     = 0;
    twi_xfr->buf          = xfr->buf;
    twi_xfr->buf_sc       = xfr->sc;
    twi_xfr->cmd_byte_cnt = 0;
    twi_xfr->rsp_byte_cnt = 0;
    twi_xfr->xfrd         = 0;
    twi_xfr->slave_ad     = xfr->slave_ad;
    twi_xfr->dir          = xfr->dir;
    twi_xfr->force_stop   = xfr->force_stop;
    twi_xfr->tx_phase     = 0;
    twi_xfr->rx_phase     = 0;
    twi_xfr->nack_rsp     = 0;
    twi_xfr->short_packet = 0;
    twi_xfr->skipped      = 0;

    // User data
    twi_xfr->user.status = AXICAT_AL_TWI_ST_SCHEDULED;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Tx_Master (CTX *ctx)

Try to transmit master command packets to the TWI part in the AxiCat.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Tx_Master (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    U8          u;
    U32         hw_left;
    U32         cnt;
    FLAG        last;
    U8          data_byte;


    // Init
    twi = &ctx->twi;


  Loop:


    // Process transmission of master stop command

    if (twi->hw_m_tx_stop)
    {
        // Send master stop command packet

        // A stop command occupies 1 byte in the AxiCat's buffer
        if (twi->hw_m_buf_free < 1) goto Stop;


        twi->hw_m_buf_free -= 1;


        // Transmit command packet

        // Command packet bytes:
        // +00: ________ b: Command code

        if (!Tx_Byte(ctx,AXICAT_CMD_TWI_MASTER_STOP)) goto Err;


        twi->hw_m_tx_stop = 0;
    }


    // Process the current master transfer, if any

    twi_xfr = twi->cur_m_tx_xfr;
    if (!twi_xfr) goto Stop;


    // If the TWI transfer is marked as short-packet, then we block further
    // processing of the TWI transfer and any further transfers. Instead,
    // we wait for remaining response packets and the current TWI transfer will
    // be ultimately completed.
    //
    if (twi_xfr->short_packet) goto Stop;


    // Dispatch phase
    if (twi_xfr->tx_phase == 0) goto Phase_Start; else goto Phase_Data;


  Phase_Start:

    // Send master start command packet

    // A start command occupies 2 bytes in the AxiCat's buffer
    if (twi->hw_m_buf_free < 2) goto Stop;


    // Send command packet

    // Command packet bytes:
    // +00: ________ b: Command code
    // +01: nnnnnnnd b: Slave address, write(0)/read(1)

    twi->hw_m_buf_free -= 2;

    u = twi_xfr->slave_ad << 1;
    if (twi_xfr->dir == AXICAT_AL_TWI_DIR_READ) u |= 0x01;

    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_MASTER_START)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;


    // Mark as active (if not already set)
    twi_xfr->active = 1;


    // Next phase

    if ((twi_xfr->dir == AXICAT_AL_TWI_DIR_WRITE) && (twi_xfr->buf_sc == 0))
    {
        // I2C probe command, don't send Tx command packet
        goto Next_Xfr;
    }

    // Proceed with data phase
    twi_xfr->tx_phase = 1;


  Phase_Data:

    // Send master Tx or Rx command packet

    // A Tx or Rx command occupies at least 4 bytes in the AxiCat's
    // buffer (3 control bytes plus the data bytes)
    if (twi->hw_m_buf_free < 4) goto Stop;

    // Room left for data bytes in a command packet (1..)
    hw_left = twi->hw_m_buf_free - 3;
    if (hw_left > 32) hw_left = 32;

    // Data bytes remaining to be transfered (1..)
    last = 1;
    cnt  = twi_xfr->buf_sc - twi_xfr->cmd_byte_cnt;

    // The command packet's max. payload in 32 bytes
    if (cnt > 32) { cnt = 32; last = 0; }

    // The command packet must fit in the free space in the AxiCat's buffer
    if (cnt > hw_left) { cnt = hw_left; last = 0; }


    // TEST. - send smaller data payloads
    //if (cnt > 2) { cnt = 2; last = 0; }


    twi->hw_m_buf_free -= (3 + cnt);


    if (twi_xfr->dir == AXICAT_AL_TWI_DIR_WRITE)
    {
        // Send master Tx command packet

        // Command packet bytes:
        // +00: ________ b: Command code
        // +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
        // +02: <bytes>

        u = (U8)cnt - 1;

        if (!Tx_Byte(ctx,AXICAT_CMD_TWI_MASTER_TX)) goto Err;
        if (!Tx_Byte(ctx,u)) goto Err;

        for (;;)
        {
            data_byte = (!twi_xfr->detached) ? twi_xfr->buf[twi_xfr->cmd_byte_cnt] : 0xFF;
            twi_xfr->cmd_byte_cnt++;

            if (!Tx_Byte(ctx,data_byte)) goto Err;

            cnt--;
            if (cnt == 0) break;
        }
    }
    else
    {
        // Send master Rx command packet

        // Command packet bytes:
        // +00: ________ b: Command code
        // +01: m00nnnnn b: Last packet, bytes minus one (0..31, corresponding with 1..32)

        u = (U8)cnt - 1;
        if (last) u |= 0x80;

        if (!Tx_Byte(ctx,AXICAT_CMD_TWI_MASTER_RX)) goto Err;
        if (!Tx_Byte(ctx,u)) goto Err;

        twi_xfr->cmd_byte_cnt += cnt;
    }


    if (twi_xfr->cmd_byte_cnt < twi_xfr->buf_sc) goto Phase_Data;


  Next_Xfr:

    // Proceed with the next scheduled transfer (if any) and determine if a
    // master stop command has to be transmitted.
    //
    // Variable twi->hw_m_tx_stop is zero at this point.

    twi->hw_m_tx_stop = twi_xfr->force_stop;

    // Next transfer (if any)
    twi->cur_m_tx_xfr = TWI_Xfr_Get_Next(twi->cur_m_tx_xfr);

    // If no next transfer is available, send a stop command
    if (!twi->cur_m_tx_xfr) twi->hw_m_tx_stop = 1;

    // Don't send a stop command if the TWI is disabled
    if (!twi->hw_enabled) twi->hw_m_tx_stop = 0;


    goto Loop;


  Stop:

    return 1;


  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Xfr_Master_Chk_Complete (TWI_XFR *twi_xfr)
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Xfr_Master_Chk_Complete (TWI_XFR *twi_xfr)
{
    TWI_CTX    *twi;
    U8          status;

    // Can't complete if Tx phase and Rx phase differ
    if (twi_xfr->rx_phase != twi_xfr->tx_phase) return 0;

    // The following checks are true for start phase and data phase
    if (twi_xfr->short_packet)
    {
        if (twi_xfr->rsp_byte_cnt == twi_xfr->cmd_byte_cnt)
        {
            goto Complete;
        }
    }
    else
    {
        if (twi_xfr->rsp_byte_cnt == twi_xfr->buf_sc)
        {
            goto Complete;
        }
    }

    // Transfer not completed
    return 0;

  Complete:

    // Get the TWI context
    twi = &twi_xfr->ctx->twi;

    // Determine the resulting status code.
    //
    // The order followed here is important, as well as the fact that all
    // flags that influence the status are cleared.

    status = AXICAT_AL_TWI_ST_SUCCESS;

    if (twi_xfr->skipped)
    {
        twi_xfr->skipped = 0;

        status = AXICAT_AL_TWI_ST_SKIPPED;
    }

    if (twi->arb_lost)
    {
        twi->arb_lost = 0;

        status = AXICAT_AL_TWI_ST_ARB_LOST;
    }

    if (twi->bus_error)
    {
        twi->bus_error = 0;

        status = AXICAT_AL_TWI_ST_BUS_ERROR;
    }

    TWI_Xfr_Master_Complete(twi_xfr,status);
    return 1;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Rx_Start (CTX *ctx)

This function is called when a master start response packet has been received.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Rx_Start (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    LDLL_ITEM  *item;

    // Init
    twi = &ctx->twi;

    // Get the first scheduled master transfer object
    item = LDLLGetHeadItem(&twi->sch_m_xfr_list_base);
    if (!item) goto Err;
    twi_xfr = GetContAd(item,TWI_XFR,list_item);

    // Consistency check - check phase
    if (twi_xfr->rx_phase != 0) goto Err;

    // Consistency check
    if (ctx->rx.twi_start.slave_ad != twi_xfr->slave_ad) goto Err;
    if (ctx->rx.twi_start.dir != twi_xfr->dir) goto Err;

    if (ctx->rx.twi_start.nack_rsp)
    {
        twi_xfr->nack_rsp = 1;
    }

    if (ctx->rx.twi_start.skipped)
    {
        // Track skipped start command
        twi_xfr->skipped = 1;

        // Mark as short-packet
        //
        // Note: twi_xfr->xfrd is zero, and that's what is needed here.
        //
        twi_xfr->short_packet = 1;
    }

    // Complete the transfer if possible. Possible reasons for completion:
    // * Skipped (the transfer is marked as short-packet).
    // * Tx of zero byte (I2C probe).
    //
    if (TWI_Xfr_Master_Chk_Complete(twi_xfr)) goto Success;

    // Mark data phase
    twi_xfr->rx_phase = 1;

  Success:

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Rx_Master_Tx_Rsp (CTX *ctx)

This function is called when a master Tx response packet has been received.

Note that a master Tx response packet doesn't carry data bytes.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Rx_Master_Tx_Rsp (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    LDLL_ITEM  *item;

    // Init
    twi = &ctx->twi;

/*
    // DBG.
    printf("MTX: cnt %d, sent %d, nack %d, sp %d\n",
           ctx->rx.twi_mtx.cnt,
           ctx->rx.twi_mtx.sent,
           ctx->rx.twi_mtx.nack_rsp,
           ctx->rx.twi_mtx.short_packet);
*/

    // Get the first scheduled master transfer object
    item = LDLLGetHeadItem(&twi->sch_m_xfr_list_base);
    if (!item) goto Err;
    twi_xfr = GetContAd(item,TWI_XFR,list_item);

    // Consistency check - check phase
    if (twi_xfr->rx_phase != 1) goto Err;

    twi_xfr->rsp_byte_cnt += ctx->rx.twi_mtx.cnt;

    twi_xfr->xfrd += ctx->rx.twi_mtx.sent;

    if (ctx->rx.twi_mtx.nack_rsp)
    {
        twi_xfr->nack_rsp = 1;
    }

    if (ctx->rx.twi_mtx.short_packet)
    {
        // Mark as short-packet
        twi_xfr->short_packet = 1;
    }
    else
    {
        // Consistency check
        if (twi_xfr->short_packet) goto Err;
    }

    // Complete the transfer if possible
    TWI_Xfr_Master_Chk_Complete(twi_xfr);

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Rx_Master_Rx_Rsp (CTX *ctx)

This function is called when a master Rx response packet, without data payload,
has been received.

There may or may not follow a data payload.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Rx_Master_Rx_Rsp (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    LDLL_ITEM  *item;

    // Init
    twi = &ctx->twi;

/*
    // DBG.
    printf("MRX: cnt %d, rcvd %d\n",
           ctx->rx.twi_mrx.cnt,
           ctx->rx.twi_mrx.rcvd);
*/

    // Get the first scheduled master transfer object
    item = LDLLGetHeadItem(&twi->sch_m_xfr_list_base);
    if (!item) goto Err;
    twi_xfr = GetContAd(item,TWI_XFR,list_item);

    // Consistency check - check phase
    if (twi_xfr->rx_phase != 1) goto Err;

    twi_xfr->rsp_byte_cnt += ctx->rx.twi_mrx.cnt;

    if (ctx->rx.twi_mrx.nack_rsp)
    {
        twi_xfr->nack_rsp = 1;
    }

    if (ctx->rx.twi_mrx.short_packet)
    {
        // Mark as short-packet
        twi_xfr->short_packet = 1;
    }
    else
    {
        // Consistency check
        if (twi_xfr->short_packet) goto Err;
    }

    // If no data payload will follow, try to complete the transfer here
    if (ctx->rx.twi_mrx.rcvd == 0)
    {
        // Complete the transfer if possible
        TWI_Xfr_Master_Chk_Complete(twi_xfr);
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Rx_Master_Rx_Byte (CTX *ctx, U8 data_byte)

This function processes an incoming data byte of a master Rx response packet.

When the last byte of the payload comes in, the function tries to complete the
transfer.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Rx_Master_Rx_Byte (CTX *ctx, U8 data_byte)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    LDLL_ITEM  *item;

    // Init
    twi = &ctx->twi;

    // Get the first scheduled master transfer object
    item = LDLLGetHeadItem(&twi->sch_m_xfr_list_base);
    if (!item) goto Err;
    twi_xfr = GetContAd(item,TWI_XFR,list_item);

    // Consistency check - check phase
    if (twi_xfr->rx_phase != 1) goto Err;

    // Store the data byte
    if (!twi_xfr->detached) twi_xfr->buf[twi_xfr->xfrd] = data_byte;

    twi_xfr->xfrd++;

    if (ctx->rx.twi_mrx.rcvd == 0)
    {
        // Last byte received

        // Complete the transfer if possible
        TWI_Xfr_Master_Chk_Complete(twi_xfr);
    }
    else
    {
        // One or more data bytes will follow; check for buffer overflow
        if (twi_xfr->xfrd == twi_xfr->buf_sc) goto Err;
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Xfr_Slave_Schedule (AXICAT_AL_TWI_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Xfr_Slave_Schedule (AXICAT_AL_TWI_XFR *xfr)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;

    // Check arguments
    if (xfr->sc == 0) goto Err;

    // Get the containing transfer context
    twi_xfr = GetContAd(xfr,TWI_XFR,user);

    if (twi_xfr->loc != TWI_XFR_LOC_ASSIGNED) goto Err;

    // Get the TWI context
    twi = &twi_xfr->ctx->twi;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&twi->assigned_xfr_list_base,&twi_xfr->list_item);
    twi->assigned_xfr_cnt--;

    if (xfr->dir == AXICAT_AL_TWI_DIR_WRITE)
    {
        // Link to the list of scheduled slave Tx transfer objects
        LDLLLinkTailItem(&twi->sch_stx_xfr_list_base,&twi_xfr->list_item);
        twi->sch_stx_xfr_cnt++;
        twi_xfr->loc = TWI_XFR_LOC_SCHEDULED_STX;

        // Set as the current Tx transfer if none is present
        if (!twi->cur_stx_tx_xfr) twi->cur_stx_tx_xfr = twi_xfr;
    }
    else
    {
        // Link to the list of scheduled slave Rx transfer objects
        LDLLLinkTailItem(&twi->sch_srx_xfr_list_base,&twi_xfr->list_item);
        twi->sch_srx_xfr_cnt++;
        twi_xfr->loc = TWI_XFR_LOC_SCHEDULED_SRX;

        // Set as the current Tx transfer if none is present
        if (!twi->cur_srx_tx_xfr) twi->cur_srx_tx_xfr = twi_xfr;
    }

    // Set up for the scheduled state
    twi_xfr->active       = 0;
    twi_xfr->detached     = 0;
    twi_xfr->buf          = xfr->buf;
    twi_xfr->buf_sc       = xfr->sc;
    twi_xfr->cmd_byte_cnt = 0;
    twi_xfr->rsp_byte_cnt = 0;
    twi_xfr->xfrd         = 0;
    twi_xfr->last         = xfr->last;
    twi_xfr->short_packet = 0;
    twi_xfr->skipped      = 0;

    // User data
    twi_xfr->user.status = AXICAT_AL_TWI_ST_SCHEDULED;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Tx_Slave_Tx (CTX *ctx)

Try to transmit slave Tx command packets to the TWI part in the AxiCat.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Tx_Slave_Tx (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    U8          u;
    U32         hw_left;
    U32         cnt;
    FLAG        last;
    U8          data_byte;


    // Init
    twi = &ctx->twi;


  Loop:


    // Process the current slave Tx transfer, if any

    twi_xfr = twi->cur_stx_tx_xfr;
    if (!twi_xfr) goto Stop;


    // If the TWI transfer is marked as short-packet, then we block further
    // processing of the TWI transfer and any further transfers. Instead,
    // we wait for remaining response packets and the current TWI transfer will
    // be ultimately completed.
    //
    if (twi_xfr->short_packet) goto Stop;


  Cmd:

    // Send slave Tx command packet

    // A Tx command occupies at least 3 bytes in the AxiCat's buffer (2 control
    // bytes plus the data bytes)
    if (twi->hw_stx_buf_free < 3) goto Stop;

    // Room left for data bytes in a command packet (1..)
    hw_left = twi->hw_stx_buf_free - 2;
    if (hw_left > 32) hw_left = 32;

    // Data bytes remaining to be transfered (1..)
    last = 1;
    cnt  = twi_xfr->buf_sc - twi_xfr->cmd_byte_cnt;

    // The command packet's max. payload in 32 bytes
    if (cnt > 32) { cnt = 32; last = 0; }

    // The command packet must fit in the free space in the AxiCat's buffer
    if (cnt > hw_left) { cnt = hw_left; last = 0; }


    // TEST. - send smaller data payloads
    //if (cnt > 2) { cnt = 2; last = 0; }


    twi->hw_stx_buf_free -= (2 + cnt);


    // Send slave Tx command packet

    // Command packet bytes:
    // +00: ________ b: Command code
    // +01: m00nnnnn b: Last packet, bytes minus one (0..31, corresponding with 1..32)
    // +02: <bytes>

    u = (U8)cnt - 1;
    if ((last) && (twi_xfr->last)) u |= 0x80;

    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_SLAVE_TX)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    for (;;)
    {
        data_byte = (!twi_xfr->detached) ? twi_xfr->buf[twi_xfr->cmd_byte_cnt] : 0xFF;
        twi_xfr->cmd_byte_cnt++;

        if (!Tx_Byte(ctx,data_byte)) goto Err;

        cnt--;
        if (cnt == 0) break;
    }


    // Mark as active (if not already set)
    twi_xfr->active = 1;


    if (twi_xfr->cmd_byte_cnt < twi_xfr->buf_sc) goto Cmd;


    // Proceed with the next scheduled transfer (if any)

    // Next transfer (if any)
    twi->cur_stx_tx_xfr = TWI_Xfr_Get_Next(twi->cur_stx_tx_xfr);

    goto Loop;


  Stop:

    return 1;


  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Xfr_Slave_Tx_Chk_Complete (TWI_XFR *twi_xfr)
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Xfr_Slave_Tx_Chk_Complete (TWI_XFR *twi_xfr)
{
    TWI_CTX    *twi;
    U8          status;

    if (twi_xfr->short_packet)
    {
        if (twi_xfr->rsp_byte_cnt == twi_xfr->cmd_byte_cnt)
        {
            if ((twi_xfr->last) && (twi_xfr->cmd_byte_cnt < twi_xfr->buf_sc))
            {
                // Send clear command for slave Tx
                if (!Tx_Byte(twi_xfr->ctx,AXICAT_CMD_TWI_CLEAR)) goto Err;
                if (!Tx_Byte(twi_xfr->ctx,0x01)) goto Err;
            }

            goto Complete;
        }
    }
    else
    {
        if (twi_xfr->rsp_byte_cnt == twi_xfr->buf_sc)
        {
            goto Complete;
        }
    }

    // Transfer not completed
    return 0;

  Complete:

    // Get the TWI context
    twi = &twi_xfr->ctx->twi;

    // Determine the resulting status code.
    //
    // The order followed here is important, as well as the fact that all
    // flags that influence the status are cleared.

    status = AXICAT_AL_TWI_ST_SUCCESS;

    if (twi_xfr->skipped)
    {
        twi_xfr->skipped = 0;

        status = AXICAT_AL_TWI_ST_SKIPPED;
    }

    if (twi->bus_error)
    {
        twi->bus_error = 0;

        status = AXICAT_AL_TWI_ST_BUS_ERROR;
    }

    TWI_Xfr_Slave_Tx_Complete(twi_xfr,status);
    return 1;

  Err:

    Mark_Detached(twi_xfr->ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Rx_Slave_Tx_Rsp (CTX *ctx)

This function is called when a slave Tx response packet has been received.

Note that a slave Tx response packet doesn't carry data bytes.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Rx_Slave_Tx_Rsp (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    LDLL_ITEM  *item;

    // Init
    twi = &ctx->twi;

    // Get the first scheduled slave Tx transfer object
    item = LDLLGetHeadItem(&twi->sch_stx_xfr_list_base);
    if (!item) goto Err;
    twi_xfr = GetContAd(item,TWI_XFR,list_item);

    twi_xfr->rsp_byte_cnt += ctx->rx.twi_stx.cnt;

    twi_xfr->xfrd += ctx->rx.twi_stx.sent;

    if (ctx->rx.twi_stx.nack_rsp)
    {
        twi_xfr->nack_rsp = 1;
    }

    if (ctx->rx.twi_stx.short_packet)
    {
        // Track skipped first command of payload
        if (twi_xfr->xfrd == 0) twi_xfr->skipped = 1;

        // Mark as short-packet
        twi_xfr->short_packet = 1;
    }
    else
    {
        // Consistency check
        if (twi_xfr->short_packet) goto Err;
    }

    // Complete the transfer if possible
    TWI_Xfr_Slave_Tx_Chk_Complete(twi_xfr);

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Tx_Slave_Rx (CTX *ctx)

Try to transmit slave Rx command packets to the TWI part in the AxiCat.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Tx_Slave_Rx (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    U8          u;
    U32         hw_left;
    U32         cnt;
    FLAG        last;


    // Init
    twi = &ctx->twi;


  Loop:


    // Process the current slave Rx transfer, if any

    twi_xfr = twi->cur_srx_tx_xfr;
    if (!twi_xfr) goto Stop;


    // If the TWI transfer is marked as short-packet, then we block further
    // processing of the TWI transfer and any further transfers. Instead,
    // we wait for remaining response packets and the current TWI transfer will
    // be ultimately completed.
    //
    if (twi_xfr->short_packet) goto Stop;


  Cmd:

    // Send slave Rx command packet

    // An Rx command occupies at least 3 bytes in the AxiCat's buffer (2
    // control bytes plus the data bytes)
    if (twi->hw_srx_buf_free < 3) goto Stop;

    // Room left for data bytes in a command packet (1..)
    hw_left = twi->hw_srx_buf_free - 2;
    if (hw_left > 32) hw_left = 32;

    // Data bytes remaining to be transfered (1..)
    last = 1;
    cnt  = twi_xfr->buf_sc - twi_xfr->cmd_byte_cnt;

    // The command packet's max. payload in 32 bytes
    if (cnt > 32) { cnt = 32; last = 0; }

    // The command packet must fit in the free space in the AxiCat's buffer
    if (cnt > hw_left) { cnt = hw_left; last = 0; }


    // TEST. - send smaller data payloads
    //if (cnt > 2) { cnt = 2; last = 0; }


    twi->hw_srx_buf_free -= (2 + cnt);


    // Send slave Rx command packet

    // Command packet bytes:
    // +00: ________ b: Command code
    // +01: m00nnnnn b: Last packet, bytes minus one (0..31, corresponding with 1..32)


    u = (U8)cnt - 1;
    if ((last) && (twi_xfr->last)) u |= 0x80;

    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_SLAVE_RX)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    twi_xfr->cmd_byte_cnt += cnt;


    // Mark as active (if not already set)
    twi_xfr->active = 1;


    if (twi_xfr->cmd_byte_cnt < twi_xfr->buf_sc) goto Cmd;


    // Proceed with the next scheduled transfer (if any)

    // Next transfer (if any)
    twi->cur_srx_tx_xfr = TWI_Xfr_Get_Next(twi->cur_srx_tx_xfr);

    goto Loop;


  Stop:

    return 1;


  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Xfr_Slave_Rx_Chk_Complete (TWI_XFR *twi_xfr)
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Xfr_Slave_Rx_Chk_Complete (TWI_XFR *twi_xfr)
{
    TWI_CTX    *twi;
    U8          status;

    if (twi_xfr->short_packet)
    {
        if (twi_xfr->rsp_byte_cnt == twi_xfr->cmd_byte_cnt)
        {
            if ((twi_xfr->last) && (twi_xfr->cmd_byte_cnt < twi_xfr->buf_sc))
            {
                // Send clear command for slave Rx
                if (!Tx_Byte(twi_xfr->ctx,AXICAT_CMD_TWI_CLEAR)) goto Err;
                if (!Tx_Byte(twi_xfr->ctx,0x02)) goto Err;
            }

            goto Complete;
        }
    }
    else
    {
        if (twi_xfr->rsp_byte_cnt == twi_xfr->buf_sc)
        {
            goto Complete;
        }
    }

    // Transfer not completed
    return 0;

  Complete:

    // Get the TWI context
    twi = &twi_xfr->ctx->twi;

    // Determine the resulting status code.
    //
    // The order followed here is important, as well as the fact that all
    // flags that influence the status are cleared.

    status = AXICAT_AL_TWI_ST_SUCCESS;

    if (twi_xfr->skipped)
    {
        twi_xfr->skipped = 0;

        status = AXICAT_AL_TWI_ST_SKIPPED;
    }

    if (twi->bus_error)
    {
        twi->bus_error = 0;

        status = AXICAT_AL_TWI_ST_BUS_ERROR;
    }

    TWI_Xfr_Slave_Rx_Complete(twi_xfr,status);
    return 1;

  Err:

    Mark_Detached(twi_xfr->ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Rx_Slave_Rx_Rsp (CTX *ctx)

This function is called when the header of a slave Rx response packet, thus
without data payload, has been received.

There may or may not follow a data payload.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Rx_Slave_Rx_Rsp (CTX *ctx)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    LDLL_ITEM  *item;

    // Init
    twi = &ctx->twi;

    // Get the first scheduled slave Rx transfer object
    item = LDLLGetHeadItem(&twi->sch_srx_xfr_list_base);
    if (!item) goto Err;
    twi_xfr = GetContAd(item,TWI_XFR,list_item);

    twi_xfr->rsp_byte_cnt += ctx->rx.twi_srx.cnt;

    if (ctx->rx.twi_srx.short_packet)
    {
        // Track skipped first command of payload
        if (twi_xfr->xfrd == 0) twi_xfr->skipped = 1;

        // Mark as short-packet
        twi_xfr->short_packet = 1;
    }
    else
    {
        // Consistency check
        if (twi_xfr->short_packet) goto Err;
    }

    // If no data payload will follow, try to complete the transfer here
    if (ctx->rx.twi_srx.rcvd == 0)
    {
        // Complete the transfer if possible
        TWI_Xfr_Slave_Rx_Chk_Complete(twi_xfr);
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  TWI_Rx_Slave_Rx_Byte (CTX *ctx, U8 data_byte)

This function processes an incoming data byte of a slave Rx response packet.

When the last byte of the payload comes in, the function tries to complete the
transfer.
-----------------------------------------------------------------------------*/


static
FLAG  TWI_Rx_Slave_Rx_Byte (CTX *ctx, U8 data_byte)
{
    TWI_XFR    *twi_xfr;
    TWI_CTX    *twi;
    LDLL_ITEM  *item;

    // Init
    twi = &ctx->twi;

    // Get the first scheduled master transfer object
    item = LDLLGetHeadItem(&twi->sch_srx_xfr_list_base);
    if (!item) goto Err;
    twi_xfr = GetContAd(item,TWI_XFR,list_item);

    // Store the data byte
    if (!twi_xfr->detached) twi_xfr->buf[twi_xfr->xfrd] = data_byte;

    twi_xfr->xfrd++;

    if (ctx->rx.twi_srx.rcvd == 0)
    {
        // Last byte received

        // Complete the transfer if possible
        TWI_Xfr_Slave_Rx_Chk_Complete(twi_xfr);
    }
    else
    {
        // One or more data bytes will follow; check for buffer overflow
        if (twi_xfr->xfrd == twi_xfr->buf_sc) goto Err;
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}
