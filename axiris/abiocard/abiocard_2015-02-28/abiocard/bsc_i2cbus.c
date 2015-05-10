
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// bsc_i2cbus.c
//
// I2C bus interface for BCM2835/2836 BSC controller.
//
// Language: GCC4 gnu89
//
// History:
//
//   2014-07-05  Peter S'heeren, Axiris
//
//      * Created.
//
//   2015-02-27  Peter S'heeren, Axiris
//
//      * Added support for RPi 2.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2014-2015  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#include "bsc_i2cbus.h"

#include "rpidetect.h"
#include "bsc.h"
#include "osrtl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


// Forward declarations

typedef struct  _CTX            CTX;
typedef struct  _BRIDGE_XFR     BRIDGE_XFR;


struct  _CTX
{
    I2CBUS_INTF         intf;

    int                 mem_fd;
    int                 lock_fd;
    BSC_MEMIO_CTX       memio_gpio;
    BSC_MEMIO_CTX       memio_bsc;
};


struct  _BRIDGE_XFR
{
    I2CBUS_XFR          user_xfr;       // Public area for user
    U8                  status;         // Most recently produced status (I2CBUS_XFR_STATUS_Xxx)
};


// GPIO registers

#define GPIO_REG_GPFSEL0    (0x00 >> 2)         // GPIO Function Select 0


// Read-only initialization data for CTX.memio_gpio and CTX.memio_bsc

static  BSC_MEMIO_CTX   memio_gpio  = { 0x00200000, 4096, MAP_FAILED };
static  BSC_MEMIO_CTX   memio_bsc0  = { 0x00205000, 4096, MAP_FAILED };
static  BSC_MEMIO_CTX   memio_bsc1  = { 0x00804000, 4096, MAP_FAILED };


static
VOID  bsc_set_speed_cb (I2CBUS_INTF *intf, U32 speed)
{
    // <>
}


static
VOID  bsc_xfr_destroy_cb (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    BRIDGE_XFR     *bridge_xfr;

    // Get the containing context
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    // Clean up
    free(bridge_xfr);
}


static
I2CBUS_XFR  *bsc_xfr_create_cb (I2CBUS_INTF *intf)
{
    BRIDGE_XFR     *bridge_xfr;

    // Allocate 
    bridge_xfr = calloc(sizeof(BRIDGE_XFR),1);
    if (!bridge_xfr) return 0;

    return &bridge_xfr->user_xfr;
}


static
FLAG  bsc_xfr_schedule_cb (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    CTX                *ctx;
    BRIDGE_XFR         *bridge_xfr;
    BSC_I2CBUS_RW_IO    io;
    FLAG                ok;

    // Get the containing contexts
    ctx        = GetContAd(intf,CTX,intf);
    bridge_xfr = GetContAd(user_xfr,BRIDGE_XFR,user_xfr);

    // Set up the transfer I/O
    memset(&io,0,sizeof(io));
    io.buf      = bridge_xfr->user_xfr.buf;
    io.len      = bridge_xfr->user_xfr.sc;
    io.slave_ad = bridge_xfr->user_xfr.slave_ad;

    // Execute the transfer
    if (bridge_xfr->user_xfr.dir == I2CBUS_DIR_WRITE)
    {
        ok = bsc_write(&ctx->memio_bsc,&io);
    }
    else
    {
        ok = bsc_read(&ctx->memio_bsc,&io);
    }

    // Gather results
    bridge_xfr->user_xfr.xfrd     = io.xfrd;
    bridge_xfr->user_xfr.nack_rsp = io.nack_rsp;

    // Produce status
    bridge_xfr->status = ok ? I2CBUS_XFR_STATUS_SUCCESS
                            : I2CBUS_XFR_STATUS_ERROR;

/*
    // DBG.
    printf("bsc_xfr_schedule_cb: %s ad:%d xfrd:%d sc:%d nack:%d st:%d ok:%d\n",
           (bridge_xfr->user_xfr.dir == I2CBUS_DIR_WRITE) ? "write" : "read",
           bridge_xfr->user_xfr.slave_ad,
           bridge_xfr->user_xfr.xfrd,
           bridge_xfr->user_xfr.sc,
           bridge_xfr->user_xfr.nack_rsp,
           bridge_xfr->status,
           ok);
*/

    return ok;
}


static
VOID  bsc_xfr_cancel_cb (I2CBUS_INTF *intf, I2CBUS_XFR *user_xfr)
{
    // Nothing to do
}


static
I2CBUS_XFR_STATUS  bsc_xfr_get_status_cb
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


static  I2CBUS_FN_TABLE     bsc_i2cbus_fn_table =
{
    .set_speed_fn       = bsc_set_speed_cb,
    .xfr_create_fn      = bsc_xfr_create_cb,
    .xfr_destroy_fn     = bsc_xfr_destroy_cb,
    .xfr_schedule_fn    = bsc_xfr_schedule_cb,
    .xfr_cancel_fn      = bsc_xfr_cancel_cb,
    .xfr_get_status_fn  = bsc_xfr_get_status_cb
};


VOID  BSC_I2CBus_Destroy (I2CBUS_INTF *intf)
{
    CTX    *ctx;

    if (!intf) return;

    // Get the containing context
    ctx = GetContAd(intf,CTX,intf);

    if (ctx->mem_fd != -1) close(ctx->mem_fd);

    if (ctx->lock_fd != -1)
    {
        struct  flock   fl;

        fl.l_type   = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start  = 0;
        fl.l_len    = 0;

        fcntl(ctx->lock_fd,F_SETLK,&fl);

        close(ctx->lock_fd);
    }

    free(ctx);
}


I2CBUS_INTF  *BSC_I2CBus_Create (BSC_I2CBUS_CREATE_IO *io)
{
    CTX            *ctx;
    RPIDETECT_IO    rpidetect_io;
    BSC_MEMIO_CTX  *src_memio_bsc;
    CHAR            lock_fname[40];
    struct  flock   fl;
    int             i;
    U32             u;
    U32             peri_base;
    FLAG            ok;


    // Allocate a cleared context
    ctx = calloc(sizeof(CTX),1);
    if (!ctx)
    {
        if (io->verbose) printf("Error: out of memory\n");
        io->err = BSC_I2CBUS_CREATE_ERR_OOM;
        goto Err;
    }


    // Set up the context
    ctx->intf.fn_table = &bsc_i2cbus_fn_table;
    ctx->mem_fd        = -1;
    ctx->lock_fd       = -1;
    ctx->memio_bsc.va  = MAP_FAILED;


    // Get the SoC chip of the RPi
    rpidetect(&rpidetect_io);
    if (rpidetect_io.chip == RPIDETECT_CHIP_BCM2835) peri_base = 0x20000000; else
    if (rpidetect_io.chip == RPIDETECT_CHIP_BCM2836) peri_base = 0x3F000000; else
    {
        io->err = BSC_I2CBUS_CREATE_ERR_NO_RPI;
        goto Err;
    }


    // Check the given index of the BSC controller
    if (io->bsc_index == 0) src_memio_bsc = &memio_bsc0; else
    if (io->bsc_index == 1) src_memio_bsc = &memio_bsc1; else
    {
        io->err = BSC_I2CBUS_CREATE_ERR_PARAM;
        goto Err;
    }


    // Put a lock file in the /var/lock section of the file system.

    sprintf(lock_fname,"/var/lock/bsc_i2cbus_%d.lock",io->bsc_index);

    ctx->lock_fd = open(lock_fname,O_WRONLY|O_CREAT,0666);
    if (ctx->lock_fd == -1)
    {
        if (io->verbose) printf("Error: can't create or open lock file %s, errno %d\n",lock_fname,errno);
        goto Err_Chk_ErrNo;
    }

    // Try to lock the file for write access. If this step succeeds then this
    // module wins exclusive access to the AbioWire. If not, another module has
    // already taken ownership over the AbioWire.

    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;

    i = fcntl(ctx->lock_fd,F_SETLK,&fl);
    if (i == -1)
    {
        if (io->verbose) printf("Error: can't lock the lock file, errno %d\n",errno);
        io->err = BSC_I2CBUS_CREATE_ERR_LOCKED;
        goto Err;
    }


    // Prepare the memory mapping structures

    memcpy(&ctx->memio_gpio,&memio_gpio,sizeof(BSC_MEMIO_CTX));
    ctx->memio_gpio.pha += peri_base;

    memcpy(&ctx->memio_bsc,src_memio_bsc,sizeof(BSC_MEMIO_CTX));
    ctx->memio_bsc.pha  += peri_base;


    ctx->mem_fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (ctx->mem_fd == -1)
    {
        if (io->verbose) printf("Error: can't open /dev/mem, errno %d\n",errno);
        goto Err_Chk_ErrNo;
    }

    ok = bsc_memio_init(&ctx->memio_gpio,ctx->mem_fd);
    if (!ok)
    {
        if (io->verbose) printf("Error: can't map GPIO, errno %d\n",errno);
        goto Err_Chk_ErrNo;
    }

    ok = bsc_memio_init(&ctx->memio_bsc,ctx->mem_fd);
    if (!ok)
    {
        if (io->verbose) printf("Error: can't map BSC, errno %d\n",errno);
        goto Err_Chk_ErrNo;
    }

    // Set up the BSC controller
    //
    // Not relevant but nice to know: FSELx=000b sets a pin to general-purpose
    // INPUT, FSELx=001b sets a pin to general-purpose OUTPUT.
    //
    if (io->bsc_index == 0)
    {
        // Set up BSC0:
        // * FSEL0=100b (ALT0): route SDA0 to pin GPIO0, pull high
        // * FSEL1=100b (ALT0): route SCL0 to pin GPIO1, pull high
        //
        u = ctx->memio_gpio.va[GPIO_REG_GPFSEL0];
        u &= ~0b111111;
        u |=  0b100100;
        ctx->memio_gpio.va[GPIO_REG_GPFSEL0] = u;
    }
    else
    {
        // Set up BSC1:
        // * FSEL2=100b (ALT0): route SDA1 to pin GPIO0, pull high
        // * FSEL3=100b (ALT0): route SCL1 to pin GPIO1, pull high
        //
        u = ctx->memio_gpio.va[GPIO_REG_GPFSEL0];
        u &= ~0b111111 << 6;
        u |=  0b100100 << 6;
        ctx->memio_gpio.va[GPIO_REG_GPFSEL0] = u;
    }

    //printf("GPIO va %p\n",memio_gpio.va);
    //printf("BSC  va %p\n",memio_bsc->va);


    return &ctx->intf;


  Err_Chk_ErrNo:

    if (errno == EACCES)
    {
        io->err = BSC_I2CBUS_CREATE_ERR_NO_PERM;

        if (io->verbose) printf("Hint: run program as root\n");
    }
    else
        io->err = BSC_I2CBUS_CREATE_ERR_OTHER;


  Err:

    BSC_I2CBus_Destroy(&ctx->intf);
    return 0;
}

