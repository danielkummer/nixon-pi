
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// i2cdev_i2cbus.c
//
// I2C bus interface for a i2c-dev file.
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


#include "i2cdev_i2cbus.h"
#include "osrtl.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>


// Forward declarations

typedef struct  _CTX            CTX;
typedef struct  _BRIDGE_XFR     BRIDGE_XFR;


struct  _CTX
{
    I2CBUS_INTF         intf;

    int                 i2cdev_fd;
};


struct  _BRIDGE_XFR
{
    I2CBUS_XFR          user_xfr;       // Public area for user
    U8                  status;         // Most recently produced status (I2CBUS_XFR_STATUS_Xxx)
};


static
VOID  i2cdev_xfr_destroy_cb (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    BRIDGE_XFR     *bridge_xfr;

    // Get the containing context
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    // Clean up
    free(bridge_xfr);
}


static
I2CBUS_XFR  *i2cdev_xfr_create_cb (I2CBUS_INTF *intf)
{
    BRIDGE_XFR     *bridge_xfr;

    // Allocate 
    bridge_xfr = calloc(sizeof(BRIDGE_XFR),1);
    if (!bridge_xfr) return 0;

    return &bridge_xfr->user_xfr;
}


static
FLAG  i2cdev_xfr_schedule_cb (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    CTX            *ctx;
    BRIDGE_XFR     *bridge_xfr;
    int             i;
    FLAG            res;
    ssize_t         xfrd;

    // Get the containing contexts
    ctx        = GetContAd(intf,CTX,intf);
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    // Initialize resulting values
    bridge_xfr->user_xfr.xfrd     = 0;
    bridge_xfr->user_xfr.nack_rsp = 0;

    // Set the slave address
    i = ioctl(ctx->i2cdev_fd,I2C_SLAVE,bridge_xfr->user_xfr.slave_ad);
    if (i < 0) goto Fail;

    // Execute the transfer
    if (bridge_xfr->user_xfr.dir == I2CBUS_DIR_WRITE)
    {
        xfrd = write(ctx->i2cdev_fd,bridge_xfr->user_xfr.buf,bridge_xfr->user_xfr.sc);
    }
    else
    {
        xfrd = read(ctx->i2cdev_fd,bridge_xfr->user_xfr.buf,bridge_xfr->user_xfr.sc);
    }

    if (xfrd < 0) xfrd = 0;

    // Gather results on success
    bridge_xfr->user_xfr.xfrd     = (U16)xfrd;
    bridge_xfr->user_xfr.nack_rsp = (bridge_xfr->user_xfr.xfrd < bridge_xfr->user_xfr.sc) ? 1 : 0;

  Success:

    res = 1;
    goto Done;

  Fail:

    res = 0;
    goto Done;

  Done:

    // Produce status
    bridge_xfr->status = res ? I2CBUS_XFR_STATUS_SUCCESS
                             : I2CBUS_XFR_STATUS_ERROR;

/*
    // DBG.
    printf("i2cdev_xfr_schedule_cb: %s ad:%d xfrd:%d sc:%d nack:%d st:%d ok:%d\n",
           (bridge_xfr->user_xfr.dir == I2CBUS_DIR_WRITE) ? "write" : "read",
           bridge_xfr->user_xfr.slave_ad,
           bridge_xfr->user_xfr.xfrd,
           bridge_xfr->user_xfr.sc,
           bridge_xfr->user_xfr.nack_rsp,
           bridge_xfr->status,
           res);
*/

    return res;
}


static
VOID  i2cdev_xfr_cancel_cb (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    // Nothing to do
}


static
I2CBUS_XFR_STATUS  i2cdev_xfr_get_status_cb
(
    I2CBUS_INTF    *intf,
    I2CBUS_XFR     *user_xfr
)
{
    BRIDGE_XFR     *bridge_xfr;

    // Get the containing context
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    return bridge_xfr->status;
}


static
VOID  i2cdev_set_speed_cb (I2CBUS_INTF *intf, U32 speed)
{
    // <>
}


static  I2CBUS_FN_TABLE     i2cdev_i2cbus_fn_table =
{
    .set_speed_fn       = i2cdev_set_speed_cb,
    .xfr_create_fn      = i2cdev_xfr_create_cb,
    .xfr_destroy_fn     = i2cdev_xfr_destroy_cb,
    .xfr_schedule_fn    = i2cdev_xfr_schedule_cb,
    .xfr_cancel_fn      = i2cdev_xfr_cancel_cb,
    .xfr_get_status_fn  = i2cdev_xfr_get_status_cb
};


VOID  I2CDev_I2CBus_Destroy (I2CBUS_INTF *intf)
{
    CTX    *ctx;

    if (!intf) return;

    // Get the containing context
    ctx = GetContAd(intf,CTX,intf);

    if (ctx->i2cdev_fd != -1) close(ctx->i2cdev_fd);

    free(ctx);
}


I2CBUS_INTF  *I2CDev_I2CBus_Create (I2CDEV_I2CBUS_CREATE_IO *io)
{
    CTX    *ctx;


    // Allocate a cleared context
    ctx = calloc(sizeof(CTX),1);
    if (!ctx)
    {
        if (io->verbose) printf("Error: out of memory\n");
        io->err = I2CDEV_I2CBUS_CREATE_ERR_OOM;
        goto Err;
    }

    // Set up the context
    ctx->intf.fn_table = &i2cdev_i2cbus_fn_table;
    //ctx->i2cdev_fd     = -1;


    ctx->i2cdev_fd = open(io->i2cdev_name,O_RDWR);
    if (ctx->i2cdev_fd == -1)
    {
        if (io->verbose) printf("Error: can't open file %s, errno %d\n",io->i2cdev_name,errno);

        if (errno == ENOENT)
            io->err = I2CDEV_I2CBUS_CREATE_ERR_NO_DEV;
        else
        if (errno == EACCES)
        {
            io->err = I2CDEV_I2CBUS_CREATE_ERR_NO_PERM;

            if (io->verbose) printf("Hint: run program as root\n");
        }
        else
            io->err = I2CDEV_I2CBUS_CREATE_ERR_OTHER;

        goto Err;
    }


    return &ctx->intf;


  Err:

    I2CDev_I2CBus_Destroy(&ctx->intf);
    return 0;
}

