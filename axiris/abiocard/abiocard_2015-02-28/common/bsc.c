
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// bsc.c
//
// BCM2835 BSC driver.
//
// Language: GCC4 gnu89
//
// History:
//
//   2012-10-24  Peter S'heeren, Axiris
//
//      * Created.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2012-2013  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#include "bsc.h"

#include <stdio.h>
#include <unistd.h>


// GPIO registers

#define GPIO_REG_GPFSEL0    (0x00 >> 2)         // GPIO Function Select 0


// Length of the FIFO of a BSC

#define BSC_FIFO_LEN        16


// BSC registers

#define BSC_REG_C           (0x00 >> 2)         // Control
#define BSC_REG_S           (0x04 >> 2)         // Status
#define BSC_REG_DLEN        (0x08 >> 2)         // Data Length
#define BSC_REG_A           (0x0C >> 2)         // Slave Address
#define BSC_REG_FIFO        (0x10 >> 2)         // Data FIFO
#define BSC_REG_DIV         (0x14 >> 2)         // Clock Divider
#define BSC_REG_DEL         (0x18 >> 2)         // Data Delay


// BSC control register

#define BSC_REG_C_I2CEN     (1 << 15)           // Enabled/disable BSC
#define BSC_REG_C_INTR      (1 << 10)
#define BSC_REG_C_INTT      (1 << 9)
#define BSC_REG_C_INTD      (1 << 8)
#define BSC_REG_C_ST        (1 << 7)
#define BSC_REG_C_CLEAR     (1 << 4)            // Clear the FIFO
#define BSC_REG_C_READ      (1 << 0)            // READ (1) or WRITE (0) command


// Initiate a read command

#define BSC_REG_C_CMD_READ  (BSC_REG_C_I2CEN | BSC_REG_C_ST | BSC_REG_C_CLEAR | BSC_REG_C_READ)


// Initiate a write command

#define BSC_REG_C_CMD_WRITE (BSC_REG_C_I2CEN | BSC_REG_C_ST | BSC_REG_C_CLEAR)


// BSC status register

#define BSC_REG_S_CLKT      (1 << 9)
#define BSC_REG_S_ERR       (1 << 8)
#define BSC_REG_S_RXF       (1 << 7)
#define BSC_REG_S_TXE       (1 << 6)
#define BSC_REG_S_RXD       (1 << 5)
#define BSC_REG_S_TXD       (1 << 4)
#define BSC_REG_S_RXR       (1 << 3)
#define BSC_REG_S_TXW       (1 << 2)
#define BSC_REG_S_DONE      (1 << 1)
#define BSC_REG_S_TA        (1 << 0)


// Clear status flags before initiating a new command

#define BSC_REG_S_CLEAR     (BSC_REG_S_CLKT | BSC_REG_S_ERR | BSC_REG_S_DONE)


VOID  bsc_memio_deinit (BSC_MEMIO_CTX *ctx)
{
    if (!ctx) return;

    if (ctx->va != MAP_FAILED)
    {
        munmap((void*)(ctx->va),ctx->len);
        ctx->va = MAP_FAILED;
    }
}


FLAG  bsc_memio_init (BSC_MEMIO_CTX *ctx, int fd)
{
    ctx->va = (U32*)mmap(0,ctx->len,PROT_READ|PROT_WRITE,MAP_SHARED,fd,ctx->pha);
    if (ctx->va == MAP_FAILED) goto Err;

    return 1;
    
  Err:
  
    bsc_memio_deinit(ctx);
    return 0;
}


VOID  bsc_dump_status (BSC_MEMIO_CTX *p)
{
    U32     u;

    u = p->va[BSC_REG_S];
    printf("Status.......: %08Xh  CLKT:%d  ERR:%d  RXF:%d   TXE:%d  RXD:%d\n"
           "                           TXD:%d  RXR:%d  TXW:%d  DONE:%d   TA:%d\n",
           u,
           (u & BSC_REG_S_CLKT) ? 1 : 0,
           (u & BSC_REG_S_ERR) ? 1 : 0,
           (u & BSC_REG_S_RXF) ? 1 : 0,
           (u & BSC_REG_S_TXE) ? 1 : 0,
           (u & BSC_REG_S_RXD) ? 1 : 0,
           (u & BSC_REG_S_TXD) ? 1 : 0,
           (u & BSC_REG_S_RXR) ? 1 : 0,
           (u & BSC_REG_S_TXW) ? 1 : 0,
           (u & BSC_REG_S_DONE) ? 1 : 0,
           (u & BSC_REG_S_TA) ? 1 : 0);
}


VOID  bsc_dump_regs (BSC_MEMIO_CTX *p)
{
    printf("Control......: %08Xh\n",p->va[BSC_REG_C]);
    bsc_dump_status(p);
    printf("Data Length..: %08Xh\n",p->va[BSC_REG_DLEN]);
    printf("Slave Address: %08Xh\n",p->va[BSC_REG_A]);
    printf("Clock Divider: %08Xh\n",p->va[BSC_REG_DIV]);
    printf("Data Delay...: %08Xh\n",p->va[BSC_REG_DEL]);
    printf("\n");
}


FLAG  bsc_write (BSC_MEMIO_CTX *p, BSC_I2CBUS_RW_IO *io)
{
    U32     dlen;
    U32     st_mask;

    //printf("bsc_write: len:%d\n",io->len);

    io->xfrd     = 0;
    io->nack_rsp = 0;

    if (io->len == 0) return 0;

    // Set up the WRITE transfer
    p->va[BSC_REG_A]    = io->slave_ad;     // Slave I2C address
    p->va[BSC_REG_DLEN] = io->len;          // Data bytes to transfer
    p->va[BSC_REG_S]    = BSC_REG_S_CLEAR;  // Clear status flags

    // Clear the FIFO, start the WRITE transfer
    p->va[BSC_REG_C] = BSC_REG_C_CMD_WRITE;

    // Set the initial status mask to monitor the DONE, TXD and ERR bits.
    //
    // The BSC will set ERR when the I2C slave hasn't acknowledged the address.
    // The BSC will set TXD before the I2C slave address has been processed i.e.
    // before ERR is updated. So, if the I2C slave doesn't acknowledge the
    // address TXD is set before ERR is said meaning the routine will have
    // stored one or more bytes in the FIFO before seeing the ERR as set.
    //
    st_mask = BSC_REG_S_DONE | BSC_REG_S_TXD | BSC_REG_S_ERR;

    for (;;)
    {
        U32     st;
        U32     cnt;

        cnt = 1000;
        for (;;)
        {
            st = p->va[BSC_REG_S];
            if (st & st_mask) break;

            cnt--;
            if (cnt == 0)
            {
                //printf("time-out of I2C write transfer\n");

                return 0;
            }

            usleep(100);
        }

        //printf("cnt:%d\n",cnt);
        //bsc_dump_status(p);

        while (st & BSC_REG_S_TXD)
        {
            // Only store data bytes when still available
            if (io->xfrd == io->len)
            {
                // Narrow done the status bits for monitoring
                st_mask = BSC_REG_S_DONE | BSC_REG_S_ERR;
                break;
            }
            else
            {
                U8      data_byte;

                // Store the next byte in the BSC's FIFO
                data_byte           = io->buf[io->xfrd];
                p->va[BSC_REG_FIFO] = data_byte;
                io->xfrd++;

                //printf("%02Xh ",data_byte);

                st = p->va[BSC_REG_S];
            }
        }

        //printf("\n");

        if (st & BSC_REG_S_ERR)
        {
            //printf("I2C slave responded with NACK\n");
            //bsc_dump_regs(p);

            io->nack_rsp = 1;
        }
        
        if (st & BSC_REG_S_DONE) break;
    }

    dlen = p->va[BSC_REG_DLEN];

    io->xfrd = io->len - (U16)dlen;

    //printf("Data Length..: %08Xh\n",dlen);

    return 1;
}


FLAG  bsc_read (BSC_MEMIO_CTX *p, BSC_I2CBUS_RW_IO *io)
{
    U32     dlen;

    //printf("bsc_read: len:%d\n",io->len);

    io->xfrd     = 0;
    io->nack_rsp = 0;

    if (io->len == 0) return 0;
    
    // Set up the READ transfer
    p->va[BSC_REG_A]    = io->slave_ad;     // Slave I2C address
    p->va[BSC_REG_DLEN] = io->len;          // Data bytes to transfer
    p->va[BSC_REG_S]    = BSC_REG_S_CLEAR;  // Clear status flags

    // Clear the FIFO, start the READ transfer
    p->va[BSC_REG_C] = BSC_REG_C_CMD_READ;

    //bsc_dump_status(p);

    for (;;)
    {
        U32     st;
        U32     cnt;

        cnt = 1000;
        for (;;)
        {
            st = p->va[BSC_REG_S];
            if (st & (BSC_REG_S_DONE | BSC_REG_S_RXD | BSC_REG_S_ERR)) break;
        
            cnt--;
            if (cnt == 0)
            {
                //printf("time-out of I2C read transfer\n");

                return 0;
            }

            usleep(100);
        }
    
        //bsc_dump_status(p);

        while (st & BSC_REG_S_RXD)
        {
            U8      data_byte;
        
            // Prevent buffer overrun (shouldn't happen though)
            if (io->xfrd == io->len) return 0;

            // Fetch the next byte from the BSC's FIFO
            data_byte         = (U8)(p->va[BSC_REG_FIFO]);
            io->buf[io->xfrd] = data_byte;
            io->xfrd++;
            
            //printf("%02Xh ",data_byte);

            st = p->va[BSC_REG_S];
        }
        
        //printf("\n");

        if (st & BSC_REG_S_ERR)
        {
            //printf("I2C slave responded with NACK\n");
            //bsc_dump_regs(p);

            io->nack_rsp = 1;
        }
        
        if (st & BSC_REG_S_DONE) break;
    }

    dlen = p->va[BSC_REG_DLEN];

    io->xfrd = io->len - (U16)dlen;

    //printf("Data Length..: %08Xh\n",dlen);

    return 1;
}


FLAG  bsc_write_reg (BSC_MEMIO_CTX *p, U8 slave_ad, U8 reg)
{
    U8                  bsc_write_reg_data;
    BSC_I2CBUS_RW_IO    bsc_write_reg_io;

    bsc_write_reg_io.buf      = &bsc_write_reg_data;
    bsc_write_reg_io.len      = 1;
    bsc_write_reg_io.slave_ad = slave_ad;
    bsc_write_reg_data        = reg;

    return bsc_write(p,&bsc_write_reg_io);
}


FLAG  bsc_read_reg (BSC_MEMIO_CTX *p, U8 slave_ad, U8 *reg)
{
    U8                  bsc_read_reg_data;
    BSC_I2CBUS_RW_IO    bsc_read_reg_io;
    FLAG                ok;

    bsc_read_reg_io.buf      = &bsc_read_reg_data;
    bsc_read_reg_io.len      = 1; 
    bsc_read_reg_io.slave_ad = slave_ad;

    ok = bsc_read(p,&bsc_read_reg_io);
    if (!ok) return 0;

    (*reg) = bsc_read_reg_data;
    return 1;
}

