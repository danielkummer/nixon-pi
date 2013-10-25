
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// bsc_i2cbus.c
//
// I2C bus interface for BCM2835 BSC controller.
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
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "bsc_i2cbus.h"
#include "bsc.h"


// Forward declarations

typedef struct  _CTX    CTX;


struct  _CTX
{
    // This field must come first
    I2CBUS_INTF         intf;
    int                 mem_fd;
    int                 lock_fd;
    BSC_MEMIO_CTX       memio_gpio;
    BSC_MEMIO_CTX       memio_bsc;
};


// GPIO registers

#define GPIO_REG_GPFSEL0    (0x00 >> 2)         // GPIO Function Select 0


// initialisation data for CTX.memio_bsc

static  BSC_MEMIO_CTX   memio_gpio  = { 0x20200000, 4096, MAP_FAILED };
static  BSC_MEMIO_CTX   memio_bsc0  = { 0x20205000, 4096, MAP_FAILED };
static  BSC_MEMIO_CTX   memio_bsc1  = { 0x20804000, 4096, MAP_FAILED };


static
FLAG  bsc_write_cb (I2CBUS_INTF *intf, I2CBUS_RW_IO *io)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)intf;

    return bsc_write(&ctx->memio_bsc,io);
}


static
FLAG  bsc_read_cb (I2CBUS_INTF *intf, I2CBUS_RW_IO *io)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)intf;

    return bsc_read(&ctx->memio_bsc,io);
}


static
FLAG  bsc_write_reg_cb (I2CBUS_INTF *intf, U8 slave_ad, U8 reg)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)intf;

    return bsc_write_reg(&ctx->memio_bsc,slave_ad,reg);
}


static
FLAG  bsc_read_reg_cb (I2CBUS_INTF *intf, U8 slave_ad, U8 *reg)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)intf;

    return bsc_read_reg(&ctx->memio_bsc,slave_ad,reg);
}


static  I2CBUS_CB_TABLE     bsc_i2cbus_cb_table =
{
    .write_fn     = bsc_write_cb,
    .read_fn      = bsc_read_cb,
    .write_reg_fn = bsc_write_reg_cb,
    .read_reg_fn  = bsc_read_reg_cb
};


VOID  BSC_I2CBus_Destroy (I2CBUS_INTF *intf)
{
    CTX    *ctx;

    if (!intf) return;

    // Init
    ctx = (CTX*)intf;

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
    BSC_MEMIO_CTX  *src_memio_bsc;
    CHAR            lock_fname[40];
    struct  flock   fl;
    int             i;
    U32             u;
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

    ctx->intf.cb_table = &bsc_i2cbus_cb_table;
    ctx->mem_fd        = -1;
    ctx->lock_fd       = -1;
    ctx->memio_bsc.va  = MAP_FAILED;

    memcpy(&ctx->memio_gpio,&memio_gpio,sizeof(BSC_MEMIO_CTX));


    // Check the given index of the BSC controller
    if (io->bsc_index == 0) src_memio_bsc = &memio_bsc0; else
    if (io->bsc_index == 1) src_memio_bsc = &memio_bsc1; else
    {
        io->err = BSC_I2CBUS_CREATE_ERR_PARAM;
        goto Err;
    }


    memcpy(&ctx->memio_bsc,src_memio_bsc,sizeof(BSC_MEMIO_CTX));


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

