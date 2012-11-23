
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocardserver.c
//
// AbioCard server.
//
// Language: GCC4 gnu89
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
//      * Released.
//
//   2012-09-10  Peter S'heeren, Axiris
//
//      * Added detection of revision number to set the default BSC index.
//      * Released.
//
//   2012-10-19  Peter S'heeren, Axiris
//
//      * Added support for AbioCard model B.
//      * Released.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2012  Peter S'heeren, Axiris
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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "bcm2835_detect.h"
#include "abiocard.h"


// You can define DISABLE_ABIOCARD here or in the makefile if you want to test
// the server process with an actual AbioCard. This option allows you to test
// the server process on a system other than a BCM2835-based computer.
//
//#define DISABLE_ABIOCARD


static
VOID  destroy_socket (int s)
{
    shutdown(s,SHUT_RDWR);
    close(s);
}


static  int                 server_socket       = -1;
static  int                 client_socket       = -1;
static  FLAG                req_exit            = 0;
static  U16                 client_timeout_sec  = 30;

static  BCM2835_DETECT_IO   detect_io           = { 0 };
static  FLAG                bsc_parsed          = 0;

static  ABIOCARD_INIT_IO    abiocard_init_io    = { 0 };

static  U16                 server_port;
static  struct sockaddr_in  server_sin;
static  struct sockaddr_in  client_sin;
static  socklen_t           client_sin_size;


// Receive buffer. This is a cache for reading data from the client socket.

#define RCV_BUF_LEN         64

static  CHAR                rcv_buf[RCV_BUF_LEN];


// Received command buffer. The end-of-line character is not stored.
//
// The longest incoming command is:
//
// PW0011xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
// -> 40 characters

#define RCV_CMD_LEN         40

static  CHAR                rcv_cmd[RCV_CMD_LEN];
static  U32                 rcv_cmd_si;
static  U32                 rcv_cmd_fi;
static  FLAG                rcv_cmd_error;


// Send response buffer.

#define SND_RSP_LEN         128

static  CHAR                snd_rsp[SND_RSP_LEN];
static  U32                 snd_rsp_si;


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
        int     n;

        n = send(client_socket,snd_rsp,snd_rsp_si,0);
        if (n < 0) return 0;

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_rtc_poll(&poll_info);
    if (!ok) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    // Set the RTC date & time
    ok = abiocard_rtc_set_time(&time);
    if (!ok) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_adc_convert(&cnv_data);
    if (!ok) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_ioexp_read(&ioexp_read_word);
    if (!ok) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_ioexp_write(ioexp_write_word);
    if (!ok) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_pwm_read_range((U8*)&pwm_read_data,start,cnt);
    if (!ok) return 0;
#else
    // Emulate
    if (start >= 17) return 0;
    if (cnt == 0) return 0;
    if (cnt > 17) return 0;
    if ((start + cnt) > 17) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_pwm_write_range((U8*)&pwm_write_data,start,cnt);
    if (!ok) return 0;
#else
    // Emulate
    if (start >= 17) return 0;
    if (cnt == 0) return 0;
    if (cnt > 17) return 0;
    if ((start + cnt) > 17) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_pwm2_read_range((ABIOCARD_PWM2_CH*)&pwm2_read_data,start,cnt);
    if (!ok) return 0;
#else
    // Emulate
    if (start >= 16) return 0;
    if (cnt == 0) return 0;
    if (cnt > 16) return 0;
    if ((start + cnt) > 16) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_pwm2_write_range((ABIOCARD_PWM2_CH*)&pwm2_write_data,start,cnt);
    if (!ok) return 0;
#else
    // Emulate
    if (start >= 16) return 0;
    if (cnt == 0) return 0;
    if (cnt > 16) return 0;
    if ((start + cnt) > 16) return 0;
#endif

    return 1;
}


static
FLAG  parse_cmd_qp (VOID)
{
    static  U8      pwm2_read_prescaler_val;

    FLAG    ok;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

#ifndef DISABLE_ABIOCARD
    ok = abiocard_pwm2_read_prescaler(&pwm2_read_prescaler_val);
    if (!ok) return 0;
#endif

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

#ifndef DISABLE_ABIOCARD
    ok = abiocard_pwm2_write_prescaler(pwm2_write_prescaler_val);
    if (!ok) return 0;
#endif

    return 1;
}


static
FLAG  parse_cmd_hi (VOID)
{
    U8      mask;

    // Check for the end of the command
    if (fetch_cmd_ch() != 0) return 0;

    mask = 0;
    if (abiocard_init_io.rtc_present)   mask |= 0b00000001;
    if (abiocard_init_io.ioexp_present) mask |= 0b00000010;
    if (abiocard_init_io.adc_present)   mask |= 0b00000100;
    if (abiocard_init_io.pwm_present)   mask |= 0b00001000;
    if (abiocard_init_io.pwm2_present)  mask |= 0b00010000;

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
    // DBG.
    {
        U32     u;

        printf("Command: ");
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
    int     n;

    // Reset the response buffer
    snd_rsp_si = 0;

    // Reset the received command buffer
    rcv_cmd_si    = 0;
    rcv_cmd_error = 0;

    for (;;)
    {
        n = recv(client_socket,rcv_buf,RCV_BUF_LEN,0);
        if (n == 0)
        {
            // The peer has shut down the socket connection
            break;
        }
        else
        if (n < 0)
        {
            // A socket error has occured, error value is set in errno. A read
            // timeout also ends up here.
            break;
        }
        else
        {
            U32     i;
            CHAR    c;

            // Process the received characters
            for (i = 0; i < (U32)n; i++)
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
#ifndef DISABLE_ABIOCARD
    FLAG    ok;

    // Initialise the AbioCard driver
    ok = abiocard_init(&abiocard_init_io);
    if (!ok) return;
#else
    // Emulate
    abiocard_init_io.rtc_present   = 0;
    abiocard_init_io.ioexp_present = 0;
    abiocard_init_io.adc_present   = 0;
    abiocard_init_io.pwm_present   = 1;
    abiocard_init_io.pwm2_present  = 0;
#endif

    main_client();

#ifndef DISABLE_ABIOCARD
    // De-initialise the AbioCard driver
    abiocard_deinit();
#endif
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
    FLAG    server_port_specified;
    int     arg_index;

    // Parse the command line. Remember that argv[0] always is valid and refers
    // to the executable file.

    server_port_specified = 0;
    arg_index             = 1;

    while (arg_index < argc)
    {
        if (strcmp(argv[arg_index],"-p") == 0)
        {
            CHAR   *s;
            U32     val;

            // Allow the option only once
            if (server_port_specified)
            {
                printf("Server port number already specified\n");
                return 0;
            }

            arg_index++;
            if (arg_index == argc) return 0;

            // Parse IPv4 port
            s = argv[arg_index];
            if (!parse_u32(&s,&val,1,65535)) return 0;
            server_port = (U16)val;
            server_port_specified = 1;

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
        if (strcmp(argv[arg_index],"-bsc") == 0)
        {
            CHAR   *s;
            U32     val;

            arg_index++;
            if (arg_index == argc) return 0;

            // Parse index
            s = argv[arg_index];
            if (!parse_u32(&s,&val,0,1)) return 0;

            abiocard_init_io.bsc_index = (U8)val;
            bsc_parsed = 1;

            if ((*s) != 0) return 0;

            arg_index++;
        }
        else
            return 0;
    }

    // A server port type must be specified
    if (!server_port_specified)
    {
        printf("Server port number not specified\n");
        return 0;
    }

    return 1;
}


static
VOID  usage_cmdline (VOID)
{
    printf
    (
        "\n" \
        "AbioCard server\n" \
        "\n" \
        "Command line arguments:\n" \
        "-p n       Server port, n=1..65535\n" \
        "-t n       Time-out value (seconds) for the client connection, n=5..65535\n" \
        "-bsc n     Choose the BSC controller, n=0..1\n" \
        "\n"
    );
}


/*=============================================================================
Entry point
=============================================================================*/


int  main (int argc, char *argv[])
{
    int     error;
    FLAG    ok;
    int     so_reuseaddr;


    // Print command line usage information if no arguments are present
    if (argc <= 1)
    {
        usage_cmdline();
        goto Stop;
    }


    // Parse the command line
    ok = parse_cmdline(argc,argv);
    if (!ok)
    {
        printf("Error in command line\n");
        goto Stop;
    }


#ifndef DISABLE_ABIOCARD

    // Detect the BCM2835 hardware
    ok = bcm2835_detect(&detect_io);
    if (!ok)
    {
        printf("Error detecting BCM2835 hardware\n");
        goto Stop;
    }

    if (!detect_io.res_detected)
    {
        printf("BCM2835 hardware not detected\n");
        goto Stop;
    }


    if (!bsc_parsed)
    {
        // If the bsc parameter wasn't parsed, determine the BSC index from the
        // revision number:
        // * <0004:  Computer rev. 1, use BSC0
        // * >=0004: Computer rev. 2, use BSC1

        if (detect_io.res_revision >= 4) abiocard_init_io.bsc_index = 1;
    }


#endif


#ifndef DISABLE_ABIOCARD

    // Acquire root permission
    seteuid(0);

    // Check the effective user ID
    if (geteuid() != 0)
    {
        printf("Can't acquire root permission\n");
        goto Stop;
    }

#endif


    // Create a blocking socket
    server_socket = socket(AF_INET,SOCK_STREAM,0);
    if (server_socket == -1)
    {
        printf("Create server socket: error %d\n",errno);
        goto Stop;
    }

    // To avoid the error code "address already in use" (errno 98) when the
    // socket is bound to a local address, we set the SO_REUSEADDR flag  first.
    so_reuseaddr = 1;
    error = setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
    if (error == -1)
    {
        printf("Failed to set option for server socket: error %d\n",errno);
        goto Stop;
    }

    // Associate a local address with the socket

    memset(&server_sin,0,sizeof(server_sin));
    server_sin.sin_family      = AF_INET;
    server_sin.sin_port        = htons(server_port);
    server_sin.sin_addr.s_addr = INADDR_ANY;

    error = bind(server_socket,(const struct sockaddr *)&server_sin,sizeof(server_sin));
    if (error == -1)
    {
        printf("Bind server socket: error %d\n",errno);
        goto Stop;
    }

    // Listen to incoming connections (set limit to one)
    error = listen(server_socket,1);
    if (error == -1)
    {
        printf("Listen to server socket: error %d\n",errno);
        goto Stop;
    }

    for (;;)
    {
        struct  timeval     tv;

        //printf("Accepting incoming connection...\n");

        // Accept client socket
        client_sin_size = sizeof(client_sin);
        client_socket = accept(server_socket,(struct sockaddr *)&client_sin,&client_sin_size);
        if (client_socket == -1)
        {
            printf("Accept client socket: error %d\n",errno);
            goto Stop;
        }

/*
        // DBG.
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
*/

        // Specify a read timeout for the client socket
        tv.tv_sec  = client_timeout_sec;
        tv.tv_usec = 0;
        setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(struct timeval));

        // Process the client connection until it's shut down
        process_client();

        // Destroy the client socket
        destroy_socket(client_socket);
        client_socket = -1;

        if (req_exit) goto Stop;
    }

  Stop:

    if (client_socket != -1) destroy_socket(client_socket);
    if (server_socket != -1) destroy_socket(server_socket);

    return 0;
}

