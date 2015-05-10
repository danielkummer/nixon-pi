
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al_uart.c
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
//      * Added AXICAT_AL_UART_Set_Baudrate_Raw().
//      * Added AXICAT_AL_UART_Set_Rx_Timeout().
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
VOID  UART_Reset_Bufs (CTX *ctx, UART_CTX *uc)

This function has a corresponding function in the AxiCat firmware. Here it's
invoked at the same code points as in the firmware.
-----------------------------------------------------------------------------*/


static
VOID  UART_Reset_Bufs (CTX *ctx, UART_CTX *uc)
{
    uc->tx_sc = 0;
    uc->tx_si = 0;
    uc->tx_fi = 0;

    uc->rx_sc = 0;
    uc->rx_si = 0;
    uc->rx_fi = 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Enable (AXICAT_AL_HANDLE handle, U8 uart)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Enable (AXICAT_AL_HANDLE handle, U8 uart)
{
    CTX    *ctx;
    U8      cmd;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (uart > 1) goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Command code
    cmd = (uart == 0) ? AXICAT_CMD_UART0_ENABLE : AXICAT_CMD_UART1_ENABLE;

    // Send command packet
    if (!Tx_Byte(ctx,cmd)) goto Err;

    // Note: ctx->uarts[uart].hw_tx_free will be set to non-zero when the Rx
    // step function receives the free buffer space response triggered by the
    // UARTx ENABLE command.

    UART_Reset_Bufs(ctx,&ctx->uarts[uart]);

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Disable (AXICAT_AL_HANDLE handle, U8 uart)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Disable (AXICAT_AL_HANDLE handle, U8 uart)
{
    CTX    *ctx;
    U8      cmd;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (uart > 1) goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Command code
    cmd = (uart == 0) ? AXICAT_CMD_UART0_DISABLE : AXICAT_CMD_UART1_DISABLE;

    // Send command packet
    if (!Tx_Byte(ctx,cmd)) goto Err;

    // Don't send any data to the AxiCat when the UART is disabled
    ctx->uarts[uart].hw_tx_free = 0;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Baudrate
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U32                 baudrate
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Baudrate
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U32                 baudrate
)
{
    CTX    *ctx;
    U8      cmd;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (uart > 1) goto Err;

    if (baudrate == 1200)   u = AXICAT_UART_BAUDRATE_1200; else
    if (baudrate == 2400)   u = AXICAT_UART_BAUDRATE_2400; else
    if (baudrate == 4800)   u = AXICAT_UART_BAUDRATE_4800; else
    if (baudrate == 9600)   u = AXICAT_UART_BAUDRATE_9600; else
    if (baudrate == 19200)  u = AXICAT_UART_BAUDRATE_19200; else
    if (baudrate == 38400)  u = AXICAT_UART_BAUDRATE_38400; else
    if (baudrate == 57600)  u = AXICAT_UART_BAUDRATE_57600; else
    if (baudrate == 115200) u = AXICAT_UART_BAUDRATE_115200; else
        goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Command code
    cmd = (uart == 0) ? AXICAT_CMD_UART0_SET_BAUDRATE : AXICAT_CMD_UART1_SET_BAUDRATE;

    // Send command packet
    if (!Tx_Byte(ctx,cmd)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Baudrate_Raw
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U16                 ubrr,
    U8                  x2
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Baudrate_Raw
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U16                 ubrr,
    U8                  x2
)
{
    CTX    *ctx;
    U8      cmd;
    U8      u1;
    U8      u2;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (ubrr > 0xFFF) goto Err;
    if (x2 > 1) goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Command code and data
    cmd = (uart == 0) ? AXICAT_CMD_UART0_SET_BAUDRATE_RAW : AXICAT_CMD_UART1_SET_BAUDRATE_RAW;
    u1  = ubrr & 0xFF;
    u2  = (ubrr >> 8) | (x2 << 7);

    // Send command packet
    if (!Tx_Byte(ctx,cmd)) goto Err;
    if (!Tx_Byte(ctx,u1)) goto Err;
    if (!Tx_Byte(ctx,u2)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Data_Bits
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                  data_bits
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Data_Bits
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                  data_bits
)
{
    CTX        *ctx;
    UART_CTX   *uc;
    U8          cmd;
    U8          u;

    // Check parameters
    if (uart > 1) goto Err;

    // Init
    ctx = (CTX*)handle;
    uc  = &ctx->uarts[uart];

    if (data_bits == 5) u = AXICAT_UART_DATA_BITS_5; else
    if (data_bits == 6) u = AXICAT_UART_DATA_BITS_6; else
    if (data_bits == 7) u = AXICAT_UART_DATA_BITS_7; else
    if (data_bits == 8) u = AXICAT_UART_DATA_BITS_8; else
    if (data_bits == 9) u = AXICAT_UART_DATA_BITS_9; else
        goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Command code
    cmd = (uart == 0) ? AXICAT_CMD_UART0_SET_DATA_BITS : AXICAT_CMD_UART1_SET_DATA_BITS;

    // Send command packet
    if (!Tx_Byte(ctx,cmd)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    uc->wide = (data_bits > 8) ? 1 : 0;

    UART_Reset_Bufs(ctx,uc);

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Stop_Bits
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                  stop_bits
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Stop_Bits
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                  stop_bits
)
{
    CTX    *ctx;
    U8      cmd;
    U8      u;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (uart > 1) goto Err;

    if (stop_bits == 1) u = AXICAT_UART_STOP_BITS_1; else
    if (stop_bits == 2) u = AXICAT_UART_STOP_BITS_2; else
        goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Command code
    cmd = (uart == 0) ? AXICAT_CMD_UART0_SET_STOP_BITS : AXICAT_CMD_UART1_SET_STOP_BITS;

    // Send command packet
    if (!Tx_Byte(ctx,cmd)) goto Err;
    if (!Tx_Byte(ctx,u)) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Rx_Timeout
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U16                 timeout
)
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Rx_Timeout
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U16                 timeout
)
{
    CTX    *ctx;
    U8      cmd;

    // Init
    ctx = (CTX*)handle;

    // Check parameters
    if (uart > 1) goto Err;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) goto Err;

    // Command code
    cmd = (uart == 0) ? AXICAT_CMD_UART0_SET_RX_TIMEOUT : AXICAT_CMD_UART1_SET_RX_TIMEOUT;

    // Send command packet
    if (!Tx_Byte(ctx,cmd)) goto Err;
    if (!Tx_Byte(ctx,(U8)(timeout & 0xFF))) goto Err;
    if (!Tx_Byte(ctx,(U8)(timeout >> 8))) goto Err;

    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Write
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                 *buf,
    U32                 len,
    U32                *xfrd
)

Write data bytes to the Tx buffer. The step function will transfer the data to
the AxiCat.

In case wide data words are set, the function expects an even number of bytes.
The function round down the given length to an even number whatsoever.

When more data bytes are written than the Tx buffer can hold, the function
won't copy all bytes to the Tx buffer. The returned xfrd field indicate the
number of transferred bytes.
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Write
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                 *buf,
    U32                 len,
    U32                *xfrd
)
{
    CTX        *ctx;
    UART_CTX   *uc;
    U32         cnt;

    // Check parameters
    if (uart > 1) return 0;

    // Init
    ctx = (CTX*)handle;
    uc  = &ctx->uarts[uart];
    cnt = 0;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) return 0;

    // Restrict to an even number of bytes if the UART is set to wide words
    if (uc->wide) len &= ~1;

    for (;;)
    {
        if (uc->tx_sc == uc->tx_buf_len) break;
        if (len == 0) break;

        uc->tx_buf[uc->tx_si] = (*buf);
        buf++;
        len--;
        uc->tx_si = (uc->tx_si + 1) % uc->tx_buf_len;
        uc->tx_sc++;
        cnt++;
    }

    if (xfrd) (*xfrd) = cnt;
    return 1;
}


/*-----------------------------------------------------------------------------
AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Read
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                 *buf,
    U32                 len,
    U32                *xfrd
)

Read data bytes from the Rx buffer. The step function read data bytes from the
AxiCat into the Rx buffer.
-----------------------------------------------------------------------------*/


AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Read
(
    AXICAT_AL_HANDLE    handle,
    U8                  uart,
    U8                 *buf,
    U32                 len,
    U32                *xfrd
)
{
    CTX        *ctx;
    UART_CTX   *uc;
    U32         cnt;

    // Check parameters
    if (uart > 1) return 0;

    // Init
    ctx = (CTX*)handle;
    uc  = &ctx->uarts[uart];
    cnt = 0;

    // The context must be in the connected state
    if (ctx->conn_state != AXICAT_AL_CONN_STATE_CONNECTED) return 0;

    for (;;)
    {
        if (uc->rx_sc == 0) break;
        if (len == 0) break;

        (*buf) = uc->rx_buf[uc->rx_fi];
        buf++;
        len--;
        uc->rx_fi = (uc->rx_fi + 1) % uc->rx_buf_len;
        uc->rx_sc--;
        cnt++;
    }

    if (xfrd) (*xfrd) = cnt;
    return 1;
}


/*-----------------------------------------------------------------------------
FLAG  UART_Tx (CTX *ctx, U8 uart)

Try to transmit a UART Tx packet to the AxiCat.
-----------------------------------------------------------------------------*/


static
FLAG  UART_Tx (CTX *ctx, U8 uart)
{
    UART_CTX   *uc;
    U32         cnt;
    U8          cnt_mo;
    U8          cmd;
    U8          u;

    // Init
    uc = &ctx->uarts[uart];

    // Number of bytes to transmit
    cnt = uc->hw_tx_free < uc->tx_sc ? uc->hw_tx_free : uc->tx_sc;
    if (cnt == 0) return 1;


    // Send command packet

    cmd    = (uart == 0) ? AXICAT_CMD_UART0_TX : AXICAT_CMD_UART1_TX;
    cnt_mo = (U8)cnt - 1;

    if (!Tx_Byte(ctx,cmd)) goto Err;
    if (!Tx_Byte(ctx,cnt_mo)) goto Err;

    for (;;)
    {
        u = uc->tx_buf[uc->tx_fi];
        if (!Tx_Byte(ctx,u)) goto Err;

        uc->tx_fi = (uc->tx_fi + 1) % uc->tx_buf_len;
        uc->tx_sc--;

        uc->hw_tx_free--;

        cnt--;
        if (cnt == 0) break;
    }


    return 1;

  Err:

    return 0;
}


/*-----------------------------------------------------------------------------
VOID  UART_Rx_Byte (CTX *ctx, U8 uart, U8 data_byte)

Process a data byte from an incoming Rx packet.
-----------------------------------------------------------------------------*/


static
VOID  UART_Rx_Byte (CTX *ctx, U8 uart, U8 data_byte)
{
    UART_CTX   *uc;

    // Init
    uc = &ctx->uarts[uart];

    // Always store the data byte in the Rx buffer, whether or not an overflow
    // condition is true
    uc->rx_buf[uc->rx_si] = data_byte;

    uc->rx_si = (uc->rx_si + 1) % uc->rx_buf_len;

    if (uc->rx_sc < uc->rx_buf_len)
    {
        uc->rx_sc++;
    }
    else
    {
        // Rx buffer overflow, skip oldest data byte
        uc->rx_fi = (uc->rx_fi + 1) % uc->rx_buf_len;
    }
}
