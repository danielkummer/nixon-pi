
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al.h
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
//      * Added 1-Wire.
//      * Released v1.1.0.
//
//   2014-09-28  Peter S'heeren, Axiris
//
//      * Added 1-Wire enumeration.
//      * AXICAT_AL_OW_XFR: moved buffer pointer outside struct/union tree.
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


#ifndef __AXICAT_AL_H__
#define __AXICAT_AL_H__


#ifdef  __cplusplus

extern "C"
{

#endif


#include "serial.h"
#include "axicat.h"


// Dynamic link library


#ifdef  AXICAT_AL_DYNLL


#ifdef  AXICAT_AL_EXPORTS

#define AXICAT_AL_EXPORT        DynllExport

#else

#define AXICAT_AL_EXPORT        DynllImport

#endif


#else   // AXICAT_AL_DYNLL


// We're not building a dynamic link library

#define AXICAT_AL_EXPORT


#endif  // AXICAT_AL_DYNLL


// Forward declarations

typedef struct  _AXICAT_AL_GPIO_XFR     AXICAT_AL_GPIO_XFR;
typedef struct  _AXICAT_AL_TWI_XFR      AXICAT_AL_TWI_XFR;
typedef struct  _AXICAT_AL_SPI_XFR      AXICAT_AL_SPI_XFR;
typedef struct  _AXICAT_AL_OW_XFR       AXICAT_AL_OW_XFR;


// Handles

typedef POINTER     AXICAT_AL_HANDLE;


// Connection state

typedef U8          AXICAT_AL_CONN_STATE;

#define AXICAT_AL_CONN_STATE_DISCONNECTED   0
#define AXICAT_AL_CONN_STATE_CONNECTING     1
#define AXICAT_AL_CONN_STATE_CONNECTED      2


// GPIO pin direction

#define AXICAT_AL_GPIO_DIR_INPUT        0   // Input direction
#define AXICAT_AL_GPIO_DIR_OUTPUT       1   // Output direction


// GPIO transfer status

#define AXICAT_AL_GPIO_ST_SUCCESS       0   // Completed: success
#define AXICAT_AL_GPIO_ST_CANCELLED     1   // Completed: cancelled
#define AXICAT_AL_GPIO_ST_SCHEDULED    15   // Scheduled (must be non-zero)


// Transfer for reading the state of a GPIO.
//
// This structure is the public part of a larger structure allocated and owned
// by the AL context. You mustn't allocate the structure yourself; use the
// corresponding Create() and Destroy() functions instead.

struct  _AXICAT_AL_GPIO_XFR
{
    U8              pin;                // [IN]  GPIO pin (0..16)
    FLAG            input_state;        // [OUT] Sensed input state
    FLAG            output_state;       // [OUT] Output state
    FLAG            dir;                // [OUT] Direction (AXICAT_AL_GPIO_DIR_Xxx constant)

    volatile
    U8              status;             // Transfer status (AXICAT_AL_GPIO_ST_Xxx)
};


// TWI data transfer direction

#define AXICAT_AL_TWI_DIR_WRITE         0   // Master transfer:   master -> slave
                                            // Slave Tx transfer: slave  -> master
#define AXICAT_AL_TWI_DIR_READ          1   // Master transfer:   slave  -> master
                                            // Slave Rx transfer: master -> slave


// TWI transfer status

#define AXICAT_AL_TWI_ST_SUCCESS        0   // Completed: success
#define AXICAT_AL_TWI_ST_CANCELLED      1   // Completed: cancelled
#define AXICAT_AL_TWI_ST_BUS_ERROR      2   // Completed: bus error
#define AXICAT_AL_TWI_ST_ARB_LOST       3   // Completed: arbitration lost (master transfer only)
#define AXICAT_AL_TWI_ST_SKIPPED        4   // Completed: skipped
#define AXICAT_AL_TWI_ST_SCHEDULED     15   // Scheduled (must be non-zero)


// TWI transfer. Used for master transfers, slave Tx transfers and slave Rx
// transfers.
//
// This structure is the public part of a larger structure allocated and owned
// by the AL context. You mustn't allocate the structure yourself; use the
// corresponding Create() and Destroy() functions instead.
//
// The data buffer will not be accessed after the transfer has been detached
// from the owner. So you can cancel or destroy an active transfer and free up
// the data buffer.
//
// Do not change any field when the transfer has the SCHEDULED status.

struct  _AXICAT_AL_TWI_XFR
{
    U8             *buf;                // [IN]  Read/write data buffer
    U16             sc;                 // [IN]  Store count: bytes to read/write
    U16             xfrd;               // [OUT] Bytes transferred
    U8              slave_ad;           // [IN]  Slave address
    FLAG            dir;                // [IN]  Read or write (AXICAT_TWI_DIR_Xxx constant)
    FLAG            force_stop;         // [IN]  Force stop (1) or allow restart (0)
    FLAG            last;               // [IN]  Last slave Tx or Rx data payload y/n
    FLAG            nack_rsp;           // [OUT] NACK (1) or ACK (0) response on SLA+W, SLA+R or last data byte

    volatile
    U8              status;             // [OUT] Transfer status (AXICAT_AL_TWI_ST_Xxx)
};


// SPI transfer status

#define AXICAT_AL_SPI_ST_SUCCESS        0   // Completed: success
#define AXICAT_AL_SPI_ST_CANCELLED      1   // Completed: cancelled
#define AXICAT_AL_SPI_ST_SKIPPED        2   // Completed: skipped
#define AXICAT_AL_SPI_ST_SCHEDULED     15   // Scheduled (must be non-zero)


// SPI transfer.
//
// This structure is the public part of a larger structure allocated and owned
// by the AL context. You mustn't allocate the structure yourself; use the
// corresponding Create() and Destroy() functions instead.
//
// The data buffer will not be accessed after the transfer has been detached
// from the owner. So you can cancel or destroy an active transfer and free up
// the data buffer.
//
// Do not change any field when the transfer has the SCHEDULED status.

struct  _AXICAT_AL_SPI_XFR
{
    U8             *buf;                // Read/write data buffer
    U16             sc;                 // Store count: bytes to read/write
    U16             xfrd;               // Bytes transferred
    U8              ss;                 // Slave select line (0..3)

    volatile
    U8              status;             // Transfer status (AXICAT_AL_SPI_ST_Xxx)
};


// 1-Wire transfer status

#define AXICAT_AL_OW_ST_SUCCESS         0   // Completed: success
#define AXICAT_AL_OW_ST_CANCELLED       1   // Completed: cancelled
#define AXICAT_AL_OW_ST_SKIPPED         2   // Completed: skipped
#define AXICAT_AL_OW_ST_SCHEDULED      15   // Scheduled (must be non-zero)


// 1-Wire transfer command identifier

#define AXICAT_AL_OW_XFR_ID_RESET       0
#define AXICAT_AL_OW_XFR_ID_TOUCH_BITS  1
#define AXICAT_AL_OW_XFR_ID_ENUM        2
#define AXICAT_AL_OW_XFR_ID_PROBE       3


// 1-Wire transfer.
//
// This structure is the public part of a larger structure allocated and owned
// by the AL context. You mustn't allocate the structure yourself; use the
// corresponding Create() and Destroy() functions instead.
//
// The data buffer will not be accessed after the transfer has been detached
// from the owner. So you can cancel or destroy an active transfer and free up
// the data buffer.
//
// Do not change any field when the transfer has the SCHEDULED status.

struct  _AXICAT_AL_OW_XFR
{
    // Note: The buffer is put outside the union. We want to preserve the
    // buffer pointer so the AxiCat Server or other programs can use the
    // pointer for caching purposes.

    // id=AXICAT_AL_OW_XFR_ID_TOUCH_BITS
    U8             *buf;                // Read/write data buffer

    union
    {
        // id=AXICAT_AL_OW_XFR_ID_RESET
        struct
        {
            FLAG            pd;                 // Presence detected y/n
        }
            reset;

        // id=AXICAT_AL_OW_XFR_ID_TOUCH_BITS
        struct
        {
            U16             sc;                 // Store count: bits to transfer
            U16             xfrd;               // Bits transferred
            FLAG            spu;                // Activate strong pull-up y/n
        }
            touch_bits;

        // id=AXICAT_AL_OW_XFR_ID_ENUM
        struct
        {
            FLAG            found;                  // ROM code enumerated y/n
            U8              rom_code[8];            // Enumerated ROM code
            FLAG            next;                   // Start enumeration (0) or enumerate next (1)
            FLAG            alarm;                  // Search all (0) or search alarmed (1)
            FLAG            enum_family;            // Enumerate family code y/n (1/0)
            U8              family_code;            // Family code to enumerate
            FLAG            smart_on_enabled;       // DS2409 smart on enabled y/n (1/0)
            FLAG            smart_on_aux;           // DS2409 smart on main (0) or smart on aux (1)
            U8              smart_on_rom_code[8];   // DS2409 ROM code
        }
            enumerate;

        // id=AXICAT_AL_OW_XFR_ID_PROBE
        struct
        {
            FLAG            found;                  // ROM code probed y/n
            U8              rom_code[8];            // ROM code
        }
            probe;
    };

    U8              id;                 // Transfer identifier (AXICAT_AL_OW_XFR_ID_Xxx)

    volatile
    U8              status;             // Transfer status (AXICAT_AL_OW_ST_Xxx)
};


// Create an instance of an AxiCat AL.

AXICAT_AL_EXPORT
AXICAT_AL_HANDLE  DynllCall  AXICAT_AL_Create (SERIAL_INTF *serial_intf);


// Destroy the given AL instance and all associated transfer objects.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_Destroy (AXICAT_AL_HANDLE handle);


// Step the given AL.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_Step (AXICAT_AL_HANDLE handle, FLAG *idle);


// Check whether the connection with the AxiCat is still good.

AXICAT_AL_EXPORT
AXICAT_AL_CONN_STATE  DynllCall  AXICAT_AL_Get_Conn_State (AXICAT_AL_HANDLE handle);


// Get the version string (UTF-8).

AXICAT_AL_EXPORT
U8 *  DynllCall  AXICAT_AL_Get_Version (AXICAT_AL_HANDLE handle);


AXICAT_AL_EXPORT
AXICAT_FW_VERSION  DynllCall  AXICAT_AL_Get_FW_Version (AXICAT_AL_HANDLE handle);


// Set the I/O direction of the given GPIO pin.
//
// pin: GPIO pin (0..16).
// dir: Set direction (AXICAT_AL_GPIO_DIR_Xxx constant).

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_GPIO_Set_Dir (AXICAT_AL_HANDLE handle, U8 pin, FLAG dir);


// Write the output state of the given GPIO pin.
//
// pin:   GPIO pin (0..16).
// state: Output state.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_GPIO_Write (AXICAT_AL_HANDLE handle, U8 pin, FLAG state);


// Create a GPIO transfer object. The object is always owned by the given AL
// context.
//
// All fields are initialized to zero. As such the status is set to SUCCESS.

AXICAT_AL_EXPORT
AXICAT_AL_GPIO_XFR *  DynllCall  AXICAT_AL_GPIO_Xfr_Create (AXICAT_AL_HANDLE handle);


// Destroy the given GPIO transfer.
//
// If the transfer is still scheduled, it'll be cleaned up afterwards.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_GPIO_Xfr_Destroy (AXICAT_AL_GPIO_XFR *xfr);


// Schedule the given GPIO transfer.
//
// Return value:
// * =0: Error.
// * =1: Scheduled.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_GPIO_Xfr_Schedule (AXICAT_AL_GPIO_XFR *xfr);


// Request cancellation of the given transfer.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_GPIO_Xfr_Cancel (AXICAT_AL_GPIO_XFR *xfr);


// Enable the TWI function.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Enable (AXICAT_AL_HANDLE handle);


// Disable the TWI function.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Disable (AXICAT_AL_HANDLE handle);


// Enable the TWI slave.
//
// Variable slave_ad: 0..127.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Slave_Enable (AXICAT_AL_HANDLE handle, U8 slave_ad, FLAG gen_call);


// Disable the TWI slave.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Slave_Disable (AXICAT_AL_HANDLE handle);


// Set the speed of the TWI.
//
// The speed variable can be: 50000, 100000, 200000, 400000

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Set_Speed (AXICAT_AL_HANDLE handle, U32 speed);


// Set the raw speed of the TWI.
//
// Variable twbr: 0..255.
// Variable twps: 0..3.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Set_Speed_Raw (AXICAT_AL_HANDLE handle, U8 twbr, U8 twps);


// Create a TWI transfer object. The object is always owned by the given AL
// context.
//
// All fields are initialized to zero. As such the status is set to SUCCESS.

AXICAT_AL_EXPORT
AXICAT_AL_TWI_XFR *  DynllCall  AXICAT_AL_TWI_Xfr_Create (AXICAT_AL_HANDLE handle);


// Destroy the given TWI transfer.
//
// If the transfer is still scheduled, it'll be cleaned up afterwards.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_TWI_Xfr_Destroy (AXICAT_AL_TWI_XFR *xfr);


// Schedule the given transfer as TWI master.
//
// Return value:
// * =0: Error.
// * =1: Scheduled.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Xfr_Master_Schedule (AXICAT_AL_TWI_XFR *xfr);


// Schedule the given transfer as TWI slave.
//
// Return value:
// * =0: Error.
// * =1: Scheduled.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_TWI_Xfr_Slave_Schedule (AXICAT_AL_TWI_XFR *xfr);


// Request cancellation of the given transfer.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_TWI_Xfr_Cancel (AXICAT_AL_TWI_XFR *xfr);


// Enable the SPI function.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Enable (AXICAT_AL_HANDLE handle);


// Disable the SPI function.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Disable (AXICAT_AL_HANDLE handle);


// Set the speed of the SPI.
//
// The speed variable can be: 750000, 1500000, 3000000, 6000000

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Set_Speed (AXICAT_AL_HANDLE handle, U32 speed);


// Set the raw speed of the SPI.
//
// Variable cr: 0..3.
// Variable x2: 0 or 1.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Set_Speed_Raw (AXICAT_AL_HANDLE handle, U8 cr, FLAG x2);


// Configure the SPI.
//
// Variable polarity: Clock polarity, SCK is low (0) or high (1) when idle.
// Variable phase: Clock phase, sample on leading (0) or trailing (1) edge.
// Variable order: Bit order msb->lsb (0) or lsb->msb (1).

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Configure (AXICAT_AL_HANDLE handle, FLAG polarity, FLAG phase, FLAG order);


// Create an SPI transfer object. The object is always owned by the given AL
// context.
//
// All fields are initialized to zero. As such the status is set to SUCCESS.

AXICAT_AL_EXPORT
AXICAT_AL_SPI_XFR *  DynllCall  AXICAT_AL_SPI_Xfr_Create (AXICAT_AL_HANDLE handle);


// Destroy the given SPI transfer.
//
// If the transfer is still scheduled, it'll be cleaned up afterwards.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_SPI_Xfr_Destroy (AXICAT_AL_SPI_XFR *xfr);


// Schedule the given SPI transfer.
//
// Return value:
// * =0: Error.
// * =1: Scheduled.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_SPI_Xfr_Schedule (AXICAT_AL_SPI_XFR *xfr);


// Request cancellation of the given transfer.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_SPI_Xfr_Cancel (AXICAT_AL_SPI_XFR *xfr);


// Enable the 1-Wire function.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Enable (AXICAT_AL_HANDLE handle);


// Disable the 1-Wire function.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Disable (AXICAT_AL_HANDLE handle);


// Create a 1-Wire transfer object. The object is always owned by the given AL
// context.
//
// All fields are initialized to zero. As such the status is set to SUCCESS.

AXICAT_AL_EXPORT
AXICAT_AL_OW_XFR *  DynllCall  AXICAT_AL_OW_Xfr_Create (AXICAT_AL_HANDLE handle);


// Destroy the given 1-Wire transfer.
//
// If the transfer is still scheduled, it'll be cleaned up afterwards.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_OW_Xfr_Destroy (AXICAT_AL_OW_XFR *xfr);


// Schedule the given 1-Wire transfer.
//
// Return value:
// * =0: Error.
// * =1: Scheduled.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_OW_Xfr_Schedule (AXICAT_AL_OW_XFR *xfr);


// Request cancellation of the given transfer.

AXICAT_AL_EXPORT
VOID  DynllCall  AXICAT_AL_OW_Xfr_Cancel (AXICAT_AL_OW_XFR *xfr);


// Enable the given UART.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Enable (AXICAT_AL_HANDLE handle, U8 uart);


// Disable the given UART.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Disable (AXICAT_AL_HANDLE handle, U8 uart);


// Set the baudrate of the given UART.
//
// Variable baudrate: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Baudrate (AXICAT_AL_HANDLE handle, U8 uart, U32 baudrate);


// Set the raw speed of the given UART.
//
// Variable ubrr: 000h..FFFh.
// Variable x2:   0, 1.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Baudrate_Raw (AXICAT_AL_HANDLE handle, U8 uart, U16 ubrr, U8 x2);


// Set the number of data bits for the given UART.
//
// Variable data_bits: 5, 6, 7, 8, 9.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Data_Bits (AXICAT_AL_HANDLE handle, U8 uart, U8 data_bits);


// Set the number of stop bits for the given UART.
//
// Variable stop_bits: 1, 2.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Stop_Bits (AXICAT_AL_HANDLE handle, U8 uart, U8 stop_bits);


// Set the Rx timeout for the given UART.
//
// Variable timeout: 0..65535 (milliseconds).

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Set_Rx_Timeout (AXICAT_AL_HANDLE handle, U8 uart, U16 timeout);


// Write data bytes from the given UART.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Write (AXICAT_AL_HANDLE handle, U8 uart, U8 *buf, U32 len, U32 *xfrd);


// Read data bytes from the given UART.

AXICAT_AL_EXPORT
FLAG  DynllCall  AXICAT_AL_UART_Read (AXICAT_AL_HANDLE handle, U8 uart, U8 *buf, U32 len, U32 *xfrd);


#ifdef  _DEBUG


// Forcibly mark the AxiCat as detached.

VOID  AXICAT_AL_Dbg_Mark_Detached (AXICAT_AL_HANDLE handle);


#endif  // _DEBUG


#ifdef  __cplusplus

}

#endif


#endif  // __AXICAT_AL_H__
