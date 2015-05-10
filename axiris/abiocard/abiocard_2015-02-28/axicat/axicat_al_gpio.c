
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al_gpio.c
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
FLAG
DynllCall  AXICAT_AL_GPIO_Set_Dir (AXICAT_AL_HANDLE handle, U8 pin, FLAG dir)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG
DynllCall  AXICAT_AL_GPIO_Set_Dir (AXICAT_AL_HANDLE handle, U8 pin, FLAG dir)
{
    CTX    *ctx;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (pin > 16) return 0;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) return 0;

    u = pin;
    if (dir) u |= 0x80;

    // Send command packet
    if (!Tx_Byte(ctx,AXICAT_CMD_GPIO_SET_DIR)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG
DynllCall  AXICAT_AL_GPIO_Write (AXICAT_AL_HANDLE handle, U8 pin, FLAG state)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG
DynllCall  AXICAT_AL_GPIO_Write (AXICAT_AL_HANDLE handle, U8 pin, FLAG state)
{
    CTX    *ctx;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (pin > 16) return 0;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) return 0;


    // Send command packet

    u = pin;
    if (state) u |= 0x80;

    if (!Tx_Byte(ctx,AXICAT_CMD_GPIO_WRITE)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;


    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
GPIO_XFR  *GPIO_Xfr_Get_Next (GPIO_XFR *gpio_xfr)
-----------------------------------------------------------------------------*/


static
GPIO_XFR  *GPIO_Xfr_Get_Next (GPIO_XFR *gpio_xfr)
{
    LDLL_ITEM  *item;

    item = LDLLGetNextItem(&gpio_xfr->list_item);
    if (item)
        return GetContAd(item,GPIO_XFR,list_item);
    else
        return 0;
}


/*-----------------------------------------------------------------------------
VOID  TWI_Xfr_Uncache (TWI_XFR *twi_xfr)

Remove the given transfer from the list of cached transfer objects and destroy
it.
-----------------------------------------------------------------------------*/


static
VOID  GPIO_Xfr_Uncache (GPIO_XFR *gpio_xfr)
{
    GPIO_CTX   *gpio;

    // Init
    gpio = &gpio_xfr->ctx->gpio;

    // Unlink from the list of cached transfer objects
    LDLLUnlinkItem(&gpio->cached_xfr_list_base,&gpio_xfr->list_item);
    gpio->cached_xfr_cnt--;

    // Clean up
    AXICAT_AL_Free(gpio_xfr);
}


/*-----------------------------------------------------------------------------
VOID  GPIO_Xfr_Unassign (GPIO_XFR *gpio_xfr)
-----------------------------------------------------------------------------*/


static
VOID  GPIO_Xfr_Unassign (GPIO_XFR *gpio_xfr)
{
    GPIO_CTX   *gpio;

    // Init
    gpio = &gpio_xfr->ctx->gpio;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&gpio->assigned_xfr_list_base,&gpio_xfr->list_item);
    gpio->assigned_xfr_cnt--;

    // TODO:
    // * Decide whether to cache or destroy the transfer
    if (1)
    {
        // Link to the list of cached transfer objects
        LDLLLinkTailItem(&gpio->cached_xfr_list_base,&gpio_xfr->list_item);
        gpio->cached_xfr_cnt++;
        gpio_xfr->loc = GPIO_XFR_LOC_CACHED;
    }
}


/*-----------------------------------------------------------------------------
FLAG  GPIO_Xfr_Complete (GPIO_XFR *gpio_xfr, U8 status)

Complete a scheduled GPIO transfer with the given status.

The transfer is moved to the list of assigned transfers. If the detached flag
is set, the transfer is unassigned as well.
-----------------------------------------------------------------------------*/


static
VOID  GPIO_Xfr_Complete (GPIO_XFR *gpio_xfr, U8 status)
{
    GPIO_CTX   *gpio;

    // Init
    gpio = &gpio_xfr->ctx->gpio;

    if (gpio->cur_tx_xfr == gpio_xfr)
    {
        gpio->cur_tx_xfr = GPIO_Xfr_Get_Next(gpio->cur_tx_xfr);
    }

    // Unlink from the list of scheduled transfer objects
    LDLLUnlinkItem(&gpio->sch_xfr_list_base,&gpio_xfr->list_item);
    gpio->sch_xfr_cnt--;

    // Link to the list of assigned transfer objects
    LDLLLinkTailItem(&gpio->assigned_xfr_list_base,&gpio_xfr->list_item);
    gpio->assigned_xfr_cnt++;
    gpio_xfr->loc = GPIO_XFR_LOC_ASSIGNED;

    // Report completion status
    gpio_xfr->user.status = status;

    // Unassign the transfer object if the user has "destroyed" it earlier
    if (gpio_xfr->detached)
    {
        GPIO_Xfr_Unassign(gpio_xfr);
    }
}


/*-----------------------------------------------------------------------------
VOID  GPIO_Complete_All_Xfrs (GPIO_CTX *gpio)

Complete all scheduled transfer forcibly, nomatter they're active or not.

This function is called when the AxiCat is deemed detached.
-----------------------------------------------------------------------------*/


static
VOID  GPIO_Complete_All_Xfrs (GPIO_CTX *gpio)
{
    GPIO_XFR   *gpio_xfr;
    LDLL_ITEM  *item;

    // Iterate all scheduled transfers until the list has depleted
    while ((item = LDLLGetHeadItem(&gpio->sch_xfr_list_base)) != 0)
    {
        gpio_xfr = GetContAd(item,GPIO_XFR,list_item);

        // Complete as cancelled
        GPIO_Xfr_Complete(gpio_xfr,AXICAT_AL_GPIO_ST_CANCELLED);
    }
}


/*-----------------------------------------------------------------------------
FLAG  GPIO_Xfr_Cancel (GPIO_XFR *gpio_xfr)

Try to cancel the given transfer. If the transfer can be cancelled, it's
completed with status CANCELLED.

The transfer can only be cancelled when it's scheduled and non-active.
-----------------------------------------------------------------------------*/


static
FLAG  GPIO_Xfr_Cancel (GPIO_XFR *gpio_xfr)
{
    GPIO_CTX   *gpio;

    // Get the GPIO context
    gpio = &gpio_xfr->ctx->gpio;

    if (gpio_xfr->loc != GPIO_XFR_LOC_SCHEDULED)
    {
        // Can't cancel a transfer that's not scheduled
        return 0;
    }

    if (gpio_xfr->active)
    {
        // Can't cancel. The command has been transmitted, and we're waiting
        // for the response.
    }
    else
    {
        // Complete as cancelled
        GPIO_Xfr_Complete(gpio_xfr,AXICAT_AL_GPIO_ST_CANCELLED);
    }

    return 1;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_GPIO_Xfr_Cancel (AXICAT_AL_GPIO_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_GPIO_Xfr_Cancel (AXICAT_AL_GPIO_XFR *xfr)
{
    GPIO_XFR   *gpio_xfr;

    // Get the containing transfer context
    gpio_xfr = GetContAd(xfr,GPIO_XFR,user);

    GPIO_Xfr_Cancel(gpio_xfr);
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_GPIO_Xfr_Destroy (AXICAT_AL_GPIO_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_GPIO_Xfr_Destroy (AXICAT_AL_GPIO_XFR *xfr)
{
    GPIO_XFR   *gpio_xfr;
    GPIO_CTX   *gpio;

    if (!xfr) return;

    // Init
    gpio_xfr = GetContAd(xfr,GPIO_XFR,user);
    gpio     = &gpio_xfr->ctx->gpio;

    // Cancel the transfer
    GPIO_Xfr_Cancel(gpio_xfr);

    if (gpio_xfr->loc == GPIO_XFR_LOC_ASSIGNED)
    {
        // The transfer was cancelled. Unassign the object (cache or destroy).
        GPIO_Xfr_Unassign(gpio_xfr);
    }
    else
    {
        // The transfer wasn't cancelled. Mark as detached and unassign later,
        // upon completion.
        gpio_xfr->detached = 1;
    }
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
AXICAT_AL_GPIO_XFR *
DynllCall  AXICAT_AL_GPIO_Xfr_Create (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
AXICAT_AL_GPIO_XFR *
DynllCall  AXICAT_AL_GPIO_Xfr_Create (AXICAT_AL_HANDLE handle)
{
    CTX        *ctx;
    GPIO_CTX   *gpio;
    GPIO_XFR   *gpio_xfr;
    LDLL_ITEM  *item;

    // Init
    ctx  = (CTX*)handle;
    gpio = &ctx->gpio;

    if (gpio->cached_xfr_cnt > 0)
    {
        // Take a GPIO transfer object from the pool of cached transfers

        // Unlink from the list of cached transfer objects
        item = LDLLUnlinkHeadItem(&gpio->cached_xfr_list_base);
        gpio->cached_xfr_cnt--;

        gpio_xfr = GetContAd(item,GPIO_XFR,list_item);

        //gpio_xfr->detached = 0;
    }
    else
    {
        // Allocate a GPIO transfer object
        gpio_xfr = AXICAT_AL_Alloc_Clear(sizeof(GPIO_XFR));
        if (!gpio_xfr) return 0;

        // Set up
        gpio_xfr->ctx = ctx;
    }

    // Move the transfer object to the list of assigned transfer objects
    LDLLLinkTailItem(&gpio->assigned_xfr_list_base,&gpio_xfr->list_item);
    gpio->assigned_xfr_cnt++;
    gpio_xfr->loc = GPIO_XFR_LOC_ASSIGNED;

    return &gpio_xfr->user;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_GPIO_Xfr_Schedule (AXICAT_AL_GPIO_XFR *xfr)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_GPIO_Xfr_Schedule (AXICAT_AL_GPIO_XFR *xfr)
{
    GPIO_XFR   *gpio_xfr;
    GPIO_CTX   *gpio;

    // Check arguments
    if (xfr->pin > 16) return 0;

    // Get the containing transfer context
    gpio_xfr = GetContAd(xfr,GPIO_XFR,user);

    if (gpio_xfr->loc != GPIO_XFR_LOC_ASSIGNED) return 0;

    // Get the GPIO context
    gpio = &gpio_xfr->ctx->gpio;

    // Unlink from the list of assigned transfer objects
    LDLLUnlinkItem(&gpio->assigned_xfr_list_base,&gpio_xfr->list_item);
    gpio->assigned_xfr_cnt--;

    // Link to the list of scheduled transfer objects
    LDLLLinkTailItem(&gpio->sch_xfr_list_base,&gpio_xfr->list_item);
    gpio->sch_xfr_cnt++;
    gpio_xfr->loc = GPIO_XFR_LOC_SCHEDULED;

    // Set as the current Tx transfer if none is present
    if (!gpio->cur_tx_xfr) gpio->cur_tx_xfr = gpio_xfr;

    // Set up for the scheduled state
    gpio_xfr->active   = 0;
    gpio_xfr->detached = 0;
    gpio_xfr->pin      = xfr->pin;

    // User data
    gpio_xfr->user.status = AXICAT_AL_GPIO_ST_SCHEDULED;

    return 1;
}


/*-----------------------------------------------------------------------------
FLAG  GPIO_Tx (CTX *ctx)
-----------------------------------------------------------------------------*/


static
FLAG  GPIO_Tx (CTX *ctx)
{
    GPIO_XFR   *gpio_xfr;
    GPIO_CTX   *gpio;

    // Init
    gpio = &ctx->gpio;

    while (gpio->cur_tx_xfr)
    {
        gpio_xfr = gpio->cur_tx_xfr;

        // Send command packet
        if (!Tx_Byte(ctx,AXICAT_CMD_GPIO_READ)) goto Err;
        if (!Tx_Byte(ctx,gpio_xfr->pin)) goto Err;

        // Mark as active
        gpio_xfr->active = 1;

        // Next transfer (if any)
        gpio->cur_tx_xfr = GPIO_Xfr_Get_Next(gpio->cur_tx_xfr);
    }

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  GPIO_Rx (CTX *ctx, U8 rsp_data)

Process AXICAT_RSP_GPIO_READ response packet.
-----------------------------------------------------------------------------*/


static
FLAG  GPIO_Rx (CTX *ctx, U8 rsp_data)
{
    GPIO_XFR   *gpio_xfr;
    GPIO_CTX   *gpio;
    LDLL_ITEM  *item;

    // Init
    gpio = &ctx->gpio;

    // Get the first scheduled transfer object
    item = LDLLGetHeadItem(&gpio->sch_xfr_list_base);
    if (!item) goto Err;
    gpio_xfr = GetContAd(item,GPIO_XFR,list_item);

    // Response packet bytes:
    // +00: ________ b: Response code
    // +01: sodnnnnn b:
    //      Bit    7: Sensed input state
    //      Bit    6: Output state
    //      Bit    5: Direction is input (0) or output (1)
    //      Bit 4..0: Pin number (0..16)

    // Check the pin number
    if (gpio_xfr->pin != (rsp_data & 0x1F)) goto Err;

    // Report results to the user
    gpio_xfr->user.dir          = (rsp_data & 0x20) ? 1 : 0;
    gpio_xfr->user.output_state = (rsp_data & 0x40) ? 1 : 0;
    gpio_xfr->user.input_state  = (rsp_data & 0x80) ? 1 : 0;

    // Complete successfully
    GPIO_Xfr_Complete(gpio_xfr,AXICAT_AL_GPIO_ST_SUCCESS);
    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}
