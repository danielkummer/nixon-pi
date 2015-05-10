
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat.h
//
// AxiCat serial I/O protocol definition.
//
// Language: AVR-GCC4 gnu99, GCC4 gnu89, MSVC60
//
// History:
//
//   2014-02-14  Peter S'heeren, Axiris
//
//      * Created.
//
//   2014-07-07  Peter S'heeren, Axiris
//
//      * Released.
//
//   2014-09-01  Peter S'heeren, Axiris
//
//      * Added AXICAT_GEN_BUF_INFO structure.
//      * Released.
//
//   2014-09-17  Peter S'heeren, Axiris
//
//      * Added special function 1-Wire.
//      * Added CMD_GEN_INFO.
//      * Removed BUF_GEN_INFO from protocol. This command code may be reused
//        for implementing another command in the future.
//      * Released v1.1.0.
//
//   2014-09-28  Peter S'heeren, Axiris
//
//      * Added 1-Wire enumeration.
//      * Reassigned command codes of debugging commands.
//      * Released v1.2.0.
//
//   2014-12-12  Peter S'heeren, Axiris
//
//      * Added 1-Wire probe command.
//      * Added macros of firmware versions.
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


#ifndef __AXICAT_H__
#define __AXICAT_H__


#include "platform.h"


//
// General
//


// No-operation command.
//
// This command isn't actually processed in the AxiCat. It's a reserved command
// code that won't be assigned a functional command, ever.
//
// This command is used for flushing the AxiCat's Tx channel in a controlled
// manner.

// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_GEN_NOP                      0


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_GEN_VERSION                  2


// Response packet bytes:
// +00: 00______ b: Response code
// +01: Zero-terminated UTF-8 string

#define AXICAT_RSP_GEN_VERSION                  2


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_GEN_INFO                     3


// Firmware versions

typedef U8      AXICAT_FW_VERSION;

#define AXICAT_FW_VERSION_1_2_0                 0
#define AXICAT_FW_VERSION_1_3_0                 1


// Response packet bytes:
// +00: 00______ b: Response code
// +01: 00010000 b: Length; includes all bytes except for the response code
// +02: nnnnnnnn b: Version (AXICAT_FW_VERSION_Xxx)
// +03: nnnnnnnn b: LSB - UART Tx buffer length minus one
// +04: nnnnnnnn b: MSB /
// +05: nnnnnnnn b: LSB - UART Rx buffer length minus one
// +06: nnnnnnnn b: MSB /
// +07: nnnnnnnn b: LSB - TWI master command buffer length minus one
// +08: nnnnnnnn b: MSB /
// +09: nnnnnnnn b: LSB - TWI slave Tx buffer length minus one
// +10: nnnnnnnn b: MSB /
// +11: nnnnnnnn b: LSB - TWI slave Rx buffer length minus one
// +12: nnnnnnnn b: MSB /
// +13: nnnnnnnn b: LSB - SPI transfer buffer length minus one
// +14: nnnnnnnn b: MSB /
// +15: nnnnnnnn b: LSB - 1-Wire command buffer length minus one
// +16: nnnnnnnn b: MSB /

#define AXICAT_RSP_GEN_INFO                     3


typedef struct  _AXICAT_GEN_INFO
{
    U8                  len;
    AXICAT_FW_VERSION   version;
    U8                  uart_tx_len_lsb;
    U8                  uart_tx_len_msb;
    U8                  uart_rx_len_lsb;
    U8                  uart_rx_len_msb;
    U8                  twi_m_len_lsb;
    U8                  twi_m_len_msb;
    U8                  twi_stx_len_lsb;
    U8                  twi_stx_len_msb;
    U8                  twi_srx_len_lsb;
    U8                  twi_srx_len_msb;
    U8                  spi_len_lsb;
    U8                  spi_len_msb;
    U8                  ow_len_lsb;
    U8                  ow_len_msb;
}
    AXICAT_GEN_INFO;


//
// GPIO - general-purpose I/O pins
//


// Command packet bytes:
// +00: ________ b: Command code
// +01: d00nnnnn b: Pin number n (0..16), d=0 for input, d=1 for output

#define AXICAT_CMD_GPIO_SET_DIR                 4


// Command packet bytes:
// +00: ________ b: Command code
// +01: s00nnnnn b: Pin number n (0..16), output state s

#define AXICAT_CMD_GPIO_WRITE                   5


// Command packet bytes:
// +00: ________ b: Command code
// +01: 000nnnnn b: Pin number n (0..16)

#define AXICAT_CMD_GPIO_READ                    6


// Response packet bytes:
// +00: 00______ b: Response code
// +01: sodnnnnn b:
//      Bit    7: Sensed input state
//      Bit    6: Output state
//      Bit    5: Direction is input (0) or output (1)
//      Bit 4..0: Pin number (0..16)

#define AXICAT_RSP_GPIO_READ                    6


//
// UARTs
//


// Data bits

#define AXICAT_UART_DATA_BITS_5                 0
#define AXICAT_UART_DATA_BITS_6                 1
#define AXICAT_UART_DATA_BITS_7                 2
#define AXICAT_UART_DATA_BITS_8                 3
#define AXICAT_UART_DATA_BITS_9                 4


// Stop bits

#define AXICAT_UART_STOP_BITS_1                 0
#define AXICAT_UART_STOP_BITS_2                 1


// Baudrate

#define AXICAT_UART_BAUDRATE_1200               0
#define AXICAT_UART_BAUDRATE_2400               1
#define AXICAT_UART_BAUDRATE_4800               2
#define AXICAT_UART_BAUDRATE_9600               3
#define AXICAT_UART_BAUDRATE_19200              4
#define AXICAT_UART_BAUDRATE_38400              5
#define AXICAT_UART_BAUDRATE_57600              6
#define AXICAT_UART_BAUDRATE_115200             7


//
// UART0
//


// Command packet bytes:
// +00: ________ b: Command code
// +01: 00000nnn b: Data bits (AXICAT_UART_DATA_BITS_Xxx)

#define AXICAT_CMD_UART0_SET_DATA_BITS          8


// Command packet bytes:
// +00: ________ b: Command code
// +01: 0000000n b: Stop bits (AXICAT_UART_STOP_BITS_Xxx)

#define AXICAT_CMD_UART0_SET_STOP_BITS         10


// Command packet bytes:
// +00: ________ b: Command code
// +01: 00000nnn b: Baudrate (AXICAT_UART_BAUDRATE_Xxx)

#define AXICAT_CMD_UART0_SET_BAUDRATE          12


// Command packet bytes:
// +00: ________ b: Command code
// +01: nnnnnnnn b: UBRR[7..0]
// +02: x000nnnn b: Double speed y/n, UBRR[11..8]

#define AXICAT_CMD_UART0_SET_BAUDRATE_RAW      14


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_UART0_ENABLE                16


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_UART0_DISABLE               18


// Command packet bytes:
// +00: ________ b: Command code
// +01: nnnnnnnn b: Rx timeout (0..255 ms)

#define AXICAT_CMD_UART0_SET_RX_TIMEOUT        20


// Command packet bytes:
// +00: ________ b: Command code
// +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
// +02: <bytes>
//      The number of words depends on the data bits setting:
//      * 5..8 bits: 1 byte per word.
//      * 9 bits: 2 bytes (LSB->MSB) per word. When an odd number of bytes is
//        sent, the last byte will be discarded.

#define AXICAT_CMD_UART0_TX                    22


// Response packet bytes:
// +00: 00______ b: Response code
// +01: nnnnnnnn b: LSB - Free bytes minus one
// +02: nnnnnnnn b: MSB /

#define AXICAT_RSP_UART0_TX_FREE               20


// Response packet bytes:
// +00: 00______ b: Response code
// +01: nnnnnnnn b: Bytes minus one (0..255, corresponding with 1..256)
// +02: <bytes>
//      The number of words depends on the data bits setting:
//      * 5..8 bits: 1 byte per word.
//      * 9 bits: 2 bytes (LSB->MSB) per word.

#define AXICAT_RSP_UART0_RX                    22


//
// UART1
//


// All UART1 command or response codes have a fixed offset of one from their
// UART0 counterparts.

#define AXICAT_CMD_UART1_SET_DATA_BITS          9

#define AXICAT_CMD_UART1_SET_STOP_BITS         11

#define AXICAT_CMD_UART1_SET_BAUDRATE          13

#define AXICAT_CMD_UART1_SET_BAUDRATE_RAW      15

#define AXICAT_CMD_UART1_ENABLE                17

#define AXICAT_CMD_UART1_DISABLE               19

#define AXICAT_CMD_UART1_SET_RX_TIMEOUT        21

#define AXICAT_CMD_UART1_TX                    23

#define AXICAT_RSP_UART1_TX_FREE               21

#define AXICAT_RSP_UART1_RX                    23


//
// TWI
//


// Set of bus speeds

#define AXICAT_TWI_SPEED_50000                  0
#define AXICAT_TWI_SPEED_100000                 1
#define AXICAT_TWI_SPEED_200000                 2
#define AXICAT_TWI_SPEED_400000                 3


// Command packet bytes:
// +00: ________ b: Command code
// +01: 000000nn b: Bus speed (AXICAT_TWI_SPEED_Xxx)

#define AXICAT_CMD_TWI_SET_SPEED               32


// Command packet bytes:
// +00: ________ b: Command code
// +01: nnnnnnnn b: TWBR register bits 7-0
// +02: 000000mm b: TWPS[1..0]

#define AXICAT_CMD_TWI_SET_SPEED_RAW           33


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_TWI_ENABLE                  34


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_TWI_DISABLE                 35


// Command packet bytes:
// +00: ________ b: Command code
// +01: nnnnnnnd b: Slave address, write(0)/read(1)

#define AXICAT_CMD_TWI_MASTER_START            36


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_TWI_MASTER_STOP             37


// Command packet bytes:
// +00: ________ b: Command code
// +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
// +02: <bytes>

#define AXICAT_CMD_TWI_MASTER_TX               38


// Command packet bytes:
// +00: ________ b: Command code
// +01: m00nnnnn b: Last packet, bytes minus one (0..31, corresponding with 1..32)

#define AXICAT_CMD_TWI_MASTER_RX               39


// Command packet bytes:
// +00: ________ b: Command code
// +01: nnnnnnng b: Slave address, respond to general call address

#define AXICAT_CMD_TWI_SLAVE_ENABLE            40


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_TWI_SLAVE_DISABLE           41


// Command packet bytes:
// +00: ________ b: Command code
// +01: m00nnnnn b: Last packet, bytes minus one (0..31, corresponding with 1..32)
// +02: <bytes>

#define AXICAT_CMD_TWI_SLAVE_TX                42


// Command packet bytes:
// +00: ________ b: Command code
// +01: m00nnnnn b: Last packet, bytes minus one (0..31, corresponding with 1..32)

#define AXICAT_CMD_TWI_SLAVE_RX                43


// Command packet bytes:
// +00: ________ b: Command code
// +01: 000000__ b:
//      Bit 7..2: Zeroes
//      Bit    1: Clear slave Rx buffer, clear skip past last flag
//      Bit    0: Clear slave Tx buffer, clear skip past last flag

#define AXICAT_CMD_TWI_CLEAR                   44


// Response packet bytes:
// +00: 00______ b: Response code

#define AXICAT_RSP_TWI_BUS_ERROR               34


// Response packet bytes:
// +00: 00______ b: Response code

#define AXICAT_RSP_TWI_ARB_LOST                35


// Response packet bytes:
// +00: sn______ b: Skipped, NACK response, response code
// +01: nnnnnnnd b: Slave address, write(0)/read(1)

#define AXICAT_RSP_TWI_MASTER_START            36


// Response packet bytes:
// +00: s0______ b: Skipped, response code

#define AXICAT_RSP_TWI_MASTER_STOP             37


// Response packet bytes:
// +00: sn______ b: Skipped, NACK response from slave, response code
// +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
// Skipped=0:
//   +02: 000nnnnn b: Sent bytes minus one (0..31, corresponding with 1..32)

#define AXICAT_RSP_TWI_MASTER_TX               38


// Response packet bytes:
// +00: sn______ b: Skipped, NACK response from master, response code
// +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
// Skipped=0:
//   +02: 000nnnnn b: Received bytes minus one (0..31, corresponding with 1..32)
//   +03: <bytes>

#define AXICAT_RSP_TWI_MASTER_RX               39


// Response packet bytes:
// +00: sn______ b: Skipped, NACK response from master, response code
// +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
// Skipped=0:
//   +02: 000nnnnn b: Sent bytes minus one (0..31, corresponding with 1..32)

#define AXICAT_RSP_TWI_SLAVE_TX                42


// Response packet bytes:
// +00: s0______ b: Skipped, response code
// +01: 000nnnnn b: Bytes minus one (0..31, corresponding with 1..32)
// Skipped=0:
//   +02: 000nnnnn b: Received bytes minus one (0..31, corresponding with 1..32)
//   +03: <bytes>

#define AXICAT_RSP_TWI_SLAVE_RX                43


//
// SPI (master)
//


// Set of bus speeds

#define AXICAT_SPI_SPEED_750000                 0
#define AXICAT_SPI_SPEED_1500000                1
#define AXICAT_SPI_SPEED_3000000                2
#define AXICAT_SPI_SPEED_6000000                3


// Command packet bytes:
// +00: ________ b: Command code
// +01: 000000nn b: Bus speed (AXICAT_SPI_SPEED_Xxx)

#define AXICAT_CMD_SPI_SET_SPEED               48


// Command packet bytes:
// +00: ________ b: Command code
// +01: 00000xnn b: Double speed, clock rate select

#define AXICAT_CMD_SPI_SET_SPEED_RAW           49


// Command packet bytes:
// +00: ________ b: Command code
// +01: 00000___ b:
//      Bit 7..3: Zeroes
//      Bit    2: Bit order msb->lsb (0) or lsb->msb (1)
//      Bit    1: Clock phase, sample on leading (0) or trailing (1) edge
//      Bit    0: Clock polarity, SCK is low (0) or high (1) when idle

#define AXICAT_CMD_SPI_SET_CFG                 50


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_SPI_ENABLE                  51


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_SPI_DISABLE                 52


// Command packet bytes:
// +00: ________ b: Command code
// +01: mssnnnnn b: Last packet, slave select (0..3), bytes minus one (0..31, corresponding with 1..32)
// +02: <bytes>

#define AXICAT_CMD_SPI_XFR                     53


// Response packet bytes:
// +00: s0______ b: Skipped, response code
// +01: mssnnnnn b: Last packet, slave select (0..3), bytes minus one (0..31, corresponding with 1..32)
// Skipped=0:
//   +02: 000nnnnn b: Received bytes minus one (0..31, corresponding with 1..32)
//   +03: <bytes>

#define AXICAT_RSP_SPI_XFR                     53


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_OW_ENABLE                   56


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_OW_DISABLE                  57


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_OW_RESET                    58


// Command packet bytes:
// +00: ________ b: Command code
// +01: s_______ b: Activate SPU y/n, bits minus one (0..127, corresponding with 1..128)
// +02: <bits>

#define AXICAT_CMD_OW_TOUCH_BITS               59


// Command packet bytes:
// +00: ________ b: Command code
// +01: 000____0 b:
//      Bit 7..5: Zeroes
//      Bit    4: DS2409 MAIN (0) or AUX (1)
//      Bit    3: DS2409 smart ON y/n (1/0)
//      Bit    2: Enumerate family code y/n (1/0)
//      Bit    1: Search all (0) or search alarmed (1)
//      Bit    0: Start enumeration (0)
// +01: 00000001 b:
//      Bit 7..1: Zeroes
//      Bit    0: Continue enumeration (1)
// Enumerate family code enabled:
//   +02: ________ b: Family code
// DS2409 smart ON enabled:
//   +nn: DS2409 ROM code (8 bytes)

#define AXICAT_CMD_OW_ENUM                     60


// Command packet bytes:
// +00: ________ b: Command code
// +01: ROM code (8 bytes)

#define AXICAT_CMD_OW_PROBE                    61


// Response packet bytes:
// +00: sp______ b: Skipped, presence, response code

#define AXICAT_RSP_OW_RESET                    58


// Response packet bytes:
// +00: s0______ b: Skipped, response code
// +01: s_______ b: Activate SPU y/n, bits minus one (0..127, corresponding with 1..128)
// Skipped=0:
//   +02: 0_______ b: Received bytes minus one (0..31, corresponding with 1..32)
//   +03: <bits>

#define AXICAT_RSP_OW_TOUCH_BITS               59


// Response packet bytes:
// +00: sf______ b: Skipped, found, response code
// Skipped=0:
//   Found=0:
//     +01: ROM code (8 bytes)

#define AXICAT_RSP_OW_ENUM                     60


// Response packet bytes:
// +00: sf______ b: Skipped, found, response code

#define AXICAT_RSP_OW_PROBE                    61


// Debugging commands. These are only available in the debug build of the
// software.


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_TWI_DUMP                   192


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_SPI_DUMP                   193


// Command packet bytes:
// +00: ________ b: Command code

#define AXICAT_CMD_OW_DUMP                    194


#endif  // __AXICAT_H__
