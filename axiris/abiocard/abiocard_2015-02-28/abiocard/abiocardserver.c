
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocardserver.c
//
// AbioCard server.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2012-07-26  Peter S'heeren, Axiris
//
//      * Created.
//
//   2012-08-18  Peter S'heeren, Axiris
//
//      * Released.
//
//   2012-09-09  Peter S'heeren, Axiris
//
//      * Added the ability to choose the BSC.
//
//   2012-09-10  Peter S'heeren, Axiris
//
//      * Added detection of revision number to set the default BSC index.
//
//   2012-10-19  Peter S'heeren, Axiris
//
//      * Added support for AbioCard model B.
//
//   2013-01-25  Peter S'heeren, Axiris
//
//      * Added support for the i2c-dev interface.
//
//   2013-09-17  Peter S'heeren, Axiris
//
//      * Adapted for the reworked abiocard driver.
//      * Added option -stdio.
//      * Added processing of partially transmitted buffers by send().
//
//   2014-07-07  Peter S'heeren, Axiris
//
//      * Added support for AxiCat.
//      * Added support for Win32.
//
//   2014-08-27  Peter S'heeren, Axiris
//
//      * Converted Win32 version from console application to application.
//      * Added console option to Win32 version.
//      * Added verbose option.
//
//   2015-02-27  Peter S'heeren, Axiris
//
//      * Added support for RPi 2.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2012-2015  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#include "abiocard.h"
#include "osrtl.h"
#include "axicat_i2cbus.h"

#include <stdio.h>
#include <string.h>

#if defined linux

#include "rpidetect.h"
#include "bsc_i2cbus.h"
#include "i2cdev_i2cbus.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#elif defined WIN32

#include "console.h"

#include <winsock.h>

#endif


/*=============================================================================
Sockets
=============================================================================*/


#if defined linux


typedef int     SOCKET_HANDLE;

typedef const void     *SETSOCKOPT_VALUE;

#define closesocket(s)  close(s)
#define socket_errno    errno

#define SHUTDOWN_READ_WRITE     SHUT_RDWR


#elif defined WIN32


typedef SOCKET  SOCKET_HANDLE;

typedef int     socklen_t;

typedef const char     *SETSOCKOPT_VALUE;

#define socket_errno    WSAGetLastError()

#define SHUTDOWN_READ_WRITE     2               // SD_BOTH isn't defined


static  FLAG    winsock_inited  = 0;


#endif


static
VOID  deinit_sockets (VOID)
{
#if defined linux

    // Nothing to do

#elif defined WIN32

    if (winsock_inited)
    {
        WSACleanup();

        winsock_inited = 0;
    }

#endif
}


static
FLAG  init_sockets (VOID)
{
#if defined linux

    // Nothing to do
    return 1;

#elif defined WIN32

    if (!winsock_inited)
    {
        WSADATA     wsadata;
        int         error;

        error = WSAStartup(0x0202,&wsadata);
        if (error) return 0;

        // WSAStartup() returns a version equal to or lower than the requested
        // version, never a higher value.
        if (wsadata.wVersion != 0x0202)
        {
            WSACleanup();
            return 0;
        }

        winsock_inited = 1;
    }

    return 1;


#endif  // WIN32
}


static
VOID  destroy_socket (SOCKET_HANDLE s)
{
    shutdown(s,SHUTDOWN_READ_WRITE);
    closesocket(s);
}


static
VOID  set_socket_rcv_timeout (SOCKET_HANDLE s, U32 seconds)
{
#if defined linux

    struct  timeval     tv;

    tv.tv_sec  = seconds;
    tv.tv_usec = 0;
    setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(SETSOCKOPT_VALUE)&tv,sizeof(struct timeval));

#elif defined WIN32

    DWORD   val;        // Timeout value in milliseconds

    val = seconds * 1000;
    setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(SETSOCKOPT_VALUE)&val,sizeof(val));

#endif
}


/*=============================================================================
Program data
=============================================================================*/


// Command line help message

#if defined linux

#define SER_PATH_EXAMPLE    "/dev/ttyUSB0"

#elif defined WIN32

#define SER_PATH_EXAMPLE    "\\\\.\\COM4"

#endif


// Enumeration of client interfaces

typedef enum    _CLIENT_TYPE
{
    CLIENT_TYPE_NONE,
    CLIENT_TYPE_SOCKET,
    CLIENT_TYPE_STDIO
}
    CLIENT_TYPE;


// Enumeration of available interfaces for connecting an AbioCard

typedef enum    _INTF_TYPE
{
    INTF_TYPE_NONE,
    INTF_TYPE_AXICAT,
#ifdef  linux
    INTF_TYPE_BSC,
    INTF_TYPE_I2CDEV
#endif
}
    INTF_TYPE;


static  FLAG                        usage               = 0;
static  FLAG                        verbose             = 0;
static  SOCKET_HANDLE               server_socket       = -1;
static  SOCKET_HANDLE               client_socket       = -1;
static  FLAG                        req_exit            = 0;
static  U16                         client_timeout_sec  = 30;
static  U8                          client_type         = CLIENT_TYPE_NONE;
static  U8                          intf_type           = INTF_TYPE_NONE;

#if defined WIN32
static  FLAG                        console_created     = 0;
#endif

static  U16                         server_port;
static  struct sockaddr_in          server_sin;
static  struct sockaddr_in          client_sin;
static  socklen_t                   client_sin_size;

static  ABIOCARD_INIT_IO            abiocard_init_io    = { 0 };
static  ABIOCARD_HANDLE             abiocard_handle     = 0;

static  AXICAT_I2CBUS_CREATE_IO     axicat_i2cbus_create_io =
{
    0,      // path
    1       // verbose
};


#ifdef  linux

static  RPIDETECT_IO                rpidetect_io        = { 0 };

static  CHAR                        dev_name[20];
static  struct  stat                stat_info;

static  BSC_I2CBUS_CREATE_IO        bsc_i2cbus_create_io =
{
    .bsc_index = 0,
    .verbose   = 1
};

static  I2CDEV_I2CBUS_CREATE_IO     i2cdev_i2cbus_create_io =
{
    .i2cdev_name = 0,
    .verbose     = 1
};

#endif  // linux


// Receive buffer. This is a cache for reading data from the client socket.

#define RCV_BUF_LEN         256

static  CHAR                rcv_buf[RCV_BUF_LEN];


// Received command buffer. The end-of-line character is not stored.
//
// The longest incoming command is QW0010, followed by 16 times xxxxxxxx
//
// -> 134 characters

#define RCV_CMD_LEN         134

static  CHAR                rcv_cmd[RCV_CMD_LEN];
static  U32                 rcv_cmd_si;
static  U32                 rcv_cmd_fi;
static  FLAG                rcv_cmd_error;


// Send response buffer.

#define SND_RSP_LEN         256

static  CHAR                snd_rsp[SND_RSP_LEN];
static  U32                 snd_rsp_si;


// Send the given number of characters. This is a blocking function. It returns
// when either all characters have been sent, or the I/O channel has been
// closed.
//
// Return value:
// * =0: I/O channel has been closed.
// * =1: All data bytes have been successfully sent.

typedef FLAG    SEND_FN (CHAR *buf,  U32 cnt);


// Receive one or more characters. This is a blocking function. It returns when
// at least one character is received, or the I/O channel has been closed.
//
// Return value:
// * =0: I/O channel has been closed.
// * >0: Number of data bytes received.

typedef U32     RCV_FN (CHAR *buf,  U32 cnt);


// Callback functions for I/O with client

static  SEND_FN            *send_fn;
static  RCV_FN             *rcv_fn;


/*=============================================================================
Callback functions for socket I/O with client
=============================================================================*/


static
FLAG  socket_send_cb (CHAR *buf, U32 cnt)
{
    int     n;

    for (;;)
    {
        n = send(client_socket,buf,cnt,0);
        if (n < 0) return 0;

        if (n == 0)
        {
            // Couldn't transmit a single character, wait a bit and try again
            OSRTL_SleepMs(200*1000);
        }
        else
        {
            cnt -= n;
            if (cnt == 0) return 1;

            printf(".");
            // A partial transmission occured, send the remaining data
            buf += n;
        }
    }
}


static
U32  socket_rcv_cb (CHAR *buf, U32 cnt)
{
    int     n;

    n = recv(client_socket,buf,cnt,0);
    if (n == 0)
    {
        // The peer has shut down the socket connection
        return 0;
    }
    else
    if (n < 0)
    {
        // A socket error has occured, error value is set in errno. A read
        // timeout also ends up here.
        return 0;
    }
    else
    {
        // Report the number of characters received
        return (U32)n;
    }
}


/*=============================================================================
Callback functions for standard I/O with client
=============================================================================*/


static
FLAG  stdio_send_cb (CHAR *buf, U32 cnt)
{
    int     n;

    while (cnt > 0)
    {
        n = fputc(*buf,stdout);
        if (n == -1) return 0;

        buf++;
        cnt--;
    }

    return 1;
}


static
U32  stdio_rcv_cb (CHAR *buf, U32 cnt)
{
    int     n;

    n = fgetc(stdin);
    if (n == -1) return 0;


    // Report one character has been received
    (*buf) = (CHAR)n;
    return 1;
}


/*=============================================================================
Client connection
=============================================================================*/


static
CHAR  fetch_cmd_ch (VOID)
{
    CHAR    c;

    if (rcv_cmd_fi < rcv_cmd_si)
    {
        c = rcv_cmd[rcv_cmd_fi];
        rcv_cmd_fi++;
    }
    else
    {
        c = 0;
    }

    return c;
}


// Fetch one decimal digit from the command buffer

static
FLAG  fetch_dec_1 (U8 *digit)
{
    CHAR    c;

    c = fetch_cmd_ch();
    if (c == 0) return 0;

    if ((c >= '0') && (c <= '9'))
    {
        (*digit) = c - '0';
        return 1;
    }
    else
        return 0;
}


// Fetch a given number of decimal digits from the command buffer

static
FLAG  fetch_dec_n (U8 digits, U32 *val)
{
    U32     res;
    U8      digit;

    res = 0;
    while (digits > 0)
    {
        if (!fetch_dec_1(&digit)) return 0;
        res = res * 10 + digit;

        digits--;
    }

    (*val) = res;
    return 1;
}


// Fetch one hexadecimal digit from the command buffer

static
FLAG  fetch_hex_1 (U8 *digit)
{
    CHAR    c;

    c = fetch_cmd_ch();
    if (c == 0) return 0;

    if ((c >= '0') && (c <= '9'))
    {
        (*digit) = c - '0';
        return 1;
    }
    else
    if ((c >= 'A') && (c <= 'F'))
    {
        (*digit) = c + 10 - 'A';
        return 1;
    }
    else
        return 0;
}


// Fetch a given number of hexadecimal digits from the command buffer

static
FLAG  fetch_hex_n (U8 digits, U32 *val)
{
    U32     res;
    U8      digit;

    res = 0;
    while (digits > 0)
    {
        if (!fetch_hex_1(&digit)) return 0;
        res = (res << 4) | digit;

        digits--;
    }

    (*val) = res;
    return 1;
}


static
FLAG  flush_rsp (VOID)
{
    if (snd_rsp_si > 0)
    {
        FLAG    ok;

/*
        // Debug
        {
            U32     u;

            printf("Response: ");
            for (u = 0; u < snd_rsp_si; u++) printf("%c",snd_rsp[u]);
            printf("\n");
        }
*/

        ok = send_fn(snd_rsp,snd_rsp_si);
        if (!ok) return 0;

        // Reset the response buffer
        snd_rsp_si = 0;
    }

    return 1;
}


static
FLAG  store_rsp_ch (CHAR c)
{
    if (snd_rsp_si == SND_RSP_LEN)
    {
        if (!flush_rsp()) return 0;
    }

    snd_rsp[snd_rsp_si] = c;
    snd_rsp_si++;
    return 1;
}


static
FLAG  start_rsp (VOID)
{
    if (!store_rsp_ch(rcv_cmd[0])) return 0;
    if (!store_rsp_ch(rcv_cmd[1])) return 0;

    return 1;
}


static
FLAG  store_eol (VOID)
{
    return store_rsp_ch(10);
}


static
FLAG  store_dec_n (U8 digits, U32 val)
{
    static  U32     div_list[9] =
    {
        10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
    };

    U8      i;
    U32     digit;
    CHAR    c;
    FLAG    ok;

    if ((digits < 1) || (digits > 10)) return 0;

    // Transform to the range 9..0, meaning the digit's position (0..) in the
    // decimal value
    digits--;

    // Runs from 8 to 0 including, meaning the digit's position plus one (1..)
    // in the decimal value
    i = 8;

    for (;;)
    {
        digit = val / div_list[i];
        val = val % div_list[i];

        if (i < digits)
        {
            c = '0' + (U8)digit;
            ok = store_rsp_ch(c);
            if (!ok) return 0;
        }

        if (i == 0) break;

        i--;
    }

    // Always format the character for the units digit
    c = '0' + (U8)val;
    ok = store_rsp_ch(c);
    if (!ok) return 0;

    return 1;
}


static
FLAG  store_hex_n (U8 digits, U32 val)
{
    U8      i;
    U32     digit;
    CHAR    c;
    FLAG    ok;

    // The value contains 8 nibbles representing 8 hexadecimal digits
    i = 8;
    for (;;)
    {
        if (digits >= i)
        {
            // The digit is copied from the highest nibble of the value
            digit = (val >> 28);

            // Format the digit
            if (digit >= 10)
                c = 'A' + (U8)digit - 10;
            else
                c = '0' + (U8)digit;

            ok = store_rsp_ch(c);
            if (!ok) return 0;
        }

        i--;
        if (i == 0) return 1;

        // Shift out the most significant nibble
        val <<= 4;
    }
}


static
FLAG  parse_cmd_cr (VOID)
{
    static  ABIOCARD_RTC_POLL_INFO   poll_info;

    FLAG    ok;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_rtc_poll(abiocard_handle,&poll_info);
    if (!ok) return 0;

    // Format the response
    start_rsp();
    store_dec_n(2,poll_info.time.year);
    store_dec_n(2,poll_info.time.month);
    store_dec_n(2,poll_info.time.day);
    store_dec_n(2,poll_info.time.hour);
    store_dec_n(2,poll_info.time.minute);
    store_dec_n(2,poll_info.time.second);
    store_dec_n(1,poll_info.power_up);
    store_dec_n(1,poll_info.battery_low);
    store_eol();

    return 1;
}


static
FLAG  parse_cmd_cw (VOID)
{
    static  ABIOCARD_RTC_TIME    time;

    FLAG    ok;
    U32     val;

    // Parse the year
    ok = fetch_dec_n(2,&val);
    if (!ok) return 0;
    time.year = (U8)val;

    // Parse the month
    ok = fetch_dec_n(2,&val);
    if (!ok) return 0;
    time.month = (U8)val;

    // Parse the day
    ok = fetch_dec_n(2,&val);
    if (!ok) return 0;
    time.day = (U8)val;

    // Parse the hour
    ok = fetch_dec_n(2,&val);
    if (!ok) return 0;
    time.hour = (U8)val;

    // Parse the minute
    ok = fetch_dec_n(2,&val);
    if (!ok) return 0;
    time.minute = (U8)val;

    // Parse the second
    ok = fetch_dec_n(2,&val);
    if (!ok) return 0;
    time.second = (U8)val;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    // Set the RTC date & time
    ok = abiocard_rtc_set_time(abiocard_handle,&time);
    if (!ok) return 0;

    return 1;
}


static
FLAG  parse_cmd_ar (VOID)
{
    static  ABIOCARD_ADC_CNV_DATA    cnv_data;

    FLAG    ok;
    U32     i;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_adc_convert(abiocard_handle,&cnv_data);
    if (!ok) return 0;

    // Format the response

    start_rsp();

    for (i = 0; i < 8; i++)
    {
        U32     val;

        val = ((cnv_data[i][0] << 8) | cnv_data[i][1]) & 4095;

        store_hex_n(3,val);
    }

    store_eol();

    return 1;
}


static
FLAG  parse_cmd_er (VOID)
{
    static  U8      ioexp_read_word;

    FLAG    ok;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_ioexp_read(abiocard_handle,&ioexp_read_word);
    if (!ok) return 0;

    // Format the response
    start_rsp();
    store_hex_n(2,ioexp_read_word);
    store_eol();

    return 1;
}


static
FLAG  parse_cmd_ew (VOID)
{
    static  U8      ioexp_write_word;

    FLAG    ok;
    U32     val;

    // Parse the word
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    ioexp_write_word = (U8)val;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_ioexp_write(abiocard_handle,ioexp_write_word);
    if (!ok) return 0;

    return 1;
}


static
FLAG  parse_cmd_pr (VOID)
{
    static  U8      pwm_read_data[17];

    FLAG    ok;
    U32     val;
    U8      start;
    U8      cnt;
    U8      i;

    // Parse the start index value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    start = (U8)val;

    // Parse the count value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    cnt = (U8)val;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_pwm_read_range(abiocard_handle,(U8*)&pwm_read_data,start,cnt);
    if (!ok) return 0;

    // Format the response
    start_rsp();
    store_hex_n(2,start);
    store_hex_n(2,cnt);
    for (i = 0; i < cnt; i++) store_hex_n(2,pwm_read_data[i]);
    store_eol();

    return 1;
}


static
FLAG  parse_cmd_pw (VOID)
{
    static  U8      pwm_write_data[17];

    FLAG    ok;
    U32     val;
    U8      start;
    U8      cnt;
    U8      i;

    // Parse the start index value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    start = (U8)val;

    // Parse the count value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    cnt = (U8)val;

    if (cnt > 17) return 0;

    for (i = 0; i < cnt; i++)
    {
        // Parse the PWM value (2 hex. digits)
        ok = fetch_hex_n(2,&val);
        if (!ok) return 0;
        pwm_write_data[i] = (U8)val;
    }

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_pwm_write_range(abiocard_handle,(U8*)&pwm_write_data,start,cnt);
    if (!ok) return 0;

    return 1;
}


static
FLAG  parse_cmd_qr (VOID)
{
    static  ABIOCARD_PWM2_CH    pwm2_read_data[16];

    FLAG    ok;
    U32     val;
    U8      start;
    U8      cnt;
    U8      i;


    // Parse the start index value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    start = (U8)val;

    // Parse the count value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    cnt = (U8)val;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_pwm2_read_range(abiocard_handle,(ABIOCARD_PWM2_CH*)&pwm2_read_data,start,cnt);
    if (!ok) return 0;

    // Format the response

    start_rsp();
    store_hex_n(2,start);
    store_hex_n(2,cnt);

    for (i = 0; i < cnt; i++)
    {
        val = pwm2_read_data[i].on_pos;
        if (pwm2_read_data[i].always_on) val |= 0x1000;
        store_hex_n(4,val);

        val = pwm2_read_data[i].off_pos;
        if (pwm2_read_data[i].always_off) val |= 0x1000;
        store_hex_n(4,val);
    }

    store_eol();

    return 1;
}


static
FLAG  parse_cmd_qw (VOID)
{
    static  ABIOCARD_PWM2_CH    pwm2_write_data[16];

    FLAG    ok;
    U32     val;
    U8      start;
    U8      cnt;
    U8      i;

    // Parse the start index value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    start = (U8)val;

    // Parse the count value (2 hex. digits)
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    cnt = (U8)val;

    if (cnt > 16) return 0;

    for (i = 0; i < cnt; i++)
    {
        // Parse the value for ON (4 hex. digits)
        ok = fetch_hex_n(4,&val);
        if (!ok) return 0;
        if (val >= 0x2000) return 0;

        pwm2_write_data[i].on_pos    = val & 0x0FFF;
        pwm2_write_data[i].always_on = (val & 0x1000) ? 1 : 0;

        // Parse the value for OFF (4 hex. digits)
        ok = fetch_hex_n(4,&val);
        if (!ok) return 0;
        if (val >= 0x2000) return 0;

        pwm2_write_data[i].off_pos    = val & 0x0FFF;
        pwm2_write_data[i].always_off = (val & 0x1000) ? 1 : 0;
    }

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_pwm2_write_range(abiocard_handle,(ABIOCARD_PWM2_CH*)&pwm2_write_data,start,cnt);
    if (!ok) return 0;

    return 1;
}


static
FLAG  parse_cmd_qp (VOID)
{
    static  U8      pwm2_read_prescaler_val;

    FLAG    ok;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_pwm2_read_prescaler(abiocard_handle,&pwm2_read_prescaler_val);
    if (!ok) return 0;

    // Format the response
    start_rsp();
    store_hex_n(2,pwm2_read_prescaler_val);
    store_eol();

    return 1;
}


static
FLAG  parse_cmd_qq (VOID)
{
    static  U8      pwm2_write_prescaler_val;

    FLAG    ok;
    U32     val;

    // Parse the word
    ok = fetch_hex_n(2,&val);
    if (!ok) return 0;
    pwm2_write_prescaler_val = (U8)val;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    ok = abiocard_pwm2_write_prescaler(abiocard_handle,pwm2_write_prescaler_val);
    if (!ok) return 0;

    return 1;
}


static
FLAG  parse_cmd_hi (VOID)
{
    U8      mask;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    mask = 0;
    if (abiocard_init_io.rtc_present)   mask |= 0x01;   /* 0b00000001 */
    if (abiocard_init_io.ioexp_present) mask |= 0x02;   /* 0b00000010 */
    if (abiocard_init_io.adc_present)   mask |= 0x04;   /* 0b00000100 */
    if (abiocard_init_io.pwm_present)   mask |= 0x08;   /* 0b00001000 */
    if (abiocard_init_io.pwm2_present)  mask |= 0x10;   /* 0b00010000 */

    //printf("HI: mask %02Xh\n",mask);

    // Format the response
    start_rsp();
    store_hex_n(2,mask);
    store_eol();

    return 1;
}


static
FLAG  parse_cmd_qu (VOID)
{
    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    req_exit = 1;

    return 1;
}


typedef FLAG    PARSE_CMD_FN (VOID);

typedef struct  _CMD_INFO       CMD_INFO;

struct  _CMD_INFO
{
    CHAR            cmd_1;
    CHAR            cmd_2;
    PARSE_CMD_FN   *parse_cmd_fn;
};


#define CMD_INFO_CNT    13

static  CMD_INFO    cmd_info_list[CMD_INFO_CNT] =
{
    { 'C','R', parse_cmd_cr },          // Clock: read
    { 'C','W', parse_cmd_cw },          // Clock: write
    { 'A','R', parse_cmd_ar },          // ADC: read
    { 'E','R', parse_cmd_er },          // I/O Expander: read
    { 'E','W', parse_cmd_ew },          // I/O Expander: write
    { 'P','R', parse_cmd_pr },          // PWM: read range
    { 'P','W', parse_cmd_pw },          // PWM: write range
    { 'Q','R', parse_cmd_qr },          // PWM2: read range
    { 'Q','W', parse_cmd_qw },          // PWM2: write range
    { 'Q','P', parse_cmd_qp },          // PWM2: read prescaler
    { 'Q','Q', parse_cmd_qq },          // PWM2: write prescaler
    { 'H','I', parse_cmd_hi },          // Get the hardware information
    { 'Q','U', parse_cmd_qu }           // Quit the server
};


static
FLAG  process_rcv_cmd (VOID)
{
    CHAR        cmd_1;
    CHAR        cmd_2;
    CMD_INFO   *cmd_info;
    U32         cmd_left;
    FLAG        ok;

/*
    // Debug
    {
        U32     u;

        printf("Command.: ");
        for (u = 0; u < rcv_cmd_si; u++) printf("%c",rcv_cmd[u]);
        printf("\n");
    }
*/

    // Reset the command receive buffer
    rcv_cmd_fi = 0;

    // Fetch the two-letter command from the command buffer
    cmd_1 = fetch_cmd_ch(); if (!cmd_1) return 0;
    cmd_2 = fetch_cmd_ch(); if (!cmd_2) return 0;

    // Look up the command in the list of command codes
    cmd_info = cmd_info_list;
    cmd_left = CMD_INFO_CNT;
    for (;;)
    {
        if ((cmd_info->cmd_1 == cmd_1) && (cmd_info->cmd_2 == cmd_2)) break;

        cmd_left--;
        if (cmd_left == 0) return 0;

        cmd_info++;
    }

    // Parse and execute the command
    ok = cmd_info->parse_cmd_fn();
    if (!ok) return 0;

    return 1;
}


static
VOID  main_client (VOID)
{
    U32     xfrd;

    // Reset the response buffer
    snd_rsp_si = 0;

    // Reset the received command buffer
    rcv_cmd_si    = 0;
    rcv_cmd_error = 0;

    for (;;)
    {
        // Stop this loop and shut down the client socket if the AbioCard
        // module has marked the device as detached.
        if (abiocard_is_detached(abiocard_handle)) return;

        // Receive data from the client socket. This is a blocking call.
        xfrd = rcv_fn(rcv_buf,RCV_BUF_LEN);
        if (xfrd == 0)
        {
            // The I/O channel has been closed
            break;
        }
        else
        {
            U32     i;
            CHAR    c;

            // Process the received characters
            for (i = 0; i < xfrd; i++)
            {
                c = rcv_buf[i];
                if ((c == 13) || (c == 10))
                {
                    if (!rcv_cmd_error)
                    {
                        if (rcv_cmd_si > 0)
                        {
                            process_rcv_cmd();

                            if (req_exit) return;
                        }
                        // else: Skip solitary end-of-line characters
                    }

                    // Reset the received command buffer
                    rcv_cmd_si    = 0;
                    rcv_cmd_error = 0;
                }
                else
                if ((c >= 32) && (c <= 126))
                {
                    if (!rcv_cmd_error)
                    {
                        if (rcv_cmd_si < RCV_CMD_LEN)
                        {
                            rcv_cmd[rcv_cmd_si] = c;
                            rcv_cmd_si++;
                        }
                        else
                        {
                            // Command buffer overflow
                            rcv_cmd_error = 1;
                        }
                    }
                }
                else
                {
                    // Invalid character received
                    rcv_cmd_error = 1;
                }
            }

            // Flush any outstanding characters for transmission
            if (!flush_rsp()) break;
        }
    }
}


static
VOID  process_client (VOID)
{
    // Create the i2cbus interface

    if (intf_type == INTF_TYPE_AXICAT)
    {
        abiocard_init_io.i2cbus_intf = AXICAT_I2CBus_Create(&axicat_i2cbus_create_io);
    }

#ifdef  linux

    else
    if (intf_type == INTF_TYPE_BSC)
    {
        abiocard_init_io.i2cbus_intf = BSC_I2CBus_Create(&bsc_i2cbus_create_io);
    }
    else
    if (intf_type == INTF_TYPE_I2CDEV)
    {
        abiocard_init_io.i2cbus_intf = I2CDev_I2CBus_Create(&i2cdev_i2cbus_create_io);
    }

#endif  // linux

    else
    {
        // Shouldn't reach this point
        goto Stop;
    }

    if (!abiocard_init_io.i2cbus_intf) goto Stop;

    // Initialise the AbioCard driver
    abiocard_handle = abiocard_init(&abiocard_init_io);
    if (!abiocard_handle) goto Stop;

    main_client();

  Stop:

    // Deinitialize the AbioCard driver
    abiocard_deinit(abiocard_handle);

    // Destroy the i2cbus interface

    if (intf_type == INTF_TYPE_AXICAT)
    {
        AXICAT_I2CBus_Destroy(abiocard_init_io.i2cbus_intf);
    }

#ifdef  linux

    else
    if (intf_type == INTF_TYPE_BSC)
    {
        BSC_I2CBus_Destroy(abiocard_init_io.i2cbus_intf);
    }
    else
    if (intf_type == INTF_TYPE_I2CDEV)
    {
        I2CDev_I2CBus_Destroy(abiocard_init_io.i2cbus_intf);
    }

#endif  // linux

    abiocard_handle              = 0;
    abiocard_init_io.i2cbus_intf = 0;
}


/*=============================================================================
Command line
=============================================================================*/


static
FLAG  parse_u32 (CHAR **s, U32 *res, U32 min, U32 max)
{
    U32     val;
    CHAR    c;
    FLAG    digit_parsed;

    val          = 0;
    digit_parsed = 0;

    for (;;)
    {
        c = **s;
        if ((c >= '0') && (c <= '9'))
        {
            digit_parsed = 1;
            (*s)++;
            val = val * 10 + (c - '0');     // Add the digit to the result
        }
        else
        {
            if (digit_parsed)
            {
                if ((val >= min) && (val <= max))
                {
                    *res = val;
                    return 1;
                }
            }

            return 0;
        }
    }
}


static
FLAG  parse_cmdline (int argc, char *argv[])
{
    int     arg_index;

    // Parse the command line. Remember that argv[0] always is valid and refers
    // to the executable file.

    arg_index = 1;

    while (arg_index < argc)
    {
        if (strcmp(argv[arg_index],"-h") == 0)
        {
            usage = 1;

            return 1;
        }
        else
#if defined WIN32
        if (strcmp(argv[arg_index],"-console") == 0)
        {
            console_created |= Console_Create(0,500);

            arg_index++;
        }
        else
#endif
        if (strcmp(argv[arg_index],"-v") == 0)
        {
            verbose = 1;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-p") == 0)
        {
            CHAR   *s;
            U32     val;

            client_type = CLIENT_TYPE_SOCKET;

            arg_index++;
            if (arg_index == argc) return 0;

            // Parse IPv4 port
            s = argv[arg_index];
            if (!parse_u32(&s,&val,1,65535)) return 0;
            server_port = (U16)val;

            if ((*s) != 0) return 0;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-t") == 0)
        {
            CHAR   *s;
            U32     val;

            arg_index++;
            if (arg_index == argc) return 0;

            // Parse the client timeout value (seconds)
            s = argv[arg_index];
            if (!parse_u32(&s,&val,0,0xFFFFFFFF)) return 0;
            if ((val >= 5) && (val <= 65535))
            {
                client_timeout_sec = (U16)val;
            }
            else
            {
                printf("Warning: client timeout value is invalid\n");
            }

            if ((*s) != 0) return 0;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-stdio") == 0)
        {
            client_type = CLIENT_TYPE_STDIO;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-axicat") == 0)
        {
            arg_index++;
            if (arg_index == argc) return 0;

            intf_type                    = INTF_TYPE_AXICAT;
            axicat_i2cbus_create_io.path = argv[arg_index];

            arg_index++;
        }

#ifdef  linux

        else
        if (strcmp(argv[arg_index],"-bsc") == 0)
        {
            CHAR   *s;
            U32     val;

            arg_index++;
            if (arg_index == argc) return 0;

            // Parse index
            s = argv[arg_index];
            if (!parse_u32(&s,&val,0,1)) return 0;

            intf_type                      = INTF_TYPE_BSC;
            bsc_i2cbus_create_io.bsc_index = (U8)val;

            if ((*s) != 0) return 0;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-dev") == 0)
        {
            arg_index++;
            if (arg_index == argc) return 0;

            intf_type                           = INTF_TYPE_I2CDEV;
            i2cdev_i2cbus_create_io.i2cdev_name = argv[arg_index];

            arg_index++;
        }

#endif  // linux

        else
            return 0;
    }

    if (client_type == CLIENT_TYPE_NONE)
    {
        printf("Specify -p or -stdio\n");
        return 0;
    }

#ifdef  WIN32

    if (intf_type == INTF_TYPE_NONE)
    {
        printf("Specify interface\n");
        return 0;
    }

#endif  // WIN32

    return 1;
}


static
VOID  usage_cmdline (VOID)
{
#if defined WIN32
    console_created |= Console_Create(80,25);
#endif

    printf
    (
        "\n"
        "AbioCard server\n"
        "\n"
        "Command line arguments:\n"
        "-h             Display this help and exit\n"
#if defined WIN32
        "-console       Open console\n"
#endif
        "-v             Enable verbose output\n"
        "-p N           Server port, N=1..65535\n"
        "-t N           Time-out value (seconds) for the client connection, N=5..65535\n"
        "-stdio         Use standard I/O instead of server port\n"
        "-axicat PATH   Specify the serial path of the AxiCat\n"
        "               Example PATH: " SER_PATH_EXAMPLE "\n"
#ifdef  linux
        "-bsc N         Choose the BSC controller, N=0..1\n"
        "-dev PATH      Specify the I2C device path\n"
        "               Example PATH: /dev/i2c-0\n"
#endif  // linux
        "\n"
    );
}


/*=============================================================================
Entry point
=============================================================================*/


int  main (int argc, char *argv[])
{
    FLAG    res;
    int     error;
    FLAG    ok;

#ifdef  linux

    int     so_reuseaddr;
    int     i;

#endif


    res = 1;


    // Print command line usage information if no arguments are present
    if (argc <= 1)
    {
        usage_cmdline();
        usage = 1;
        goto Success;
    }


    // Parse the command line
    ok = parse_cmdline(argc,argv);
    if (!ok)
    {
        printf("Error in command line\n");
        goto Err;
    }


    // Print command line usage if specified
    if (usage)
    {
        usage_cmdline();
        goto Success;
    }


#ifdef  linux

    if ((intf_type == INTF_TYPE_NONE) || (intf_type == INTF_TYPE_BSC))
    {
        // Detect the RPi hardware
        rpidetect(&rpidetect_io);

        if (rpidetect_io.chip == RPIDETECT_CHIP_UNKNOWN)
        {
            printf("BCM2835/2836 hardware not detected\n");
            goto Err;
        }


        if (intf_type == INTF_TYPE_NONE)
        {
            // The bsc parameter wasn't parsed

            intf_type = INTF_TYPE_BSC;

            bsc_i2cbus_create_io.bsc_index = rpidetect_io.bsc_index;

            // Check whether the i2c-dev file for the target BSC is present. If
            // the file is present then use the i2c-dev interface rather than
            // direct I/O.

            sprintf(dev_name,"/dev/i2c-%d",bsc_i2cbus_create_io.bsc_index);

            i = stat(dev_name,&stat_info);
            if (i == 0)
            {
                // Switch interface to i2c-dev
                intf_type                           = INTF_TYPE_I2CDEV;
                i2cdev_i2cbus_create_io.i2cdev_name = dev_name;
            }
        }
    }

#endif  // linux


/*
    // Debug
    printf("abiocardserver: intf %d, ",intf_type);
    if (intf_type == INTF_TYPE_AXICAT) printf("axicat %s\n",axicat_i2cbus_create_io.path);
#ifdef  linux
    if (intf_type == INTF_TYPE_BSC) printf("bsc %d\n",bsc_i2cbus_create_io.bsc_index);
    if (intf_type == INTF_TYPE_I2CDEV) printf("dev %s\n",i2cdev_i2cbus_create_io.i2cdev_name);
#endif  // linux
*/


    if (client_type == CLIENT_TYPE_STDIO)
    {
        // Use standard I/O
        send_fn = stdio_send_cb;
        rcv_fn  = stdio_rcv_cb;

        // Process standard I/O
        process_client();
    }
    else
    {
        ok = init_sockets();
        if (!ok)
        {
            printf("Can't initialize sockets\n");
            goto Err;
        }

        // Use client socket I/O
        send_fn = socket_send_cb;
        rcv_fn  = socket_rcv_cb;

        // Create a blocking socket
        server_socket = socket(AF_INET,SOCK_STREAM,0);
        if (server_socket == -1)
        {
            printf("Create server socket: error %d\n",socket_errno);
            goto Err;
        }

#ifdef  linux

        // Set option re-use address. Doing so avoids a failure during bind()
        // after a previous server socket was suddenly destroyed and the TCP/IP
        // stack is still waiting for clients to close their side.
        so_reuseaddr = 1;
        error = setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
        if (error == -1)
        {
            printf("Failed to set option for server socket: error %d\n",socket_errno);
            goto Err;
        }

#endif  // linux

        // Associate a local address with the socket

        memset(&server_sin,0,sizeof(server_sin));
        server_sin.sin_family      = AF_INET;
        server_sin.sin_port        = htons(server_port);
        server_sin.sin_addr.s_addr = INADDR_ANY;

        error = bind(server_socket,(const struct sockaddr *)&server_sin,sizeof(server_sin));
        if (error == -1)
        {
            printf("Bind server socket: error %d\n",socket_errno);
            goto Err;
        }

        // Listen to incoming connections (set limit to one)
        error = listen(server_socket,1);
        if (error == -1)
        {
            printf("Listen to server socket: error %d\n",socket_errno);
            goto Err;
        }

        for (;;)
        {
            //printf("Accepting incoming connection...\n");

            // Accept client socket
            client_sin_size = sizeof(client_sin);
            client_socket = accept(server_socket,(struct sockaddr *)&client_sin,&client_sin_size);
            if (client_socket == -1)
            {
                printf("Accept client socket: error %d\n",socket_errno);
                goto Err;
            }

            // Verbose
            if (verbose)
            {
                U32     client_ip;
                U16     client_port;

                client_ip   = ntohl(client_sin.sin_addr.s_addr);
                client_port = ntohs(client_sin.sin_port);

                printf("Client IPv4 address: %d.%d.%d.%d:%d\n",
                       client_ip >> 24,
                       (client_ip >> 16) & 255,
                       (client_ip >> 8) & 255,
                       client_ip & 255,
                       client_port);
            }

            // Specify a read timeout for the client socket
            set_socket_rcv_timeout(client_socket,client_timeout_sec);

            // Process the client connection until it's shut down
            process_client();

            // Destroy the client socket
            destroy_socket(client_socket);
            client_socket = -1;

            if (req_exit) goto Success;
        }
    }


  Err:

    res = 0;

  Success:

    if (client_socket != -1) destroy_socket(client_socket);
    if (server_socket != -1) destroy_socket(server_socket);

    deinit_sockets();

#if defined WIN32

    if ((console_created) && ((usage) || (!res)))
    {
        // Wait for a key press before closing the console. Since Windows reads
        // an entire line at once before reporting a key press, we better ask
        // the user to punch the enter key.
        printf("Press enter to exit...");
        getchar();
    }

#endif

    return 0;
}


#if defined WIN32

int  WINAPI  WinMain
(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    PSTR        lpCmdLine,
    int         nCmdShow
)
{
    return main(__argc,__argv);
}


#endif
