
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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include "i2cdev_i2cbus.h"


typedef struct  _CTX    CTX;


struct  _CTX
{
    // This field must come first
    I2CBUS_INTF         intf;

    int                 i2cdev_fd;
};


static
FLAG  i2cdev_write_cb (I2CBUS_INTF *intf, I2CBUS_RW_IO *io)
{
    CTX        *ctx;
    int         res;
    ssize_t     xfrd;

    // Init
    ctx = (CTX*)intf;

    io->xfrd = 0;

    res = ioctl(ctx->i2cdev_fd,I2C_SLAVE,io->slave_ad);
    if (res < 0) return 0;

    xfrd = write(ctx->i2cdev_fd,io->buf,io->len);

    if (xfrd > 0) io->xfrd = (U16)xfrd;

    return (io->xfrd < io->len) ? 0 : 1;
}


static
FLAG  i2cdev_read_cb (I2CBUS_INTF *intf, I2CBUS_RW_IO *io)
{
    CTX        *ctx;
    int         res;
    ssize_t     xfrd;

    // Init
    ctx = (CTX*)intf;

    io->xfrd = 0;

    res = ioctl(ctx->i2cdev_fd,I2C_SLAVE,io->slave_ad);
    if (res < 0) return 0;

    xfrd = read(ctx->i2cdev_fd,io->buf,io->len);

    if (xfrd > 0) io->xfrd = (U16)xfrd;

    return (io->xfrd < io->len) ? 0 : 1;
}


static
FLAG  i2cdev_write_reg_cb (I2CBUS_INTF *intf, U8 slave_ad, U8 reg)
{
    CTX        *ctx;
    int         res;
    ssize_t     xfrd;

    // Init
    ctx = (CTX*)intf;

    xfrd = 0;

    res = ioctl(ctx->i2cdev_fd,I2C_SLAVE,slave_ad);
    if (res < 0) return 0;

    xfrd = write(ctx->i2cdev_fd,&reg,1);

    return (xfrd == 1) ? 1 : 0;
}


static
FLAG  i2cdev_read_reg_cb (I2CBUS_INTF *intf, U8 slave_ad, U8 *reg)
{
    CTX        *ctx;
    int         res;
    ssize_t     xfrd;

    // Init
    ctx = (CTX*)intf;

    xfrd = 0;

    res = ioctl(ctx->i2cdev_fd,I2C_SLAVE,slave_ad);
    if (res < 0) return 0;

    xfrd = read(ctx->i2cdev_fd,reg,1);

    return (xfrd == 1) ? 1 : 0;
}


static  I2CBUS_CB_TABLE     i2cdev_i2cbus_cb_table =
{
    .write_fn     = i2cdev_write_cb,
    .read_fn      = i2cdev_read_cb,
    .write_reg_fn = i2cdev_write_reg_cb,
    .read_reg_fn  = i2cdev_read_reg_cb
};


VOID  I2CDev_I2CBus_Destroy (I2CBUS_INTF *intf)
{
    CTX    *ctx;

    if (!intf) return;

    // Init
    ctx = (CTX*)intf;

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
    ctx->intf.cb_table = &i2cdev_i2cbus_cb_table;
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

    BSC_I2CBus_Destroy(&ctx->intf);
    return 0;
}

