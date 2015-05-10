
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al_spi.c
//
// AxiCat AL.
//
// This is an include file.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-03-21  Peter S'heeren, Axiris
//
//      * Created.
//
//   2014-07-07  Peter S'heeren, Axiris
//
//      * Released.
//
//   2014-09-01  Peter S'heeren, Axiris
//
//      * Fixed parameter checking in AXICAT_AL_SPI_Set_Speed_Raw().
//      * Data buffer of transfer isn't accessed when the transfer has been
//        detached from the user.
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
FLAG  DynllCall  AXICAT_AL_SPI_Enable (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Enable (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_SPI_ENABLE)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Disable (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Disable (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_SPI_DISABLE)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Set_Speed (AXICAT_AL_HANDLE handle, U32 speed)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Set_Speed (AXICAT_AL_HANDLE handle, U32 speed)
{
    CTX    *ctx;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    if (speed == 750000)  u = AXICAT_SPI_SPEED_750000; else
    if (speed == 1500000) u = AXICAT_SPI_SPEED_1500000; else
    if (speed == 3000000) u = AXICAT_SPI_SPEED_3000000; else
    if (speed == 6000000) u = AXICAT_SPI_SPEED_6000000; else
        goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_SPI_SET_SPEED)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Set_Speed_Raw
(
    AXICAT_AL_HANDLE    handle,
    U8                  cr,
    FLAG                x2
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Set_Speed_Raw
(
    AXICAT_AL_HANDLE    handle,
    U8                  cr,
    FLAG                x2
)
{
    CTX    *ctx;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (cr > 3) goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;


    // Send command packet

    u = cr;
    if (x2) u |= 0x04;

    if (!Tx_Byte(ctx,AXICAT_CMD_SPI_SET_SPEED_RAW)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;


    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Configure
(
    AXICAT_AL_HANDLE    handle,
    FLAG                polarity,
    FLAG                phase,
    FLAG                order
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Configure
(
    AXICAT_AL_HANDLE    handle,
    FLAG                polarity,
    FLAG                phase,
    FLAG                order
)
{
    CTX    *ctx;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;


    // Send command packet

    u = 0x00;
    if (polarity) u |= 0x01;
    if (phase)    u |= 0x02;
    if (order)    u |= 0x04;

    if (!Tx_Byte(ctx,AXICAT_CMD_SPI_SET_CFG)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;


    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
SPI_XFR  *SPI_Xfr_Get_Next (SPI_XFR *spi_xfr)
-----------------------------------------------------------------------------*/


static
SPI_XFR  *SPI_Xfr_Get_Next (SPI_XFR *spi_xfr)
{
    LDLL_ITEM  *item;

    item = LDLLGetNextItem(&spi_xfr->list_item);
    if (item)
        return GetContAd(item,SPI_XFR,list_item);
    else
        return 0;
}


/*-----------------------------------------------------------------------------
VOID  SPI_Xfr_Uncache (SPI_XFR *spi_xfr)

Remove the given transfer from the list of cached transfer objects and destroy
it.
-----------------------------------------------------------------------------*/


static
VOID  SPI_Xfr_Uncache (SPI_XFR *spi_xfr)
{
    SPI_CTX    *spi;

    // Init
    spi = &spi_xfr->ctx->spi;

    // Unlink from the list of cached transfer objects
    LDLLUnlinkItem(&spi->cached_xfr_list_base,&spi_xfr->list_item);
    spi->cached_xfr_cnt--;

    // Clean up
    AXICAT_AL_Free(spi_xfr);
}


/*-----------------------------------------------------------------------------
VOID  SPI_Xfr_Unassign (SPI_XFR *spi_xfr)
-----------------------------------------------------------------------------*/


static
VOID  SPI_Xfr_Unassign (SPI_XFR *spi_xfr)
{
    SPI_CTX    *spi;

    // Init
    spi = &spi_xfr->ctx->spi;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&spi->assigned_xfr_list_base,&spi_xfr->list_item);
    spi->assigned_xfr_cnt--;

    // TODO:
    // * Decide whether to cache or destroy the transfer
    if (1)
    {
        // Link to the list of cached transfer objects
        LDLLLinkTailItem(&spi->cached_xfr_list_base,&spi_xfr->list_item);
        spi->cached_xfr_cnt++;
        spi_xfr->loc = SPI_XFR_LOC_CACHED;
    }
}


/*-----------------------------------------------------------------------------
FLAG  SPI_Xfr_Complete (SPI_XFR *spi_xfr, U8 status)

Complete a scheduled SPI transfer with the given status.

The transfer is moved to the list of assigned transfers. If the detached flag
is set, the transfer is unassigned as well.
-----------------------------------------------------------------------------*/


static
VOID  SPI_Xfr_Complete (SPI_XFR *spi_xfr, U8 status)
{
    SPI_CTX   *spi;

    // Init
    spi = &spi_xfr->ctx->spi;

    if (spi->cur_tx_xfr == spi_xfr)
    {
        spi->cur_tx_xfr = SPI_Xfr_Get_Next(spi->cur_tx_xfr);
    }

    // Unlink from the list of scheduled transfer objects
    LDLLUnlinkItem(&spi->sch_xfr_list_base,&spi_xfr->list_item);
    spi->sch_xfr_cnt--;

    // Link to the list of assigned transfer objects
    LDLLLinkTailItem(&spi->assigned_xfr_list_base,&spi_xfr->list_item);
    spi->assigned_xfr_cnt++;
    spi_xfr->loc = SPI_XFR_LOC_ASSIGNED;

    // Report completion information
    spi_xfr->user.xfrd   = spi_xfr->xfrd;
    spi_xfr->user.status = status;

    // Unassign the transfer object if the user has "destroyed" it earlier
    if (spi_xfr->detached)
    {
        SPI_Xfr_Unassign(spi_xfr);
    }
}


/*-----------------------------------------------------------------------------
VOID  SPI_Complete_All_Xfrs (SPI_CTX *spi)

Complete all scheduled transfers forcibly, nomatter they're active or not.

This function is called when the AxiCat is deemed detached.
-----------------------------------------------------------------------------*/


static
VOID  SPI_Complete_All_Xfrs (SPI_CTX *spi)
{
    SPI_XFR    *spi_xfr;
    LDLL_ITEM  *item;

    // Iterate all scheduled transfers until the list has depleted
    while ((item = LDLLGetHeadItem(&spi->sch_xfr_list_base)) != 0)
    {
        spi_xfr = GetContAd(item,SPI_XFR,list_item);

        // Complete as cancelled
        SPI_Xfr_Complete(spi_xfr,AXICAT_AL_SPI_ST_CANCELLED);
    }
}


/*-----------------------------------------------------------------------------
FLAG  SPI_Xfr_Cancel (SPI_XFR *spi_xfr)

Try to cancel the given transfer. If the transfer can be cancelled, it's
completed with status CANCELLED.

The transfer can only be cancelled when it's scheduled and non-active.
-----------------------------------------------------------------------------*/


static
FLAG  SPI_Xfr_Cancel (SPI_XFR *spi_xfr)
{
    SPI_CTX   *spi;

    // Get the SPI context
    spi = &spi_xfr->ctx->spi;

    if (spi_xfr->loc != SPI_XFR_LOC_SCHEDULED)
    {
        // Can't cancel a transfer that's not scheduled
        return 0;
    }

    if (spi_xfr->active)
    {
        // Can't cancel. One or more command packets have been transmitted, and
        // we're waiting for the response packet(s).
    }
    else
    {
        // Complete as cancelled
        SPI_Xfr_Complete(spi_xfr,AXICAT_AL_SPI_ST_CANCELLED);
    }

    return 1;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_SPI_Xfr_Cancel (AXICAT_AL_SPI_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_SPI_Xfr_Cancel (AXICAT_AL_SPI_XFR *xfr)
{
    SPI_XFR   *spi_xfr;

    // Get the containing transfer context
    spi_xfr = GetContAd(xfr,SPI_XFR,user);

    SPI_Xfr_Cancel(spi_xfr);
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_SPI_Xfr_Destroy (AXICAT_AL_SPI_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_SPI_Xfr_Destroy (AXICAT_AL_SPI_XFR *xfr)
{
    SPI_XFR   *spi_xfr;
    SPI_CTX   *spi;

    if (!xfr) return;

    // Init
    spi_xfr = GetContAd(xfr,SPI_XFR,user);
    spi     = &spi_xfr->ctx->spi;

    // Cancel the transfer
    SPI_Xfr_Cancel(spi_xfr);

    if (spi_xfr->loc == SPI_XFR_LOC_ASSIGNED)
    {
        // The transfer was cancelled. Unassign the object (cache or destroy).
        SPI_Xfr_Unassign(spi_xfr);
    }
    else
    {
        // The transfer wasn't cancelled. Mark as detached and unassign later,
        // upon completion.
        spi_xfr->detached = 1;
    }
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
AXICAT_AL_SPI_XFR *
DynllCall  AXICAT_AL_SPI_Xfr_Create (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
AXICAT_AL_SPI_XFR *
DynllCall  AXICAT_AL_SPI_Xfr_Create (AXICAT_AL_HANDLE handle)
{
    CTX        *ctx;
    SPI_CTX    *spi;
    SPI_XFR    *spi_xfr;
    LDLL_ITEM  *item;

    // Init
    ctx = (CTX*)handle;
    spi = &ctx->spi;

    if (spi->cached_xfr_cnt > 0)
    {
        // Take an SPI transfer object from the pool of cached transfers

        // Unlink from the list of cached transfer objects
        item = LDLLUnlinkHeadItem(&spi->cached_xfr_list_base);
        spi->cached_xfr_cnt--;

        spi_xfr = GetContAd(item,SPI_XFR,list_item);
    }
    else
    {
        // Allocate an SPI transfer object
        spi_xfr = AXICAT_AL_Alloc_Clear(sizeof(SPI_XFR));
        if (!spi_xfr) return 0;

        // Set up
        spi_xfr->ctx = ctx;
    }

    // Move the transfer object to the list of assigned transfer objects
    LDLLLinkTailItem(&spi->assigned_xfr_list_base,&spi_xfr->list_item);
    spi->assigned_xfr_cnt++;
    spi_xfr->loc = SPI_XFR_LOC_ASSIGNED;

    return &spi_xfr->user;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Xfr_Schedule (AXICAT_AL_SPI_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Xfr_Schedule (AXICAT_AL_SPI_XFR *xfr)
{
    SPI_XFR    *spi_xfr;
    SPI_CTX    *spi;

    // Check arguments
    if (xfr->ss > 3) goto Err;
    if (xfr->sc == 0) goto Err;

    // Get the containing transfer context
    spi_xfr = GetContAd(xfr,SPI_XFR,user);

    if (spi_xfr->loc != SPI_XFR_LOC_ASSIGNED) goto Err;

    // Get the SPI context
    spi = &spi_xfr->ctx->spi;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&spi->assigned_xfr_list_base,&spi_xfr->list_item);
    spi->assigned_xfr_cnt--;

    // Link to the list of scheduled transfer objects
    LDLLLinkTailItem(&spi->sch_xfr_list_base,&spi_xfr->list_item);
    spi->sch_xfr_cnt++;
    spi_xfr->loc = SPI_XFR_LOC_SCHEDULED;

    // Set as the current Tx transfer if none is present
    if (!spi->cur_tx_xfr) spi->cur_tx_xfr = spi_xfr;

    // Set up for the scheduled state
    spi_xfr->active   = 0;
    spi_xfr->detached = 0;
    spi_xfr->buf      = xfr->buf;
    spi_xfr->buf_sc   = xfr->sc;
    spi_xfr->buf_fi   = 0;
    spi_xfr->buf_si   = 0;
    spi_xfr->xfrd     = 0;
    spi_xfr->ss       = xfr->ss;
    spi_xfr->skipped  = 0;

    // User data
    spi_xfr->user.status = AXICAT_AL_SPI_ST_SCHEDULED;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  SPI_Tx (CTX *ctx)

Try to transmit command packets to the SPI part in the AxiCat.
-----------------------------------------------------------------------------*/


static
FLAG  SPI_Tx (CTX *ctx)
{
    SPI_XFR    *spi_xfr;
    SPI_CTX    *spi;
    U32         cnt;
    U32         hw_left;
    FLAG        last;
    U8          u;
    U8          data_byte;

    // Init
    spi = &ctx->spi;

    while (spi->cur_tx_xfr)
    {
        spi_xfr = spi->cur_tx_xfr;

        // If the SPI transfer is marked as skipped, the we block further
        // processing of the SPI transfer and any further transfers. Instead,
        // we wait for remaining response packets and SPI_Rx_Skip() will
        // ultimately complete the current SPI transfer.
        //
        if (spi_xfr->skipped) break;

        // A command packet occupies at least 3 bytes in the AxiCat's buffer (2
        // control bytes plus the data bytes)
        if (spi->hw_buf_free < 3) break;

        // Room left for data bytes in a command packet (1..)
        hw_left = spi->hw_buf_free - 2;
        if (hw_left > 32) hw_left = 32;

        // Data bytes remaining to be transfered (1..)
        last = 1;
        cnt  = spi_xfr->buf_sc - spi_xfr->buf_fi;

        // The command packet's max. payload in 32 bytes
        if (cnt > 32) { cnt = 32; last = 0; }

        // The command packet must fit in the free space in the AxiCat's buffer
        if (cnt > hw_left) { cnt = hw_left; last = 0; }


        // Send command packet

        // Command packet bytes:
        // +00: ________ b: Command code
        // +01: mssnnnnn b: Last packet
        //                  Slave select (0..3)
        //                  Bytes minus one (0..31, corresponding with 1..32)
        // +02: <bytes>

        spi->hw_buf_free -= (2 + cnt);

        u = (U8)cnt - 1;
        u |= (spi_xfr->ss << 5);
        if (last) u |= 0x80;

        if (!Tx_Byte(ctx,AXICAT_CMD_SPI_XFR)) goto Err;
        if (!Tx_Byte(ctx,u)) goto Err;

        for (;;)
        {
            data_byte = (!spi_xfr->detached) ? spi_xfr->buf[spi_xfr->buf_fi] : 0xFF;
            spi_xfr->buf_fi++;

            if (!Tx_Byte(ctx,data_byte)) goto Err;

            cnt--;
            if (cnt == 0) break;
        }


        // Mark as active (if not already set)
        spi_xfr->active = 1;

        if (spi_xfr->buf_fi == spi_xfr->buf_sc)
        {
            // Next transfer (if any)
            spi->cur_tx_xfr = SPI_Xfr_Get_Next(spi->cur_tx_xfr);
        }
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  SPI_Rx_Chk_Rsp (CTX *ctx)
-----------------------------------------------------------------------------*/


static
FLAG  SPI_Rx_Chk_Rsp (CTX *ctx)
{
    SPI_XFR    *spi_xfr;
    SPI_CTX    *spi;
    LDLL_ITEM  *item;

    // Init
    spi = &ctx->spi;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&spi->sch_xfr_list_base);
    if (!item) goto Err;
    spi_xfr = GetContAd(item,SPI_XFR,list_item);

    // Check
    if (spi_xfr->ss != ctx->rx.spi_xfr.ss) goto Err;

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  SPI_Rx_Byte (CTX *ctx, U8 data_byte)

Process a data byte from an incoming response packet.
-----------------------------------------------------------------------------*/


static
FLAG  SPI_Rx_Byte (CTX *ctx, U8 data_byte)
{
    SPI_XFR    *spi_xfr;
    SPI_CTX    *spi;
    LDLL_ITEM  *item;
    FLAG        last_byte;

    // Init
    spi = &ctx->spi;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&spi->sch_xfr_list_base);
    if (!item) goto Err;
    spi_xfr = GetContAd(item,SPI_XFR,list_item);

    // The transfer mustn't be marked as skipped
    if (spi_xfr->skipped) goto Err;

    // Store the data byte
    if (!spi_xfr->detached) spi_xfr->buf[spi_xfr->buf_si] = data_byte;

    spi_xfr->buf_si++;

    last_byte = 0;
    if ((ctx->rx.spi_xfr.rcvd_mo == 0) && (ctx->rx.spi_xfr.last)) last_byte = 1;

    if (last_byte)
    {
        spi_xfr->xfrd = spi_xfr->buf_si;
        
        // Complete successfully
        SPI_Xfr_Complete(spi_xfr,AXICAT_AL_SPI_ST_SUCCESS);
    }
    else
    {
        // One or more data bytes will follow; check for buffer overflow
        if (spi_xfr->buf_si == spi_xfr->buf_sc) goto Err;
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  SPI_Rx_Skip (CTX *ctx)

Process an incoming response packet that's marked as skipped.
-----------------------------------------------------------------------------*/


static
FLAG  SPI_Rx_Skip (CTX *ctx)
{
    SPI_XFR    *spi_xfr;
    SPI_CTX    *spi;
    LDLL_ITEM  *item;

    // Init
    spi = &ctx->spi;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&spi->sch_xfr_list_base);
    if (!item) goto Err;
    spi_xfr = GetContAd(item,SPI_XFR,list_item);

    // Mark as skipped (if not already done)
    if (!spi_xfr->skipped)
    {
        spi_xfr->skipped = 1;
        spi_xfr->xfrd    = spi_xfr->buf_si;
    }

    spi_xfr->buf_si += (ctx->rx.spi_xfr.cnt_mo + 1);

    // An SPI command will be skipped only when command SPI DISABLE was sent to
    // the AxiCat. Note that this is different from TWI.
    //
    // An SPI transfer may spawn multiple SPI commands. If command SPI DISABLE
    // is issued, the current SPI transfer mustn't spawn any further SPI
    // commands and must be completed instead. Order:
    // [1] Flag spi_xfr->skipped is set, hence SPI_Tx() doesn't issue another
    //     SPI command for this SPI transfer.
    // [2] The following code check for the arrival of the last SPI response.

    if (spi_xfr->buf_si == spi_xfr->buf_fi)
    {
        // Complete as skipped
        SPI_Xfr_Complete(spi_xfr,AXICAT_AL_SPI_ST_SKIPPED);
    }
    else
    {
        if (spi_xfr->buf_si > spi_xfr->buf_fi) goto Err;
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}
