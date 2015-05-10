
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al_ow.c
//
// AxiCat AL.
//
// This is an include file.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-09-15  Peter S'heeren, Axiris
//
//      * Created.
//
//   2014-09-17  Peter S'heeren, Axiris
//
//      * Released v1.1.0.
//
//   2014-09-28  Peter S'heeren, Axiris
//
//      * Added 1-Wire enumeration.
//      * Released v1.2.0.
//
//   2014-12-12  Peter S'heeren, Axiris
//
//      * Added 1-Wire probing.
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
FLAG  DynllCall  AXICAT_AL_OW_Enable (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Enable (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_OW_ENABLE)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Disable (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Disable (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_OW_DISABLE)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
OW_XFR  *OW_Xfr_Get_Next (OW_XFR *ow_xfr)
-----------------------------------------------------------------------------*/


static
OW_XFR  *OW_Xfr_Get_Next (OW_XFR *ow_xfr)
{
    LDLL_ITEM  *item;

    item = LDLLGetNextItem(&ow_xfr->list_item);
    if (item)
        return GetContAd(item,OW_XFR,list_item);
    else
        return 0;
}


/*-----------------------------------------------------------------------------
VOID  OW_Xfr_Uncache (OW_XFR *ow_xfr)

Remove the given transfer from the list of cached transfer objects and destroy
it.
-----------------------------------------------------------------------------*/


static
VOID  OW_Xfr_Uncache (OW_XFR *ow_xfr)
{
    OW_CTX     *ow;

    // Init
    ow = &ow_xfr->ctx->ow;

    // Unlink from the list of cached transfer objects
    LDLLUnlinkItem(&ow->cached_xfr_list_base,&ow_xfr->list_item);
    ow->cached_xfr_cnt--;

    // Clean up
    AXICAT_AL_Free(ow_xfr);
}


/*-----------------------------------------------------------------------------
VOID  OW_Xfr_Unassign (OW_XFR *ow_xfr)
-----------------------------------------------------------------------------*/


static
VOID  OW_Xfr_Unassign (OW_XFR *ow_xfr)
{
    OW_CTX     *ow;

    // Init
    ow = &ow_xfr->ctx->ow;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&ow->assigned_xfr_list_base,&ow_xfr->list_item);
    ow->assigned_xfr_cnt--;

    // TODO:
    // * Decide whether to cache or destroy the transfer
    if (1)
    {
        // Link to the list of cached transfer objects
        LDLLLinkTailItem(&ow->cached_xfr_list_base,&ow_xfr->list_item);
        ow->cached_xfr_cnt++;
        ow_xfr->loc = OW_XFR_LOC_CACHED;
    }
}


/*-----------------------------------------------------------------------------
VOID  OW_Xfr_Complete (OW_XFR *ow_xfr, U8 status)

Complete a scheduled 1-Wire transfer with the given status.

The transfer is moved to the list of assigned transfers. If the detached flag
is set, the transfer is unassigned as well.
-----------------------------------------------------------------------------*/


static
VOID  OW_Xfr_Complete (OW_XFR *ow_xfr, U8 status)
{
    OW_CTX    *ow;

    // Init
    ow = &ow_xfr->ctx->ow;

    if (ow->cur_tx_xfr == ow_xfr)
    {
        ow->cur_tx_xfr = OW_Xfr_Get_Next(ow->cur_tx_xfr);
    }

    // Unlink from the list of scheduled transfer objects
    LDLLUnlinkItem(&ow->sch_xfr_list_base,&ow_xfr->list_item);
    ow->sch_xfr_cnt--;

    // Link to the list of assigned transfer objects
    LDLLLinkTailItem(&ow->assigned_xfr_list_base,&ow_xfr->list_item);
    ow->assigned_xfr_cnt++;
    ow_xfr->loc = OW_XFR_LOC_ASSIGNED;

    // Report completion information - generic
    ow_xfr->user.status = status;

    // Report completion information - specific per command
    if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_RESET)
    {
        // N.A.
    }
    else
    if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_TOUCH_BITS)
    {
        ow_xfr->user.touch_bits.xfrd = ow_xfr->touch_bits.xfrd;
    }
    else
    if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_ENUM)
    {
        // N.A.
    }

    // Unassign the transfer object if the user has "destroyed" it earlier
    if (ow_xfr->detached)
    {
        OW_Xfr_Unassign(ow_xfr);
    }
}


/*-----------------------------------------------------------------------------
VOID  OW_Complete_All_Xfrs (OW_CTX *ow)

Complete all scheduled transfers forcibly, nomatter they're active or not.

This function is called when the AxiCat is deemed detached.
-----------------------------------------------------------------------------*/


static
VOID  OW_Complete_All_Xfrs (OW_CTX *ow)
{
    OW_XFR     *ow_xfr;
    LDLL_ITEM  *item;

    // Iterate all scheduled transfers until the list has depleted
    while ((item = LDLLGetHeadItem(&ow->sch_xfr_list_base)) != 0)
    {
        ow_xfr = GetContAd(item,OW_XFR,list_item);

        // Complete as cancelled
        OW_Xfr_Complete(ow_xfr,AXICAT_AL_OW_ST_CANCELLED);
    }
}


/*-----------------------------------------------------------------------------
FLAG  OW_Xfr_Cancel (OW_XFR *ow_xfr)

Try to cancel the given transfer. If the transfer can be cancelled, it's
completed with status CANCELLED.

The transfer can only be cancelled when it's scheduled and non-active.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Xfr_Cancel (OW_XFR *ow_xfr)
{
    OW_CTX    *ow;

    // Get the 1-Wire context
    ow = &ow_xfr->ctx->ow;

    if (ow_xfr->loc != OW_XFR_LOC_SCHEDULED)
    {
        // Can't cancel a transfer that's not scheduled
        return 0;
    }

    if (ow_xfr->active)
    {
        // Can't cancel. One or more command packets have been transmitted, and
        // we're waiting for the response packet(s).
    }
    else
    {
        // Complete as cancelled
        OW_Xfr_Complete(ow_xfr,AXICAT_AL_OW_ST_CANCELLED);
    }

    return 1;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_OW_Xfr_Cancel (AXICAT_AL_OW_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_OW_Xfr_Cancel (AXICAT_AL_OW_XFR *xfr)
{
    OW_XFR    *ow_xfr;

    // Get the containing transfer context
    ow_xfr = GetContAd(xfr,OW_XFR,user);

    OW_Xfr_Cancel(ow_xfr);
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_OW_Xfr_Destroy (AXICAT_AL_OW_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_OW_Xfr_Destroy (AXICAT_AL_OW_XFR *xfr)
{
    OW_XFR    *ow_xfr;
    OW_CTX    *ow;

    if (!xfr) return;

    // Init
    ow_xfr = GetContAd(xfr,OW_XFR,user);
    ow     = &ow_xfr->ctx->ow;

    // Cancel the transfer
    OW_Xfr_Cancel(ow_xfr);

    if (ow_xfr->loc == OW_XFR_LOC_ASSIGNED)
    {
        // The transfer was cancelled. Unassign the object (cache or destroy).
        OW_Xfr_Unassign(ow_xfr);
    }
    else
    {
        // The transfer wasn't cancelled. Mark as detached and unassign later,
        // upon completion.
        ow_xfr->detached = 1;
    }
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
AXICAT_AL_OW_XFR *
DynllCall  AXICAT_AL_OW_Xfr_Create (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
AXICAT_AL_OW_XFR *
DynllCall  AXICAT_AL_OW_Xfr_Create (AXICAT_AL_HANDLE handle)
{
    CTX        *ctx;
    OW_CTX     *ow;
    OW_XFR     *ow_xfr;
    LDLL_ITEM  *item;

    // Init
    ctx = (CTX*)handle;
    ow  = &ctx->ow;

    if (ow->cached_xfr_cnt > 0)
    {
        // Take a 1-Wire transfer object from the pool of cached transfers

        // Unlink from the list of cached transfer objects
        item = LDLLUnlinkHeadItem(&ow->cached_xfr_list_base);
        ow->cached_xfr_cnt--;

        ow_xfr = GetContAd(item,OW_XFR,list_item);
    }
    else
    {
        // Allocate a 1-Wire transfer object
        ow_xfr = AXICAT_AL_Alloc_Clear(sizeof(OW_XFR));
        if (!ow_xfr) return 0;

        // Set up
        ow_xfr->ctx = ctx;
    }

    // Move the transfer object to the list of assigned transfer objects
    LDLLLinkTailItem(&ow->assigned_xfr_list_base,&ow_xfr->list_item);
    ow->assigned_xfr_cnt++;
    ow_xfr->loc = OW_XFR_LOC_ASSIGNED;

    return &ow_xfr->user;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Xfr_Schedule (AXICAT_AL_OW_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Xfr_Schedule (AXICAT_AL_OW_XFR *xfr)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;

    // Get the containing transfer context
    ow_xfr = GetContAd(xfr,OW_XFR,user);

    if (ow_xfr->loc != OW_XFR_LOC_ASSIGNED) goto Err;

    // Check arguments
    if (xfr->id == AXICAT_AL_OW_XFR_ID_TOUCH_BITS)
    {
        if (xfr->touch_bits.sc == 0) goto Err;
    }
    else
    if (xfr->id == AXICAT_AL_OW_XFR_ID_PROBE)
    {
        // Check the firmware version: v1.3.0 supports AXICAT_CMD_OW_PROBE
        if (ow_xfr->ctx->fw_version < AXICAT_FW_VERSION_1_3_0) goto Err;
    }

    // Get the 1-Wire context
    ow = &ow_xfr->ctx->ow;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&ow->assigned_xfr_list_base,&ow_xfr->list_item);
    ow->assigned_xfr_cnt--;

    // Link to the list of scheduled transfer objects
    LDLLLinkTailItem(&ow->sch_xfr_list_base,&ow_xfr->list_item);
    ow->sch_xfr_cnt++;
    ow_xfr->loc = OW_XFR_LOC_SCHEDULED;

    // Set as the current Tx transfer if none is present
    if (!ow->cur_tx_xfr) ow->cur_tx_xfr = ow_xfr;

    // Set up for the scheduled state
    ow_xfr->active            = 0;
    ow_xfr->detached          = 0;
    ow_xfr->buf               = xfr->buf;
    ow_xfr->active_packets    = 0;
    ow_xfr->skipped           = 0;
    ow_xfr->id                = xfr->id;

    // Set up specific fields
    if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_RESET)
    {
        // N.A.
    }
    else
    if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_TOUCH_BITS)
    {
        ow_xfr->touch_bits.buf_sc = xfr->touch_bits.sc;
        ow_xfr->touch_bits.buf_fi = 0;
        ow_xfr->touch_bits.buf_si = 0;
        ow_xfr->touch_bits.xfrd   = 0;
    }
    else
    if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_ENUM)
    {
        // N.A.
    }
    else
    if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_PROBE)
    {
        // N.A.
    }

    // User data
    ow_xfr->user.status = AXICAT_AL_OW_ST_SCHEDULED;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Tx (CTX *ctx)

Try to transmit command packets to the 1-Wire part in the AxiCat.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Tx (CTX *ctx)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;

    // Init
    ow = &ctx->ow;

    while (ow->cur_tx_xfr)
    {
        ow_xfr = ow->cur_tx_xfr;

        // If the 1-Wire transfer is marked as skipped, then we block further
        // processing of the 1-Wire transfer and any further transfers. Instead,
        // we wait for remaining response packets and OW_Rx_Skip_Xxx() will
        // ultimately complete the current 1-Wire transfer.
        //
        if (ow_xfr->skipped) break;

        if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_RESET)
        {
            // A RESET command occupies 1 byte in the AxiCat's buffer
            if (ow->hw_buf_free < 1) break;

            // Send command packet

            // Command packet bytes:
            // +00: ________ b: Command code

            ow->hw_buf_free--;

            if (!Tx_Byte(ctx,AXICAT_CMD_OW_RESET)) goto Err;

            // Mark as active
            ow_xfr->active = 1;

            // Next transfer (if any)
            ow->cur_tx_xfr = OW_Xfr_Get_Next(ow->cur_tx_xfr);
        }
        else
        if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_TOUCH_BITS)
        {
            U32     byte_cnt;
            U32     bit_cnt;
            U32     hw_bytes_left;
            U32     hw_bits_left;
            U8      u;
            U8      data_byte;
            U16     data_bits;

            // A TOUCH BITS command occupies at least 4 bytes in the
            // AxiCat's buffer (3 control bytes plus the data bytes)
            if (ow->hw_buf_free < 4) break;

            // Room left for data bytes in a command packet (1..)
            hw_bytes_left = ow->hw_buf_free - 3;
            if (hw_bytes_left > 16) hw_bytes_left = 16;

            // Data bits remaining to be transfered (1..)
            bit_cnt = ow_xfr->touch_bits.buf_sc - ow_xfr->touch_bits.buf_fi;

            // The command packet's max. payload is 128 bits
            if (bit_cnt > 128) bit_cnt = 128;

            // The command packet must fit in the free space in the AxiCat's buffer
            hw_bits_left = hw_bytes_left << 3;
            if (bit_cnt > hw_bits_left) bit_cnt = hw_bits_left;

            // Bytes spanning bits
            byte_cnt = ((bit_cnt - 1) >> 3) + 1;


            // Send command packet

            // Command packet bytes:
            // +00: ________ b: Command code
            // +01: snnnnnnn b: Activate SPU y/n
            //                  Bits minus one (0..127, corresponding with 1..128)
            // +02: <bits>

            ow->hw_buf_free -= (3 + byte_cnt);
            ow_xfr->active_packets++;

            u = (U8)bit_cnt - 1;
            u |= (ow_xfr->user.touch_bits.spu << 7);

            if (!Tx_Byte(ctx,AXICAT_CMD_OW_TOUCH_BITS)) goto Err;
            if (!Tx_Byte(ctx,u)) goto Err;

            for (;;)
            {
                data_byte = (!ow_xfr->detached)
                                ? ow_xfr->buf[ow_xfr->touch_bits.buf_fi >> 3]
                                : 0xFF;

                data_bits = ow_xfr->touch_bits.buf_sc - ow_xfr->touch_bits.buf_fi;
                if (data_bits > 8) data_bits = 8;

                ow_xfr->touch_bits.buf_fi += data_bits;

                if (!Tx_Byte(ctx,data_byte)) goto Err;

                byte_cnt--;
                if (byte_cnt == 0) break;
            }


            // Mark as active (if not already set)
            ow_xfr->active = 1;

            if (ow_xfr->touch_bits.buf_fi == ow_xfr->touch_bits.buf_sc)
            {
                // Next transfer (if any)
                ow->cur_tx_xfr = OW_Xfr_Get_Next(ow->cur_tx_xfr);
            }
        }
        else
        if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_ENUM)
        {
            U8      flags;
            U8      u;

            // An ENUM command occupies 11 bytes in the AxiCat's buffer
            if (ow->hw_buf_free < 11) break;

            // Send command packet

            // Command packet bytes:
            // +00: ________ b: Command code

            ow->hw_buf_free -= 11;

            // Compose the flags byte
            flags = 0x00;
            if (ow_xfr->user.enumerate.next) flags |= 0x01;
            if (ow_xfr->user.enumerate.alarm) flags |= 0x02;
            if (ow_xfr->user.enumerate.enum_family) flags |= 0x04;
            if (ow_xfr->user.enumerate.smart_on_enabled)
            {
                flags |= 0x08;
                if (ow_xfr->user.enumerate.smart_on_aux) flags |= 0x10;
            }

            if (!Tx_Byte(ctx,AXICAT_CMD_OW_ENUM)) goto Err;
            if (!Tx_Byte(ctx,flags)) goto Err;

            if (ow_xfr->user.enumerate.enum_family)
                if (!Tx_Byte(ctx,ow_xfr->user.enumerate.family_code)) goto Err;

            if (ow_xfr->user.enumerate.smart_on_enabled)
                for (u = 0; u < 8; u++)
                    if (!Tx_Byte(ctx,ow_xfr->user.enumerate.smart_on_rom_code[u])) goto Err;

            // Mark as active
            ow_xfr->active = 1;

            // Next transfer (if any)
            ow->cur_tx_xfr = OW_Xfr_Get_Next(ow->cur_tx_xfr);
        }
        else
        if (ow_xfr->id == AXICAT_AL_OW_XFR_ID_PROBE)
        {
            U8      u;

            // A PROBE command occupies 9 bytes in the AxiCat's buffer
            if (ow->hw_buf_free < 9) break;

            // Send command packet

            // Command packet bytes:
            // +00: ________ b: Command code
            // +01: <ROM code>

            ow->hw_buf_free -= 9;

            if (!Tx_Byte(ctx,AXICAT_CMD_OW_PROBE)) goto Err;
            for (u = 0; u < 8; u++)
                if (!Tx_Byte(ctx,ow_xfr->user.probe.rom_code[u])) goto Err;

            // Mark as active
            ow_xfr->active = 1;

            // Next transfer (if any)
            ow->cur_tx_xfr = OW_Xfr_Get_Next(ow->cur_tx_xfr);
        }
        else
            // Shouldn't end up here
            goto Err;
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Rx_Reset (CTX *ctx)

Try to transmit command packets to the 1-Wire part in the AxiCat.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Rx_Reset (CTX *ctx)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;
    LDLL_ITEM  *item;
    U8          status;

    // Init
    ow = &ctx->ow;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&ow->sch_xfr_list_base);
    if (!item) goto Err;
    ow_xfr = GetContAd(item,OW_XFR,list_item);

    // Consistency check
    if (ow_xfr->id != AXICAT_AL_OW_XFR_ID_RESET) goto Err;

    ow_xfr->user.reset.pd = ctx->rx.ow_reset.pd;

    status = ctx->rx.ow_reset.skipped
                ? AXICAT_AL_OW_ST_SKIPPED
                : AXICAT_AL_OW_ST_SUCCESS;

    OW_Xfr_Complete(ow_xfr,status);
    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Rx_Chk_Touch_Bits (CTX *ctx)
-----------------------------------------------------------------------------*/


static
FLAG  OW_Rx_Chk_Touch_Bits (CTX *ctx)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;
    LDLL_ITEM  *item;

    // Init
    ow = &ctx->ow;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&ow->sch_xfr_list_base);
    if (!item) goto Err;
    ow_xfr = GetContAd(item,OW_XFR,list_item);

    // Consistency check
    if (ow_xfr->id != AXICAT_AL_OW_XFR_ID_TOUCH_BITS) goto Err;
    if (ow_xfr->user.touch_bits.spu != ctx->rx.ow_touch_bits.spu) goto Err;

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Rx_Byte_Touch_Bits (CTX *ctx, U8 data_byte)

Process a data byte from an incoming touch bits response.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Rx_Byte_Touch_Bits (CTX *ctx, U8 data_byte)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;
    LDLL_ITEM  *item;
    U16         data_bits;


    // Init
    ow = &ctx->ow;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&ow->sch_xfr_list_base);
    if (!item) goto Err;
    ow_xfr = GetContAd(item,OW_XFR,list_item);

    // Store the data byte
    if (!ow_xfr->detached) ow_xfr->buf[ow_xfr->touch_bits.buf_si >> 3] = data_byte;


    // Increment the store index (number of bits stored)

    data_bits = ow_xfr->touch_bits.buf_sc - ow_xfr->touch_bits.buf_si;
    if (data_bits > 8) data_bits = 8;

    ow_xfr->touch_bits.buf_si += data_bits;


    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Rx_EOP_Touch_Bits (CTX *ctx)

Process the end of an incoming touch bits response packet.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Rx_EOP_Touch_Bits (CTX *ctx)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;
    LDLL_ITEM  *item;
    FLAG        last_packet;


    // Init
    ow = &ctx->ow;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&ow->sch_xfr_list_base);
    if (!item) goto Err;
    ow_xfr = GetContAd(item,OW_XFR,list_item);


    ow_xfr->active_packets--;

    last_packet = 0;

    if (ctx->rx.ow_touch_bits.short_packet)
    {
        // In case of short-packet, no more packet will be scheduled wo we wait
        // for the remaining packets (if any) to be skipped (drained).

        if (ow_xfr->active_packets == 0) last_packet = 1;
    }
    else
    {
        // Active packets may be zero, but this doesn't mean the transfer has
        // completed as more packets may have to be sent to handle the
        // remaining data bits.
        //
        // Check progression of the transferred bits to determine whether the
        // transfer has completed.

        if (ow_xfr->touch_bits.buf_si == ow_xfr->touch_bits.buf_sc) last_packet = 1;
    }

    if (last_packet)
    {
        ow_xfr->touch_bits.xfrd = ow_xfr->touch_bits.buf_si;

        // Complete successfully
        OW_Xfr_Complete(ow_xfr,AXICAT_AL_OW_ST_SUCCESS);
    }
    else
    {
        // One or more data bytes will follow; check for buffer overflow
        if (ow_xfr->touch_bits.buf_si == ow_xfr->touch_bits.buf_sc) goto Err;
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Rx_Skip_Touch_Bits (CTX *ctx)

Process an incoming touch bits response that's marked as skipped.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Rx_Skip_Touch_Bits (CTX *ctx)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;
    LDLL_ITEM  *item;

    // Init
    ow = &ctx->ow;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&ow->sch_xfr_list_base);
    if (!item) goto Err;
    ow_xfr = GetContAd(item,OW_XFR,list_item);

    ow_xfr->active_packets--;

    // Mark as skipped (if not already done)
    if (!ow_xfr->skipped)
    {
        ow_xfr->skipped         = 1;
        ow_xfr->touch_bits.xfrd = ow_xfr->touch_bits.buf_si;
    }

    // A 1-Wire command will be skipped only when command OW DISABLE was sent
    // to the AxiCat. Note that this is different from TWI.
    //
    // A touch bits transfer may spawn multiple 1-Wire commands. Since the
    // current touch bits command is skipped, the current transfer mustn't
    // spawn any further touch bits commands and must be completed instead.
    // Order:
    // [1] Flag ow_xfr->skipped is set, hence OW_Tx() doesn't issue another
    //     touch bits command for this transfer.
    // [2] The following code check for the arrival of the last touch bits
    //     response.

    if (ow_xfr->active_packets == 0)
    {
        // Complete as skipped
        OW_Xfr_Complete(ow_xfr,AXICAT_AL_OW_ST_SKIPPED);
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Rx_Enum (CTX *ctx)

Process an enumeration response.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Rx_Enum (CTX *ctx)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;
    LDLL_ITEM  *item;
    U8          status;

    // Init
    ow = &ctx->ow;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&ow->sch_xfr_list_base);
    if (!item) goto Err;
    ow_xfr = GetContAd(item,OW_XFR,list_item);

    // Consistency check
    if (ow_xfr->id != AXICAT_AL_OW_XFR_ID_ENUM) goto Err;

    status = ctx->rx.ow_enum.skipped
                ? AXICAT_AL_OW_ST_SKIPPED
                : AXICAT_AL_OW_ST_SUCCESS;

    ow_xfr->user.enumerate.found = ctx->rx.ow_enum.found;

    if (ctx->rx.ow_enum.found) memcpy(ow_xfr->user.enumerate.rom_code,ctx->rx.ow_enum.rom_code,8);

    // Complete
    OW_Xfr_Complete(ow_xfr,status);
    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  OW_Rx_Probe (CTX *ctx)

Process a probe response.
-----------------------------------------------------------------------------*/


static
FLAG  OW_Rx_Probe (CTX *ctx)
{
    OW_XFR     *ow_xfr;
    OW_CTX     *ow;
    LDLL_ITEM  *item;
    U8          status;

    // Init
    ow = &ctx->ow;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&ow->sch_xfr_list_base);
    if (!item) goto Err;
    ow_xfr = GetContAd(item,OW_XFR,list_item);

    // Consistency check
    if (ow_xfr->id != AXICAT_AL_OW_XFR_ID_PROBE) goto Err;

    status = ctx->rx.ow_probe.skipped
                ? AXICAT_AL_OW_ST_SKIPPED
                : AXICAT_AL_OW_ST_SUCCESS;

    ow_xfr->user.probe.found = ctx->rx.ow_probe.found;

    // Complete
    OW_Xfr_Complete(ow_xfr,status);
    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}
