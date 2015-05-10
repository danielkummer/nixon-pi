
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al.c
//
// AxiCat AL.
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
//      * Added 1-Wire enumeration.
//      * Added AXICAT_AL_Get_Version().
//      * Released v1.2.0.
//
//   2014-12-12  Peter S'heeren, Axiris
//
//      * Added 1-Wire probing.
//      * Added AXICAT_AL_Get_FW_Version().
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


// First include this file
#include "axicat_al_conf.h"

#include "axicat_al.h"

#include "ldllist.h"
#include "axicat.h"

#include <string.h>


/*-----------------------------------------------------------------------------
Defines, types, data
-----------------------------------------------------------------------------*/


// Forward declarations

typedef struct  _SERIAL_CTX     SERIAL_CTX;
typedef struct  _RX_MECH        RX_MECH;
typedef struct  _GPIO_XFR       GPIO_XFR;
typedef struct  _GPIO_CTX       GPIO_CTX;
typedef struct  _TWI_XFR        TWI_XFR;
typedef struct  _TWI_CTX        TWI_CTX;
typedef struct  _SPI_XFR        SPI_XFR;
typedef struct  _SPI_CTX        SPI_CTX;
typedef struct  _OW_XFR         OW_XFR;
typedef struct  _OW_CTX         OW_CTX;
typedef struct  _UART_CTX       UART_CTX;
typedef struct  _CTX            CTX;


// Shorthand declarations

typedef AXICAT_AL_CONN_STATE    CONN_STATE;


#ifdef  min
#undef  min
#endif

#define min(a,b)    (((a) < (b)) ? (a) : (b))


// Serial interface with the FT245.

struct  _SERIAL_CTX
{
    SERIAL_INTF        *serial_intf;

    U8                 *tx_buf;
    U32                 tx_buf_len;
    U32                 tx_si;
};


// Receive mechanism context. Processing of individual bytes received from the
// AxiCat.
//
// This structure is used for receiving response packets from the AxiCat. Each
// response packet is read byte by byte and broken down into individual fields,
// if any. Note that some data bytes aren't stored in the structure but fed to
// a specific subroutine instead, esp. data payloads are handled this way.

struct  _RX_MECH
{
    FLAG                enabled;
    U8                  state;

    union
    {
        // AXICAT_RSP_GEN_INFO
        struct
        {
            union
            {
                AXICAT_GEN_INFO     info;
                U8                  buf[sizeof(AXICAT_GEN_INFO)];
            };

            U8                  si;
        }
            info;

        // AXICAT_RSP_UART0_TX_FREE
        // AXICAT_RSP_UART1_TX_FREE
        struct
        {
            U8                  free_lsb;
        }
            uart_tx_free;

        // AXICAT_RSP_UART0_RX
        // AXICAT_RSP_UART1_RX
        struct
        {
            U8                  cnt_mo;
        }
            uart_rx;

        // AXICAT_RSP_TWI_MASTER_START
        struct
        {
            FLAG                skipped;
            FLAG                nack_rsp;
            U8                  slave_ad;
            FLAG                dir;
        }
            twi_start;

        // AXICAT_RSP_TWI_MASTER_TX
        struct
        {
            FLAG                short_packet;   // Short packet (includes skipped) y/n
            FLAG                nack_rsp;       // NACK response from slave y/n
            U8                  cnt;            // 1..32
            U8                  sent;           // 0..32
        }
            twi_mtx;

        // AXICAT_RSP_TWI_MASTER_RX
        struct
        {
            FLAG                short_packet;   // Short packet (includes skipped) y/n
            FLAG                nack_rsp;       // NACK response from master y/n
            U8                  cnt;            // 1..32
            U8                  rcvd;           // 0..32
        }
            twi_mrx;

        // AXICAT_RSP_TWI_SLAVE_TX
        struct
        {
            FLAG                short_packet;   // Short packet (includes skipped) y/n
            FLAG                nack_rsp;       // NACK response from master y/n
            U8                  cnt;            // 1..32
            U8                  sent;           // 0..32
        }
            twi_stx;

        // AXICAT_RSP_TWI_SLAVE_RX
        struct
        {
            FLAG                short_packet;   // Short packet (includes skipped) y/n
            U8                  cnt;            // 1..32
            U8                  rcvd;           // 0..32
        }
            twi_srx;

        // AXICAT_RSP_SPI_XFR
        struct
        {
            FLAG                skipped;
            FLAG                last;
            U8                  ss;
            U8                  cnt_mo;
            U8                  rcvd_mo;
        }
            spi_xfr;

        // AXICAT_RSP_OW_RESET
        struct
        {
            FLAG                skipped;
            FLAG                pd;             // Presence detected y/n
        }
            ow_reset;

        // AXICAT_RSP_OW_TOUCH_BITS
        struct
        {
            FLAG                short_packet;   // Short packet (includes skipped) y/n
            FLAG                spu;
            U8                  cnt_mo;
            U8                  rcvd_mo;
        }
            ow_touch_bits;

        // AXICAT_RSP_OW_ENUM
        struct
        {
            FLAG                skipped;
            FLAG                found;
            U8                  rom_code[8];
            U8                  si;
        }
            ow_enum;

        // AXICAT_RSP_OW_PROBE
        struct
        {
            FLAG                skipped;
            FLAG                found;
        }
            ow_probe;

        // AXICAT_RSP_GEN_VERSION
        struct
        {
            U8                  si;
        }
            version;
    };
};


// Location identifier for GPIO transfers

#define GPIO_XFR_LOC_CACHED         0
#define GPIO_XFR_LOC_ASSIGNED       1
#define GPIO_XFR_LOC_SCHEDULED      2


// GPIO transfer context

struct  _GPIO_XFR
{
    CTX                *ctx;                    // Parent context

    LDLL_ITEM           list_item;              // Always linked
    U8                  loc;                    // Location identifier (GPIO_XFR_LOC_Xxx constant)
    FLAG                active;                 // Active command in AxiCat y/n
    FLAG                detached;               // Transfer is detached from user y/n

    U8                  pin;                    // GPIO pin (0..16)

    AXICAT_AL_GPIO_XFR  user;                   // User-accessible part
};


// GPIO context
//
// Since a GPIO transfer always issues one GPIO command, field cur_tx_xfr
// points to the transfer that'll issue the next GPIO command.

struct  _GPIO_CTX
{
    // Cached transfers
    LDLL_BASE           cached_xfr_list_base;
    U32                 cached_xfr_cnt;

    // Assigned transfers
    LDLL_BASE           assigned_xfr_list_base;
    U32                 assigned_xfr_cnt;

    // Scheduled transfers
    LDLL_BASE           sch_xfr_list_base;
    U32                 sch_xfr_cnt;
    GPIO_XFR           *cur_tx_xfr;
};


// Location identifier for TWI transfers

#define TWI_XFR_LOC_CACHED          0
#define TWI_XFR_LOC_ASSIGNED        1
#define TWI_XFR_LOC_SCHEDULED_M     2
#define TWI_XFR_LOC_SCHEDULED_STX   3
#define TWI_XFR_LOC_SCHEDULED_SRX   4


// TWI transfer context

struct  _TWI_XFR
{
    CTX                *ctx;                    // Parent context

    LDLL_ITEM           list_item;              // Always linked
    U8                  loc;                    // Location identifier (TWI_XFR_LOC_Xxx constant)
    FLAG                active;                 // Active command(s) in AxiCat y/n
    FLAG                detached;               // Transfer is detached from user y/n

    U8                 *buf;                    // Read/write buffer
    U16                 buf_sc;                 // Store count: bytes to read or write
    U16                 cmd_byte_cnt;           // Bytes covered by the currently transmitted command packets
    U16                 rsp_byte_cnt;           // Bytes covered by the currently received response packets
    U16                 xfrd;                   // Bytes transferred
    U8                  slave_ad;               // Slave address (0..127)
    FLAG                dir;                    // Direction (AXICAT_TWI_DIR_Xxx)
    FLAG                force_stop;
    FLAG                last;
    U8                  tx_phase;               // Tx phase - Phase of transfer processing:
    U8                  rx_phase;               // Rx phase /   =0: START + SLA+W/R
                                                //              =1: Tx or Rx data bytes
    FLAG                short_packet;           // Short_packet detected y/n:
                                                // * Less bytes transmitted than requested.
                                                // * Response packet marked as skipped (zero bytes transmitted)
    FLAG                nack_rsp;               // NACK (1) or ACK (0) response on SLA+W, SLA+R or last data byte
    FLAG                skipped;                // Master transfer: Master start command was skipped y/n
                                                // Slave Tx/Rx transfer: First command of payload was skipped y/n
    AXICAT_AL_TWI_XFR   user;                   // User-accessible part
};


// TWI context
//
// Flag hw_enabled is set when the TWI ENABLE or TWI DISABLE command is emitted
// to the Tx FIFO. With regard to sending and receiving master commands, it's
// always correct to check this flag.

struct  _TWI_CTX
{
    // Cached transfers
    LDLL_BASE           cached_xfr_list_base;
    U32                 cached_xfr_cnt;

    // Assigned transfers
    LDLL_BASE           assigned_xfr_list_base;
    U32                 assigned_xfr_cnt;

    // Scheduled master transfers
    LDLL_BASE           sch_m_xfr_list_base;
    U32                 sch_m_xfr_cnt;
    TWI_XFR            *cur_m_tx_xfr;

    // Tracking of AxiCat TWI master buffer space
    U32                 hw_m_buf_len;
    U32                 hw_m_buf_free;

    FLAG                hw_m_tx_stop;           // Send master stop command y/n

    // Scheduled slave Tx transfers
    LDLL_BASE           sch_stx_xfr_list_base;
    U32                 sch_stx_xfr_cnt;
    TWI_XFR            *cur_stx_tx_xfr;

    // Tracking of AxiCat TWI slave Tx buffer space
    U32                 hw_stx_buf_len;
    U32                 hw_stx_buf_free;

    // Scheduled slave Rx transfers
    LDLL_BASE           sch_srx_xfr_list_base;
    U32                 sch_srx_xfr_cnt;
    TWI_XFR            *cur_srx_tx_xfr;

    // Tracking of AxiCat TWI slave Rx buffer space
    U32                 hw_srx_buf_len;
    U32                 hw_srx_buf_free;

    FLAG                hw_enabled;

    FLAG                bus_error;
    FLAG                arb_lost;
};


// Location identifier for SPI transfers

#define SPI_XFR_LOC_CACHED          0
#define SPI_XFR_LOC_ASSIGNED        1
#define SPI_XFR_LOC_SCHEDULED       2


// SPI transfer context

struct  _SPI_XFR
{
    CTX                *ctx;                    // Parent context

    LDLL_ITEM           list_item;              // Always linked
    U8                  loc;                    // Location identifier (SPI_XFR_LOC_Xxx constant)
    FLAG                active;                 // Active command(s) in AxiCat y/n
    FLAG                detached;               // Transfer is detached from user y/n

    U8                 *buf;                    // Read/write buffer
    U16                 buf_fi;                 // Fetch index
    U16                 buf_si;                 // Store index
    U16                 buf_sc;                 // Store count: bytes to read/write
    U16                 xfrd;                   // Bytes transferred
    U8                  ss;                     // Slave select line (0..3)
    FLAG                skipped;                // Skipped y/n

    AXICAT_AL_SPI_XFR   user;                   // User-accessible part
};


// SPI context

struct  _SPI_CTX
{
    // Cached transfers
    LDLL_BASE           cached_xfr_list_base;
    U32                 cached_xfr_cnt;

    // Assigned transfers
    LDLL_BASE           assigned_xfr_list_base;
    U32                 assigned_xfr_cnt;

    // Scheduled transfers
    LDLL_BASE           sch_xfr_list_base;
    U32                 sch_xfr_cnt;
    SPI_XFR            *cur_tx_xfr;

    // Tracking of AxiCat SPI buffer space
    U32                 hw_buf_len;
    U32                 hw_buf_free;
};


// Location identifier for 1-Wire transfers

#define OW_XFR_LOC_CACHED           0
#define OW_XFR_LOC_ASSIGNED         1
#define OW_XFR_LOC_SCHEDULED        2


// 1-Wire transfer context
//
// Notes:
// * Value active_packets is used when short packet is detected.

struct  _OW_XFR
{
    CTX                *ctx;                    // Parent context

    LDLL_ITEM           list_item;              // Always linked
    U8                  loc;                    // Location identifier (OW_XFR_LOC_Xxx constant)
    FLAG                active;                 // Active command(s) in AxiCat y/n
    FLAG                detached;               // Transfer is detached from user y/n

    // id=AXICAT_AL_OW_XFR_ID_TOUCH_BITS
    U8                 *buf;                    // Read/write buffer

    union
    {
        // id=AXICAT_AL_OW_XFR_ID_TOUCH_BITS
        struct
        {
            U16                 buf_fi;                 // Fetch index
            U16                 buf_si;                 // Store index
            U16                 buf_sc;                 // Store count: bits to transfer
            U16                 xfrd;                   // Bits transferred
        }
            touch_bits;
    };

    U16                 active_packets;         // Number of command packets active in the AxiCat
    FLAG                skipped;                // Skipped y/n
    U8                  id;                     // Transfer identifier (AXICAT_AL_OW_XFR_ID_Xxx)

    AXICAT_AL_OW_XFR    user;                   // User-accessible part
};


// 1-Wire context

struct  _OW_CTX
{
    // Cached transfers
    LDLL_BASE           cached_xfr_list_base;
    U32                 cached_xfr_cnt;

    // Assigned transfers
    LDLL_BASE           assigned_xfr_list_base;
    U32                 assigned_xfr_cnt;

    // Scheduled transfers
    LDLL_BASE           sch_xfr_list_base;
    U32                 sch_xfr_cnt;
    OW_XFR             *cur_tx_xfr;

    // Tracking of AxiCat 1-Wire buffer space
    U32                 hw_buf_len;
    U32                 hw_buf_free;
};


// UART context

struct  _UART_CTX
{
    // Tx FIFO
    U8                 *tx_buf;
    U32                 tx_buf_len;
    U32                 tx_si;
    U32                 tx_fi;
    U32                 tx_sc;

    // Rx FIFO
    U8                 *rx_buf;
    U32                 rx_buf_len;
    U32                 rx_si;
    U32                 rx_fi;
    U32                 rx_sc;

    // Settings and state in the AxiCat
    U32                 hw_tx_buf_len;
    U32                 hw_tx_free;
    FLAG                wide;
};


// AL context

struct  _CTX
{
    SERIAL_CTX          serial;                 // Serial I/O with AxiCat

    RX_MECH             rx;
    FLAG                buf_info_read;

    U8                  version_utf8z[64];

    GPIO_CTX            gpio;                   // GPIO context
    TWI_CTX             twi;                    // TWI context
    SPI_CTX             spi;                    // SPI context
    OW_CTX              ow;                     // 1-Wire context
    UART_CTX            uarts[2];               // UART #0 and UART #1 context

    CONN_STATE          conn_state;             // Connection state
    U8                  state_main;
    U32                 ticks;                  // Timer start ticke
    U32                 timeout;                // Timeout ticks
    FLAG                wait_idle;              // Wait receive period (0) or idle period (1)
    AXICAT_FW_VERSION   fw_version;             // Firmware version (FW_VERSION_Xxx)
};


static
VOID  GPIO_Complete_All_Xfrs (GPIO_CTX *gpio);

static
VOID  SPI_Complete_All_Xfrs (SPI_CTX *spi);

static
VOID  TWI_Complete_All_Xfrs (TWI_CTX *twi);


/*-----------------------------------------------------------------------------
FLAG  Is_Detached (CTX *ctx)
-----------------------------------------------------------------------------*/


static
FLAG  Is_Detached (CTX *ctx)
{
    return (ctx->conn_state == AXICAT_AL_CONN_STATE_DISCONNECTED);
}


/*-----------------------------------------------------------------------------
VOID  Mark_Detached (CTX *ctx)
-----------------------------------------------------------------------------*/


static
VOID  Mark_Detached (CTX *ctx)
{
    // Only process the transition to detached state once
    if (Is_Detached(ctx)) return;

    ctx->conn_state = AXICAT_AL_CONN_STATE_DISCONNECTED;

    // Forcibly complete all scheduled transfers as cancelled. The main thing
    // are is to neglect the active flag of a transfer.
    //
    GPIO_Complete_All_Xfrs(&ctx->gpio);
    SPI_Complete_All_Xfrs(&ctx->spi);
    TWI_Complete_All_Xfrs(&ctx->twi);
}


/*-----------------------------------------------------------------------------
VOID  AXICAT_AL_Dbg_Mark_Detached (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


#ifdef  _DEBUG


VOID  AXICAT_AL_Dbg_Mark_Detached (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    Mark_Detached(ctx);
}


#endif  // _DEBUG


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
U8 *  DynllCall  AXICAT_AL_Get_Version (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
U8 *  DynllCall  AXICAT_AL_Get_Version (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return ctx->version_utf8z;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
AXICAT_FW_VERSION  DynllCall  AXICAT_AL_Get_FW_Version
(
    AXICAT_AL_HANDLE    handle
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
AXICAT_FW_VERSION  DynllCall  AXICAT_AL_Get_FW_Version
(
    AXICAT_AL_HANDLE    handle
)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return ctx->fw_version;
}


/*-----------------------------------------------------------------------------
FLAG  Tx_Flush (CTX *ctx)
-----------------------------------------------------------------------------*/


static
FLAG  Tx_Flush (CTX *ctx)
{
    FLAG    ok;
    U32     xfrd;

    if (ctx->serial.tx_si == 0) return 1;

    // Write the transmit buffer
    ok = ctx->serial.serial_intf->fn_table->write_fn
            (
                ctx->serial.serial_intf,
                ctx->serial.tx_buf,
                ctx->serial.tx_si,
                &xfrd
            );
    if (!ok) goto Err;
    if (xfrd < ctx->serial.tx_si) goto Err;

    // Reset the transmit buffer
    ctx->serial.tx_si = 0;

    return 1;

  Err:

    Mark_Detached(ctx);
    return 0;
}


/*-----------------------------------------------------------------------------
FLAG  Tx_Byte (CTX *ctx, U8 byte)
-----------------------------------------------------------------------------*/


static
FLAG  Tx_Byte (CTX *ctx, U8 byte)
{
    ctx->serial.tx_buf[ctx->serial.tx_si] = byte;
    ctx->serial.tx_si++;

    if (ctx->serial.tx_si == ctx->serial.tx_buf_len)
    {
        return Tx_Flush(ctx);
    }
    else
    {
        return 1;
    }
}


/*-----------------------------------------------------------------------------
GPIO
-----------------------------------------------------------------------------*/


#include "axicat_al_gpio.c"


/*-----------------------------------------------------------------------------
TWI
-----------------------------------------------------------------------------*/


#include "axicat_al_twi.c"


/*-----------------------------------------------------------------------------
SPI
-----------------------------------------------------------------------------*/


#include "axicat_al_spi.c"


/*-----------------------------------------------------------------------------
UARTs
-----------------------------------------------------------------------------*/


#include "axicat_al_uart.c"


/*-----------------------------------------------------------------------------
1-Wire
-----------------------------------------------------------------------------*/


#include "axicat_al_ow.c"


/*-----------------------------------------------------------------------------
FLAG  Step (CTX *ctx)
-----------------------------------------------------------------------------*/


#define STATE_MAIN_START    0   // Must be zero!
#define STATE_MAIN_W1       1
#define STATE_MAIN_W2       2
#define STATE_MAIN_W3       3


static
FLAG  Step_Main (CTX *ctx)
{
    FLAG    ok;
    U32     cur_ticks;
    U32     delta_ticks;
    U32     xfrd;
    U32     total_xfrd;
    U8      purge_rx_buf[32];
    U32     u;


    switch (ctx->state_main)
    {
        case STATE_MAIN_START:  goto Start;
        case STATE_MAIN_W1:     goto W1;
        case STATE_MAIN_W2:     goto W2;
        case STATE_MAIN_W3:     goto W3;
    }

    // Shouldn't end up here

  Wait:

    return 1;


  Err:

    Mark_Detached(ctx);
    return 0;


  Start:

    //
    // Initialize
    //

    // Initialize hardware:
    // * Flush out any active command by transmitting a string of NOP commands.
    // * Purge any remaining data bytes from the Rx FIFO in the AxiCat.
    // * Disable special functions. TWI and SPI commands that are still present
    //   in the AxiCat's buffers will be completed as skipped.
    // * Force the AxiCat to send some data. This will put some data in the Rx
    //   FIFO in none is present.
    //
    // We don't program default settings. It's up to the user to set up each
    // interface as required.

    // Send 32 NOP commands
    for (u = 0; u < 32; u++) if (!Tx_Byte(ctx,AXICAT_CMD_GEN_NOP)) goto Err;

    // UART0 DISABLE
    if (!Tx_Byte(ctx,AXICAT_CMD_UART0_DISABLE)) goto Err;

    // UART1 DISABLE
    if (!Tx_Byte(ctx,AXICAT_CMD_UART1_DISABLE)) goto Err;

    // SPI DISABLE
    if (!Tx_Byte(ctx,AXICAT_CMD_SPI_DISABLE)) goto Err;

    // 1-WIRE DISABLE
    if (!Tx_Byte(ctx,AXICAT_CMD_OW_DISABLE)) goto Err;

    // TWI SLAVE DISABLE
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_SLAVE_DISABLE)) goto Err;

    // TWI DISABLE -> TWI MASTER START -> TWI DISABLE: This sequence forces the
    // AxiCat to send a response packet i.e. MASTER START completed as skipped.
    // At least, if the AxiCat is up and running.

    // TWI DISABLE
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_DISABLE)) goto Err;

    // TWI MASTER START (address and direction are irrelevant here)
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_MASTER_START)) goto Err;
    if (!Tx_Byte(ctx,0x80)) goto Err;

    // TWI DISABLE
    if (!Tx_Byte(ctx,AXICAT_CMD_TWI_DISABLE)) goto Err;

    if (!Tx_Flush(ctx)) goto Err;


    // Purge Rx buffer:
    // 1. Wait for data bytes to arrived. This data includes the skipped TWI
    //    MASTER START command we've just forced in the AxiCat.
    // 2. When data has arrived, wait for an idle time (no received data)
    //    before proceeding. Note that data may arrive in multiple reads.
    //
    // Typical situation:
    //
    // --------XXXX------XXXX------------------------------> t
    // |       |   |     |   |           |
    // start   |   start |   start       timeout
    // receive |   idle  |   idle        idle
    // timer   |   timer |   timer       period
    //         |         |          
    //         data      data
    //         received  received
    //
    // Situation were the AxiCat doesn't respond:
    //
    // --------------------------------------> t
    // |                   |
    // start               timeout
    // receive             receive
    // timer               period

    ctx->state_main = STATE_MAIN_W1;
    ctx->ticks      = AXICAT_AL_Get_Ticks();
    ctx->timeout    = 2000;
    ctx->wait_idle  = 0;
    goto Wait;

  W1:
    total_xfrd = 0;
    for (;;)
    {
        ok = ctx->serial.serial_intf->fn_table->read_fn
                (
                    ctx->serial.serial_intf,
                    purge_rx_buf,
                    sizeof(purge_rx_buf),
                    &xfrd
                );
        if (!ok) goto Err;
        if (xfrd == 0) break;

        total_xfrd += xfrd;
    }

    if (total_xfrd > 0)
    {
        // We've received data. Restart the timer for idle detection.
        ctx->ticks     = AXICAT_AL_Get_Ticks();
        ctx->timeout   = 200;
        ctx->wait_idle = 1;

        goto Wait;
    }
    else
    {
        // Check for timeout
        cur_ticks   = AXICAT_AL_Get_Ticks();
        delta_ticks = cur_ticks - ctx->ticks;
        if (delta_ticks < ctx->timeout) goto Wait;

        // Fail if no data came in during the receive period
        if (ctx->wait_idle == 0) goto Err;
    }


    // Enable the Rx mechanism

    ctx->rx.enabled = 1;


    // Read information block and version string
    //
    if (!Tx_Byte(ctx,AXICAT_CMD_GEN_INFO)) goto Err;
    if (!Tx_Byte(ctx,AXICAT_CMD_GEN_VERSION)) goto Err;
    if (!Tx_Flush(ctx)) goto Err;

    ctx->state_main = STATE_MAIN_W2;
  W2:
    if (!ctx->buf_info_read) goto Wait;


    //
    // Connected
    //

    ctx->conn_state = AXICAT_AL_CONN_STATE_CONNECTED;

    ctx->state_main = STATE_MAIN_W3;

  W3:

    goto Wait;
}


#define STATE_RX_RSP                    0   // Must be zero!
#define STATE_RX_VERSION                1
#define STATE_RX_INFO                   2
#define STATE_RX_GPIO_READ              3
#define STATE_RX_UART0_TX_FREE_LSB      4
#define STATE_RX_UART0_TX_FREE_MSB      5
#define STATE_RX_UART1_TX_FREE_LSB      6
#define STATE_RX_UART1_TX_FREE_MSB      7
#define STATE_RX_UART0_RX_CNT           8
#define STATE_RX_UART0_RX_DATA          9
#define STATE_RX_UART1_RX_CNT          10
#define STATE_RX_UART1_RX_DATA         11
#define STATE_RX_SPI_XFR_CNT           12
#define STATE_RX_SPI_XFR_RCVD          13
#define STATE_RX_SPI_XFR_DATA          14
#define STATE_RX_TWI_MASTER_START      15
#define STATE_RX_TWI_MASTER_TX_CNT     16
#define STATE_RX_TWI_MASTER_TX_SENT    17
#define STATE_RX_TWI_MASTER_RX_CNT     18
#define STATE_RX_TWI_MASTER_RX_RCVD    19
#define STATE_RX_TWI_MASTER_RX_DATA    20
#define STATE_RX_TWI_SLAVE_TX_CNT      21
#define STATE_RX_TWI_SLAVE_TX_SENT     22
#define STATE_RX_TWI_SLAVE_RX_CNT      23
#define STATE_RX_TWI_SLAVE_RX_RCVD     24
#define STATE_RX_TWI_SLAVE_RX_DATA     25
#define STATE_RX_OW_TOUCH_BITS_CNT     26
#define STATE_RX_OW_TOUCH_BITS_RCVD    27
#define STATE_RX_OW_TOUCH_BITS_DATA    28
#define STATE_RX_OW_ENUM_ROM_CODE      29


static
FLAG  Step_Rx (CTX *ctx)
{
    U8      data_byte;
    U8      rsp_code;
    U32     xfrd;
    FLAG    ok;

    for (;;)
    {
        ok = ctx->serial.serial_intf->fn_table->read_fn
                    (
                        ctx->serial.serial_intf,
                        &data_byte,
                        1,
                        &xfrd
                    );
        if (!ok) goto Err;
        if (xfrd == 0) break;

/*
        // DBG. - debug-print all received data bytes
        if (ctx->rx.state == STATE_RX_RSP) printf("\n");
        printf("%02X ",data_byte);
*/

        //
        // Response codes
        //

        if (ctx->rx.state == STATE_RX_RSP)
        {
            // Received a response code

            if (data_byte == AXICAT_RSP_GEN_VERSION)
            {
                // Prepare to receive version string
                ctx->rx.state      = STATE_RX_VERSION;
                ctx->rx.version.si = 0;
            }
            else
            if (data_byte == AXICAT_RSP_GEN_INFO)
            {
                // Prepare to receive buffer information
                ctx->rx.state   = STATE_RX_INFO;
                ctx->rx.info.si = 0;
            }
            else
            if (data_byte == AXICAT_RSP_GPIO_READ)
            {
                // Prepare to receive GPIO read data
                ctx->rx.state = STATE_RX_GPIO_READ;
            }
            else
            if (data_byte == AXICAT_RSP_UART0_TX_FREE)
            {
                // Prepare to receive UART0 Tx free. LSB comes first.
                ctx->rx.state = STATE_RX_UART0_TX_FREE_LSB;
            }
            else
            if (data_byte == AXICAT_RSP_UART1_TX_FREE)
            {
                // Prepare to receive UART1 Tx free. LSB comes first.
                ctx->rx.state = STATE_RX_UART1_TX_FREE_LSB;
            }
            else
            if (data_byte == AXICAT_RSP_UART0_RX)
            {
                // Prepare to receive UART0 Rx data. Count minus one comes
                // first.
                ctx->rx.state = STATE_RX_UART0_RX_CNT;
            }
            else
            if (data_byte == AXICAT_RSP_UART1_RX)
            {
                // Prepare to receive UART1 Rx data. Count minus one comes
                // first.
                ctx->rx.state = STATE_RX_UART1_RX_CNT;
            }
            else
            if (data_byte == AXICAT_RSP_TWI_BUS_ERROR)
            {
                // Mark bus error
                ctx->twi.bus_error = 1;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            if (data_byte == AXICAT_RSP_TWI_ARB_LOST)
            {
                // Mark arbitration lost
                ctx->twi.arb_lost = 1;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            {
                // Process response codes with variable bits 7-6

                rsp_code = data_byte & 0x3F;

                if (rsp_code == AXICAT_RSP_TWI_MASTER_START)
                {
                    // Bit 6: NACK response y/n
                    ctx->rx.twi_start.nack_rsp = (data_byte & 0x40) ? 1 : 0;

                    // Bit 7: skipped y/n
                    ctx->rx.twi_start.skipped = (data_byte & 0x80) ? 1 : 0;

                    // Prepare to receive the next byte
                    ctx->rx.state = STATE_RX_TWI_MASTER_START;
                }
                else
                if (rsp_code == AXICAT_RSP_TWI_MASTER_STOP)
                {
                    // Bit 6 must be zero
                    if (data_byte & 0x40) goto Err;

                    // Bit 7: skipped y/n
                    //
                    // Not required

                    ctx->twi.hw_m_buf_free += 1;

                    // Response packet processed
                    ctx->rx.state = STATE_RX_RSP;
                }
                else
                if (rsp_code == AXICAT_RSP_TWI_MASTER_TX)
                {
                    // Bit 6: NACK response y/n
                    ctx->rx.twi_mtx.nack_rsp = (data_byte & 0x40) ? 1 : 0;

                    // Bit 7: skipped y/n
                    ctx->rx.twi_mtx.short_packet = (data_byte & 0x80) ? 1 : 0;

                    // Prepare to receive the next byte
                    ctx->rx.state = STATE_RX_TWI_MASTER_TX_CNT;
                }
                else
                if (rsp_code == AXICAT_RSP_TWI_MASTER_RX)
                {
                    // Bit 6: NACK response y/n
                    ctx->rx.twi_mrx.nack_rsp = (data_byte & 0x40) ? 1 : 0;

                    // Bit 7: skipped y/n
                    ctx->rx.twi_mrx.short_packet = (data_byte & 0x80) ? 1 : 0;

                    // Prepare to receive the next byte
                    ctx->rx.state = STATE_RX_TWI_MASTER_RX_CNT;
                }
                else
                if (rsp_code == AXICAT_RSP_TWI_SLAVE_TX)
                {
                    // Bit 6: NACK response y/n
                    ctx->rx.twi_stx.nack_rsp = (data_byte & 0x40) ? 1 : 0;

                    // Bit 7: skipped y/n
                    ctx->rx.twi_stx.short_packet = (data_byte & 0x80) ? 1 : 0;

                    // Prepare to receive the next byte
                    ctx->rx.state = STATE_RX_TWI_SLAVE_TX_CNT;
                }
                else
                if (rsp_code == AXICAT_RSP_TWI_SLAVE_RX)
                {
                    // Bit 6 must be zero
                    if (data_byte & 0x40) goto Err;

                    // Bit 7: skipped y/n
                    ctx->rx.twi_srx.short_packet = (data_byte & 0x80) ? 1 : 0;

                    // Prepare to receive the next byte
                    ctx->rx.state = STATE_RX_TWI_SLAVE_RX_CNT;
                }
                else
                if (rsp_code == AXICAT_RSP_SPI_XFR)
                {
                    // Bit 6 must be zero
                    if (data_byte & 0x40) goto Err;

                    // Bit 7: skipped y/n
                    ctx->rx.spi_xfr.skipped = (data_byte & 0x80) ? 1 : 0;

                    // Prepare to receive the next byte
                    ctx->rx.state = STATE_RX_SPI_XFR_CNT;
                }
                else
                if (rsp_code == AXICAT_RSP_OW_RESET)
                {
                    // Bit 6: presence detected y/n
                    ctx->rx.ow_reset.pd = (data_byte & 0x40) ? 1 : 0;

                    // Bit 7: skipped y/n
                    ctx->rx.ow_reset.skipped = (data_byte & 0x80) ? 1 : 0;

                    // We can increment the free byte count here as the AxiCat
                    // has removed the command from its buffer at this point.
                    ctx->ow.hw_buf_free++;

                    ok = OW_Rx_Reset(ctx);
                    if (!ok) goto Err;
                }
                else
                if (rsp_code == AXICAT_RSP_OW_TOUCH_BITS)
                {
                    // Bit 6 must be zero
                    if (data_byte & 0x40) goto Err;

                    // Bit 7: skipped y/n
                    ctx->rx.ow_touch_bits.short_packet = (data_byte & 0x80) ? 1 : 0;

                    // Prepare to receive the next byte
                    ctx->rx.state = STATE_RX_OW_TOUCH_BITS_CNT;
                }
                else
                if (rsp_code == AXICAT_RSP_OW_ENUM)
                {
                    // Bit 7: skipped y/n
                    ctx->rx.ow_enum.skipped = (data_byte & 0x80) ? 1 : 0;

                    // Bit 6: Found y/n
                    ctx->rx.ow_enum.found = (data_byte & 0x40) ? 1 : 0;

                    // We can increment the free byte count here as the AxiCat
                    // has removed the command from its buffer at this point.
                    ctx->ow.hw_buf_free += 11;

                    if (ctx->rx.ow_enum.found)
                    {
                        // Prepare to receive the ROM code
                        ctx->rx.state      = STATE_RX_OW_ENUM_ROM_CODE;
                        ctx->rx.ow_enum.si = 0;
                    }
                    else
                    {
                        ok = OW_Rx_Enum(ctx);
                        if (!ok) goto Err;

                        // Response packet processed
                        ctx->rx.state = STATE_RX_RSP;
                    }
                }
                else
                if (rsp_code == AXICAT_RSP_OW_PROBE)
                {
                    // Bit 7: skipped y/n
                    ctx->rx.ow_probe.skipped = (data_byte & 0x80) ? 1 : 0;

                    // Bit 6: Found y/n
                    ctx->rx.ow_probe.found = (data_byte & 0x40) ? 1 : 0;

                    // We can increment the free byte count here as the AxiCat
                    // has removed the command from its buffer at this point.
                    ctx->ow.hw_buf_free += 9;

                    ok = OW_Rx_Probe(ctx);
                    if (!ok) goto Err;

                    // Response packet processed
                    ctx->rx.state = STATE_RX_RSP;
                }
                else
                    goto Err;
            }
        }
        else

        //
        // Buffer information response
        //

        if (ctx->rx.state == STATE_RX_VERSION)
        {
            ctx->version_utf8z[ctx->rx.version.si] = data_byte;

            if (data_byte != 0)
            {
                ctx->rx.version.si++;
                if (ctx->rx.version.si == sizeof(ctx->version_utf8z)-1)
                {
                    // Terminate string
                    ctx->version_utf8z[ctx->rx.version.si] = 0;

                    goto Err;
                }
            }
            else
            {
                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else
        if (ctx->rx.state == STATE_RX_INFO)
        {
            if (ctx->rx.info.si == 0)
            {
                // The length value
                if (data_byte < sizeof(AXICAT_GEN_INFO)) goto Err;
            }

            if (ctx->rx.info.si < sizeof(AXICAT_GEN_INFO))
            {
                // Only store data bytes in the supported structure. If the
                // AxiCat reports more bytes, it has a newer firmware version.
                ctx->rx.info.buf[ctx->rx.info.si] = data_byte;
            }

            ctx->rx.info.si++;

            // Check for end of data block
            if (ctx->rx.info.si == ctx->rx.info.info.len)
            {
                // Process the received values

                ctx->uarts[0].hw_tx_buf_len = ctx->rx.info.info.uart_tx_len_lsb + (ctx->rx.info.info.uart_tx_len_msb << 8) + 1;
                ctx->uarts[1].hw_tx_buf_len = ctx->uarts[0].hw_tx_buf_len;

                ctx->twi.hw_m_buf_len       = ctx->rx.info.info.twi_m_len_lsb + (ctx->rx.info.info.twi_m_len_msb << 8) + 1;
                ctx->twi.hw_m_buf_free      = ctx->twi.hw_m_buf_len;

                ctx->twi.hw_stx_buf_len     = ctx->rx.info.info.twi_stx_len_lsb + (ctx->rx.info.info.twi_stx_len_msb << 8) + 1;
                ctx->twi.hw_stx_buf_free    = ctx->twi.hw_stx_buf_len;

                ctx->twi.hw_srx_buf_len     = ctx->rx.info.info.twi_srx_len_lsb + (ctx->rx.info.info.twi_srx_len_msb << 8) + 1;
                ctx->twi.hw_srx_buf_free    = ctx->twi.hw_srx_buf_len;

                ctx->spi.hw_buf_len         = ctx->rx.info.info.spi_len_lsb + (ctx->rx.info.info.spi_len_msb << 8) + 1;
                ctx->spi.hw_buf_free        = ctx->spi.hw_buf_len;

                ctx->ow.hw_buf_len          = ctx->rx.info.info.ow_len_lsb + (ctx->rx.info.info.ow_len_msb << 8) + 1;
                ctx->ow.hw_buf_free         = ctx->ow.hw_buf_len;

                // Figure out the firmware version, up to the version this
                // version of this module knows.
                //
                // Default version is v1.2.0.
                //
                ctx->fw_version = min(ctx->rx.info.info.version,AXICAT_FW_VERSION_1_3_0);

                // Mark buffer information has read
                ctx->buf_info_read = 1;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // GPIO read response
        //

        if (ctx->rx.state == STATE_RX_GPIO_READ)
        {
            if (!GPIO_Rx(ctx,data_byte)) goto Err;

            // Response packet processed
            ctx->rx.state = STATE_RX_RSP;
        }
        else

        //
        // UART0 Tx free response
        //

        if (ctx->rx.state == STATE_RX_UART0_TX_FREE_LSB)
        {
            ctx->rx.uart_tx_free.free_lsb = data_byte;

            // Next byte will be the MSB
            ctx->rx.state = STATE_RX_UART0_TX_FREE_MSB;
        }
        else
        if (ctx->rx.state == STATE_RX_UART0_TX_FREE_MSB)
        {
            U32     tx_free;

            tx_free = (data_byte << 8) + ctx->rx.uart_tx_free.free_lsb + 1;

            if (tx_free > ctx->uarts[0].hw_tx_buf_len) goto Err;

            ctx->uarts[0].hw_tx_free = tx_free;

            // Response packet processed
            ctx->rx.state = STATE_RX_RSP;
        }
        else

        //
        // UART1 Tx free response
        //

        if (ctx->rx.state == STATE_RX_UART1_TX_FREE_LSB)
        {
            ctx->rx.uart_tx_free.free_lsb = data_byte;

            // Next byte will be the MSB
            ctx->rx.state = STATE_RX_UART1_TX_FREE_MSB;
        }
        else
        if (ctx->rx.state == STATE_RX_UART1_TX_FREE_MSB)
        {
            U32     tx_free;

            tx_free = (data_byte << 8) + ctx->rx.uart_tx_free.free_lsb + 1;

            if (tx_free > ctx->uarts[1].hw_tx_buf_len) goto Err;

            ctx->uarts[1].hw_tx_free = tx_free;

            // Response packet processed
            ctx->rx.state = STATE_RX_RSP;
        }
        else

        //
        // UART0 Rx response
        //

        if (ctx->rx.state == STATE_RX_UART0_RX_CNT)
        {
            ctx->rx.uart_rx.cnt_mo = data_byte;

            // Next byte will be an Rx data byte
            ctx->rx.state = STATE_RX_UART0_RX_DATA;
        }
        else
        if (ctx->rx.state == STATE_RX_UART0_RX_DATA)
        {
            UART_Rx_Byte(ctx,0,data_byte);

            if (ctx->rx.uart_rx.cnt_mo > 0)
            {
                // One or more data bytes expected
                ctx->rx.uart_rx.cnt_mo--;
            }
            else
            {
                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // UART1 Rx response
        //

        if (ctx->rx.state == STATE_RX_UART1_RX_CNT)
        {
            ctx->rx.uart_rx.cnt_mo = data_byte;

            // Next byte will be an Rx data byte
            ctx->rx.state = STATE_RX_UART1_RX_DATA;
        }
        else
        if (ctx->rx.state == STATE_RX_UART1_RX_DATA)
        {
            UART_Rx_Byte(ctx,1,data_byte);

            if (ctx->rx.uart_rx.cnt_mo > 0)
            {
                // One or more data bytes expected
                ctx->rx.uart_rx.cnt_mo--;
            }
            else
            {
                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // TWI master start response
        //

        if (ctx->rx.state == STATE_RX_TWI_MASTER_START)
        {
            // Bit 0: direction write(0)/read(1)
            ctx->rx.twi_start.dir = (data_byte & 0x01) ? AXICAT_AL_TWI_DIR_READ : AXICAT_AL_TWI_DIR_WRITE;

            // Bits 7-1: slave address
            ctx->rx.twi_start.slave_ad = (data_byte >> 1);

            ctx->twi.hw_m_buf_free += 2;

            // Report the reception of the response packet
            ok = TWI_Rx_Start(ctx);
            if (!ok) goto Err;

            // Response packet processed
            ctx->rx.state = STATE_RX_RSP;
        }
        else

        //
        // TWI master Tx response
        //

        if (ctx->rx.state == STATE_RX_TWI_MASTER_TX_CNT)
        {
            // Response packet bytes:
            // +00: sn______ b: Skipped, NACK response from slave, response code
            // +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
            // Skipped=0:
            //   +02: 000nnnnn b: Transmitted bytes minus one (0..31, corresponding with 1..32)

            ctx->rx.twi_mtx.cnt  = (data_byte & 0x1F) + 1;
            ctx->rx.twi_mtx.sent = 0;

            // We can increment the free byte count here as the AxiCat has
            // removed the command from its buffer at this point.
            ctx->twi.hw_m_buf_free += (ctx->rx.twi_mtx.cnt + 3);

            // Consistency check
            if (ctx->twi.hw_m_buf_free > ctx->twi.hw_m_buf_len) goto Err;

            if (ctx->rx.twi_mtx.short_packet)
            {
                // Report the reception of the response packet
                ok = TWI_Rx_Master_Tx_Rsp(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            {
                ctx->rx.state = STATE_RX_TWI_MASTER_TX_SENT;
            }
        }
        else
        if (ctx->rx.state == STATE_RX_TWI_MASTER_TX_SENT)
        {
            // Bits 7-5 must be zero
            if (data_byte & 0xE0) goto Err;

            // Bits 4-0: sent bytes minus one
            ctx->rx.twi_mtx.sent = (data_byte & 0x1F) + 1;

            // Consistency check
            if (ctx->rx.twi_mtx.sent > ctx->rx.twi_mtx.cnt) goto Err;

            if (ctx->rx.twi_mtx.sent < ctx->rx.twi_mtx.cnt)
            {
                // Mark as short-packet
                ctx->rx.twi_mtx.short_packet = 1;
            }

            // Report the reception of the response packet
            ok = TWI_Rx_Master_Tx_Rsp(ctx);
            if (!ok) goto Err;

            // Response packet processed
            ctx->rx.state = STATE_RX_RSP;
        }
        else

        //
        // TWI master Rx response
        //

        if (ctx->rx.state == STATE_RX_TWI_MASTER_RX_CNT)
        {
            // Response packet bytes:
            // +00: s0______ b: Skipped, response code
            // +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
            // Skipped=0:
            //   +02: 000nnnnn b: Received bytes minus one (0..31, corresponding with 1..32)
            //   +03: <bytes>

            ctx->rx.twi_mrx.cnt  = (data_byte & 0x1F) + 1;
            ctx->rx.twi_mrx.rcvd = 0;

            // We can increment the free byte count here as the AxiCat has
            // removed the command from its buffer at this point.
            ctx->twi.hw_m_buf_free += (ctx->rx.twi_mrx.cnt + 3);

            // Consistency check
            if (ctx->twi.hw_m_buf_free > ctx->twi.hw_m_buf_len) goto Err;

            if (ctx->rx.twi_mrx.short_packet)
            {
                // Report the reception of the response packet
                ok = TWI_Rx_Master_Rx_Rsp(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            {
                ctx->rx.state = STATE_RX_TWI_MASTER_RX_RCVD;
            }
        }
        else
        if (ctx->rx.state == STATE_RX_TWI_MASTER_RX_RCVD)
        {
            // Bits 7-5 must be zero
            if (data_byte & 0xE0) goto Err;

            // Bits 4-0: received bytes minus one
            ctx->rx.twi_mrx.rcvd = (data_byte & 0x1F) + 1;

            // Consistency check
            if (ctx->rx.twi_mrx.rcvd > ctx->rx.twi_mrx.cnt) goto Err;

            if (ctx->rx.twi_mrx.rcvd < ctx->rx.twi_mrx.cnt)
            {
                // Mark as short-packet
                ctx->rx.twi_mrx.short_packet = 1;
            }

            // Report the reception of the response packet. Note that the data
            // bytes still have to come in.
            ok = TWI_Rx_Master_Rx_Rsp(ctx);
            if (!ok) goto Err;

            // Receive the data payload
            ctx->rx.state = STATE_RX_TWI_MASTER_RX_DATA;
        }
        else
        if (ctx->rx.state == STATE_RX_TWI_MASTER_RX_DATA)
        {
            ctx->rx.twi_mrx.rcvd--;

            ok = TWI_Rx_Master_Rx_Byte(ctx,data_byte);
            if (!ok) goto Err;

            if (ctx->rx.twi_mrx.rcvd == 0)
            {
                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // TWI slave Tx response
        //

        if (ctx->rx.state == STATE_RX_TWI_SLAVE_TX_CNT)
        {
            // Response packet bytes:
            // +00: sn______ b: Skipped, NACK response from master, response code
            // +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
            // Skipped=0:
            //   +02: 000nnnnn b: Transmitted bytes minus one (0..31, corresponding with 1..32)

            ctx->rx.twi_stx.cnt  = (data_byte & 0x1F) + 1;
            ctx->rx.twi_stx.sent = 0;

            // We can increment the free byte count here as the AxiCat has
            // removed the command from its buffer at this point.
            ctx->twi.hw_stx_buf_free += (ctx->rx.twi_stx.cnt + 2);

            // Consistency check
            if (ctx->twi.hw_stx_buf_free > ctx->twi.hw_stx_buf_len) goto Err;

            if (ctx->rx.twi_stx.short_packet)
            {
                // Report the reception of the response packet
                ok = TWI_Rx_Slave_Tx_Rsp(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            {
                ctx->rx.state = STATE_RX_TWI_SLAVE_TX_SENT;
            }
        }
        else
        if (ctx->rx.state == STATE_RX_TWI_SLAVE_TX_SENT)
        {
            // Bits 7-5 must be zero
            if (data_byte & 0xE0) goto Err;

            // Bits 4-0: sent bytes minus one
            ctx->rx.twi_stx.sent = (data_byte & 0x1F) + 1;

            // Consistency check
            if (ctx->rx.twi_stx.sent > ctx->rx.twi_stx.cnt) goto Err;

            if (ctx->rx.twi_stx.sent < ctx->rx.twi_stx.cnt)
            {
                // Mark as short-packet
                ctx->rx.twi_stx.short_packet = 1;
            }

            // Report the reception of the response packet
            ok = TWI_Rx_Slave_Tx_Rsp(ctx);
            if (!ok) goto Err;

            // Response packet processed
            ctx->rx.state = STATE_RX_RSP;
        }
        else

        //
        // TWI slave Rx response
        //

        if (ctx->rx.state == STATE_RX_TWI_SLAVE_RX_CNT)
        {
            // Response packet bytes:
            // +00: s0______ b: Skipped, response code
            // +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
            // Skipped=0:
            //   +02: 000nnnnn b: Received bytes minus one (0..31, corresponding with 1..32)
            //   +03: <bytes>

            ctx->rx.twi_srx.cnt  = (data_byte & 0x1F) + 1;
            ctx->rx.twi_srx.rcvd = 0;

            // We can increment the free byte count here as the AxiCat has
            // removed the command from its buffer at this point.
            ctx->twi.hw_srx_buf_free += (ctx->rx.twi_srx.cnt + 2);

            // Consistency check
            if (ctx->twi.hw_srx_buf_free > ctx->twi.hw_srx_buf_len) goto Err;

            if (ctx->rx.twi_srx.short_packet)
            {
                // Report the reception of the response packet
                ok = TWI_Rx_Slave_Rx_Rsp(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            {
                ctx->rx.state = STATE_RX_TWI_SLAVE_RX_RCVD;
            }
        }
        else
        if (ctx->rx.state == STATE_RX_TWI_SLAVE_RX_RCVD)
        {
            // Bits 7-5 must be zero
            if (data_byte & 0xE0) goto Err;

            // Bits 4-0: received bytes minus one
            ctx->rx.twi_srx.rcvd = (data_byte & 0x1F) + 1;

            // Consistency check
            if (ctx->rx.twi_srx.rcvd > ctx->rx.twi_srx.cnt) goto Err;

            if (ctx->rx.twi_srx.rcvd < ctx->rx.twi_srx.cnt)
            {
                // Mark as short-packet
                ctx->rx.twi_srx.short_packet = 1;
            }

            // Report the reception of the response packet. Note that the data
            // bytes still have to come in.
            ok = TWI_Rx_Slave_Rx_Rsp(ctx);
            if (!ok) goto Err;

            // Receive the data payload
            ctx->rx.state = STATE_RX_TWI_SLAVE_RX_DATA;
        }
        else
        if (ctx->rx.state == STATE_RX_TWI_SLAVE_RX_DATA)
        {
            ctx->rx.twi_srx.rcvd--;

            ok = TWI_Rx_Slave_Rx_Byte(ctx,data_byte);
            if (!ok) goto Err;

            if (ctx->rx.twi_srx.rcvd == 0)
            {
                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // SPI transfer response
        //

        if (ctx->rx.state == STATE_RX_SPI_XFR_CNT)
        {
            // Response packet bytes:
            // +00: s0______ b: Skipped, response code
            // +01: mssnnnnn b: Last packet
            //                  Slave select (0..3)
            //                  Bytes minus one (0..31, corresponding with 1..32)
            // Skipped=0:
            //   +02: 000nnnnn b: Received bytes minus one (0..31, corresponding with 1..32)
            //   +03: <bytes>

            ctx->rx.spi_xfr.cnt_mo = (data_byte & 0x1F);
            ctx->rx.spi_xfr.ss     = (data_byte >> 5) & 0x03;
            ctx->rx.spi_xfr.last   = (data_byte & 0x80) ? 1 : 0;

            ok = SPI_Rx_Chk_Rsp(ctx);
            if (!ok) goto Err;

            // We can increment the free byte count here as the AxiCat has
            // removed the command from its buffer at this point.
            ctx->spi.hw_buf_free += (ctx->rx.spi_xfr.cnt_mo + 1 + 2);

            // Consistency check
            if (ctx->spi.hw_buf_free > ctx->spi.hw_buf_len) goto Err;

            if (ctx->rx.spi_xfr.skipped)
            {
                ok = SPI_Rx_Skip(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            {
                ctx->rx.state = STATE_RX_SPI_XFR_RCVD;
            }
        }
        else
        if (ctx->rx.state == STATE_RX_SPI_XFR_RCVD)
        {
            // Bits 7-5 must be zero
            if (data_byte & 0xE0) goto Err;

            // Bits 4-0: received bytes minus one
            ctx->rx.spi_xfr.rcvd_mo = (data_byte & 0x1F);

            // Consistency check
            if (ctx->rx.spi_xfr.rcvd_mo > ctx->rx.spi_xfr.cnt_mo) goto Err;

            // Receive one or more data bytes
            ctx->rx.state = STATE_RX_SPI_XFR_DATA;
        }
        else
        if (ctx->rx.state == STATE_RX_SPI_XFR_DATA)
        {
            ok = SPI_Rx_Byte(ctx,data_byte);
            if (!ok) goto Err;

            if (ctx->rx.spi_xfr.rcvd_mo > 0)
            {
                ctx->rx.spi_xfr.rcvd_mo--;
            }
            else
            {
                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // 1-Wire touch bits response
        //

        if (ctx->rx.state == STATE_RX_OW_TOUCH_BITS_CNT)
        {
            // Response packet bytes:
            // +00: s0______ b: Skipped, response code
            // +01: s_______ b: Activate SPU y/n
            //                  Bits minus one (0..127, corresponding with 1..128)
            // Skipped=0:
            //   +02: 0_______ b: Received bytes minus one (0..31, corresponding with 1..32)
            //   +03: <bits>

            ctx->rx.ow_touch_bits.cnt_mo = (data_byte & 0x7F);
            ctx->rx.ow_touch_bits.spu    = (data_byte & 0x80) ? 1 : 0;

            ok = OW_Rx_Chk_Touch_Bits(ctx);
            if (!ok) goto Err;

            // We can increment the free byte count here as the AxiCat has
            // removed the command from its buffer at this point.
            ctx->ow.hw_buf_free += ((ctx->rx.ow_touch_bits.cnt_mo >> 3) + 1 + 3);

            // Consistency check
            if (ctx->ow.hw_buf_free > ctx->ow.hw_buf_len) goto Err;

            if (ctx->rx.ow_touch_bits.short_packet)
            {
                ok = OW_Rx_Skip_Touch_Bits(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
            else
            {
                ctx->rx.state = STATE_RX_OW_TOUCH_BITS_RCVD;
            }
        }
        else
        if (ctx->rx.state == STATE_RX_OW_TOUCH_BITS_RCVD)
        {
            // Bit 7 must be zero
            if (data_byte & 0x80) goto Err;

            // Bits 6-0: received bits minus one
            ctx->rx.ow_touch_bits.rcvd_mo = (data_byte & 0x7F);

            // Consistency check
            if (ctx->rx.ow_touch_bits.rcvd_mo > ctx->rx.ow_touch_bits.cnt_mo) goto Err;

            // Check for short-packet
            if (ctx->rx.ow_touch_bits.rcvd_mo < ctx->rx.ow_touch_bits.cnt_mo)
            {
                ctx->rx.ow_touch_bits.short_packet = 1;
            }

            // Receive one or more data bytes
            ctx->rx.state = STATE_RX_OW_TOUCH_BITS_DATA;
        }
        else
        if (ctx->rx.state == STATE_RX_OW_TOUCH_BITS_DATA)
        {
            ok = OW_Rx_Byte_Touch_Bits(ctx,data_byte);
            if (!ok) goto Err;

            if (ctx->rx.ow_touch_bits.rcvd_mo >= 8)
            {
                // This was not the last data byte of the response packet
                ctx->rx.ow_touch_bits.rcvd_mo -= 8;
            }
            else
            {
                // This was the last data byte of the response packet
                ok = OW_Rx_EOP_Touch_Bits(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // 1-Wire enumerate response
        //

        if (ctx->rx.state == STATE_RX_OW_ENUM_ROM_CODE)
        {
            ctx->rx.ow_enum.rom_code[ctx->rx.ow_enum.si] = data_byte;
            ctx->rx.ow_enum.si++;

            if (ctx->rx.ow_enum.si == 8)
            {
                ok = OW_Rx_Enum(ctx);
                if (!ok) goto Err;

                // Response packet processed
                ctx->rx.state = STATE_RX_RSP;
            }
        }
        else

        //
        // Unknown state - shouldn't end up here
        //

        {
            goto Err;
        }
    }

    return 1;


  Err:

    Mark_Detached(ctx);
    return 0;
}


static
FLAG  Step (CTX *ctx, FLAG *idle)
{
    U32     sch_cnt;


    if (Is_Detached(ctx)) goto Err;


    //
    // Reception of responses from the AxiCat
    //

    if (ctx->rx.enabled)
        if (!Step_Rx(ctx)) goto Err;


    //
    // Main procedure
    //

    if (!Step_Main(ctx)) goto Err;


    //
    // Transmission of commands to the AxiCat
    //

    // GPIO transfers
    if (!GPIO_Tx(ctx)) goto Err;

    // TWI transfers
    if (!TWI_Tx_Master(ctx)) goto Err;
    if (!TWI_Tx_Slave_Tx(ctx)) goto Err;
    if (!TWI_Tx_Slave_Rx(ctx)) goto Err;

    // SPI transfers
    if (!SPI_Tx(ctx)) goto Err;

    // 1-Wire transfers
    if (!OW_Tx(ctx)) goto Err;

    // Transmit UART commands as last, as these can choke the Tx FIFO in the
    // AxiCat
    if (!UART_Tx(ctx,0)) goto Err;
    if (!UART_Tx(ctx,1)) goto Err;

    // Flush any remaining Tx bytes to the AxiCat
    if (!Tx_Flush(ctx)) goto Err;


    if (idle)
    {
        sch_cnt = ctx->gpio.sch_xfr_cnt +
                  ctx->twi.sch_m_xfr_cnt +
                  ctx->twi.sch_stx_xfr_cnt +
                  ctx->twi.sch_srx_xfr_cnt +
                  ctx->spi.sch_xfr_cnt;

        (*idle) = (sch_cnt == 0) ? 1 : 0;
    }


    return 1;


  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_Step (AXICAT_AL_HANDLE handle, FLAG *idle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_Step (AXICAT_AL_HANDLE handle, FLAG *idle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return Step(ctx,idle);
}


/*-----------------------------------------------------------------------------
VOID  Destroy (CTX *ctx)
-----------------------------------------------------------------------------*/


static
VOID  Destroy (CTX *ctx)
{
    LDLL_ITEM  *item;
    TWI_XFR    *twi_xfr;
    SPI_XFR    *spi_xfr;
    OW_XFR     *ow_xfr;
    GPIO_XFR   *gpio_xfr;


    // Flush any pending data. The caller expect us to do this. The caller may
    // send some clean up commands like TWI DISABLE before destroying the
    // AxiCat AL context.

    Tx_Flush(ctx);


    // TWI transfers

    // [1] Drop all from list of scheduled transfers
    TWI_Complete_All_Xfrs(&ctx->twi);

    // [2] Drop all from list of assigned transfers
    while ((item = LDLLGetTailItem(&ctx->twi.assigned_xfr_list_base)) != 0)
    {
        twi_xfr = GetContAd(item,TWI_XFR,list_item);

        TWI_Xfr_Unassign(twi_xfr);
    }

    // [3] Uncache all => destroy transfers
    while ((item = LDLLGetTailItem(&ctx->twi.cached_xfr_list_base)) != 0)
    {
        twi_xfr = GetContAd(item,TWI_XFR,list_item);

        TWI_Xfr_Uncache(twi_xfr);
    }


    // SPI transfers

    // [1] Drop all from list of scheduled transfers
    SPI_Complete_All_Xfrs(&ctx->spi);

    // [2] Drop all from list of assigned transfers
    while ((item = LDLLGetTailItem(&ctx->spi.assigned_xfr_list_base)) != 0)
    {
        spi_xfr = GetContAd(item,SPI_XFR,list_item);

        SPI_Xfr_Unassign(spi_xfr);
    }

    // [3] Uncache all => destroy transfers
    while ((item = LDLLGetTailItem(&ctx->spi.cached_xfr_list_base)) != 0)
    {
        spi_xfr = GetContAd(item,SPI_XFR,list_item);

        SPI_Xfr_Uncache(spi_xfr);
    }


    // 1-Wire transfers

    // [1] Drop all from list of scheduled transfers
    OW_Complete_All_Xfrs(&ctx->ow);

    // [2] Drop all from list of assigned transfers
    while ((item = LDLLGetTailItem(&ctx->ow.assigned_xfr_list_base)) != 0)
    {
        ow_xfr = GetContAd(item,OW_XFR,list_item);

        OW_Xfr_Unassign(ow_xfr);
    }

    // [3] Uncache all => destroy transfers
    while ((item = LDLLGetTailItem(&ctx->ow.cached_xfr_list_base)) != 0)
    {
        ow_xfr = GetContAd(item,OW_XFR,list_item);

        OW_Xfr_Uncache(ow_xfr);
    }


    // GPIO transfers

    // [1] Drop all from list of scheduled transfers
    GPIO_Complete_All_Xfrs(&ctx->gpio);

    // [2] Drop all from list of assigned transfers
    while ((item = LDLLGetTailItem(&ctx->gpio.assigned_xfr_list_base)) != 0)
    {
        gpio_xfr = GetContAd(item,GPIO_XFR,list_item);

        GPIO_Xfr_Unassign(gpio_xfr);
    }

    // [3] Uncache all => destroy transfers
    while ((item = LDLLGetTailItem(&ctx->gpio.cached_xfr_list_base)) != 0)
    {
        gpio_xfr = GetContAd(item,GPIO_XFR,list_item);

        GPIO_Xfr_Uncache(gpio_xfr);
    }


    AXICAT_AL_Free(ctx->uarts[0].tx_buf);
    AXICAT_AL_Free(ctx->uarts[0].rx_buf);
    AXICAT_AL_Free(ctx->uarts[1].tx_buf);
    AXICAT_AL_Free(ctx->uarts[1].rx_buf);
    AXICAT_AL_Free(ctx->serial.tx_buf);
    AXICAT_AL_Free(ctx);
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
AXICAT_AL_CONN_STATE
DynllCall  AXICAT_AL_Get_Conn_State (AXICAT_AL_HANDLE handle)

Get the connection state of the AL context.
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
AXICAT_AL_CONN_STATE
DynllCall  AXICAT_AL_Get_Conn_State (AXICAT_AL_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return ctx->conn_state;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_Destroy (AXICAT_AL_HANDLE handle)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_Destroy (AXICAT_AL_HANDLE handle)
{
    CTX        *ctx;

    if (!handle) return;

    // Init
    ctx = (CTX*)handle;

    Destroy(ctx);
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
AXICAT_AL_HANDLE  DynllCall  AXICAT_AL_Create (SERIAL_INTF *serial_intf)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
AXICAT_AL_HANDLE  DynllCall  AXICAT_AL_Create (SERIAL_INTF *serial_intf)
{
    CTX    *ctx;


    // Allocate a cleared context
    ctx = AXICAT_AL_Alloc_Clear(sizeof(CTX));
    if (!ctx) return 0;


    // Serial I/O interface

    ctx->serial.serial_intf = serial_intf;

    ctx->serial.tx_buf_len = 1024;
    ctx->serial.tx_buf     = AXICAT_AL_Alloc(ctx->serial.tx_buf_len);
    if (!ctx->serial.tx_buf) goto Err;


    // AxiCat

    ctx->uarts[0].tx_buf_len = 4096;
    ctx->uarts[0].rx_buf_len = 4096;
    ctx->uarts[1].tx_buf_len = 4096;
    ctx->uarts[1].rx_buf_len = 4096;

    ctx->uarts[0].tx_buf = AXICAT_AL_Alloc(ctx->uarts[0].tx_buf_len);
    if (!ctx->uarts[0].tx_buf) goto Err;

    ctx->uarts[0].rx_buf = AXICAT_AL_Alloc(ctx->uarts[0].rx_buf_len);
    if (!ctx->uarts[0].rx_buf) goto Err;

    ctx->uarts[1].tx_buf = AXICAT_AL_Alloc(ctx->uarts[1].tx_buf_len);
    if (!ctx->uarts[1].tx_buf) goto Err;

    ctx->uarts[1].rx_buf = AXICAT_AL_Alloc(ctx->uarts[1].rx_buf_len);
    if (!ctx->uarts[1].rx_buf) goto Err;


    ctx->conn_state = AXICAT_AL_CONN_STATE_CONNECTING;


    // Return successfully
    return (AXICAT_AL_HANDLE)ctx;

  Err:

    Destroy(ctx);

    // Report failure
    return 0;
}
