
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// bcm2835_detect.c
//
// Detect the BCM2835 hardware.
//
// Language: GCC4 gnu89
//
// History:
//
//   2012-07-30  Peter S'heeren, Axiris
//
//      * Created.
//
//   2012-08-18  Peter S'heeren, Axiris
//
//      * Released.
//
//   2012-09-10  Peter S'heeren, Axiris
//
//      * Added detection of revision number.
//      * Released.
//
//   2012-11-30  Peter S'heeren, Axiris
//
//      * The revision number is an hexadecimal value. Replaced the parser.
//
//   2012-12-10  Peter S'heeren, Axiris
//
//      * Added support for the warranty void bit in the revision code.
//      * Fixed the hexadecimal parser and added overflow checking.
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


#include <fcntl.h>

#include "bcm2835_detect.h"


// Hardware.
//
// This is the line we're looking for in /proc/cpuinfo. We're looking for an
// exact match.

static  const   CHAR    match_hw_asciiz[] =
{
    'H','a','r','d','w','a','r','e',9,':',' ',
    'B','C','M','2','7','0','8',
    0
};


// Revision.
//
// This is the line we're looking for in /proc/cpuinfo. We're looking for an
// exact match excluding the actual revision number. We parse the revision
// number separately.

static  const   CHAR    match_rev_asciiz[] =
{
    'R','e','v','i','s','i','o','n',9,':',' ',
    0
};

#define MATCH_REV_LEN   (sizeof(match_rev_asciiz)-1)


// The read buffer is filled with chunk of data read from cpuinfo. The buffer is
// essentially a read cache.

#define RD_BUF_LEN      512

static  CHAR    rd_buf[RD_BUF_LEN];
static  U32     rd_buf_sc;


// The line buffer is populated with characters from the read buffer. Character
// linefeed (ASCII 10) is the delimiter between lines.

#define LINE_BUF_LEN    64

static  CHAR    line_buf[LINE_BUF_LEN + 1];
static  U32     line_buf_si;
static  FLAG    line_buf_error;


static
FLAG  parse_hex_u32 (CHAR **s, U32 *res)
{
    U32     val;
    U32     prev_val;
    U8      digit;
    CHAR    c;
    FLAG    digit_parsed;

    val          = 0;
    digit_parsed = 0;

    for (;;)
    {
        c = **s;
        if ((c >= '0') && (c <= '9')) digit = c - '0'; else
        if ((c >= 'A') && (c <= 'F')) digit = c - 'A' + 10; else
        if ((c >= 'a') && (c <= 'f')) digit = c - 'a' + 10; else
            break;

        digit_parsed = 1;
        (*s)++;

        prev_val = val;

        val = val * 16 + (U32)digit;    // Add the digit to the result

        if (val < prev_val) return 0;   // Check for overflow
    }

    if (!digit_parsed) return 0;

    *res = val;
    return 1;
}


FLAG  bcm2835_detect (BCM2835_DETECT_IO *io)
{
    int     fd;
    size_t  xfrd;
    FLAG    detected;
    FLAG    io_ok;

    io->res_detected = 0;
    io->res_word     = 0;

    fd = open("/proc/cpuinfo",O_RDONLY);
//    fd = open("cpuinfo.txt",O_RDONLY);    // TEST.
    if (fd == -1) return 0;

    // Result values
    io_ok    = 1;
    detected = 0;

    // Reset the line buffer
    line_buf_si    = 0;
    line_buf_error = 0;

    for (;;)
    {
        // Return value of read():
        // * >0: Number of bytes transferred.
        // * =0: Attempted to read from position at or after end-of-file i.e.
        //       no more data to read.
        // * -1: Error occured, error code set in errno.
        //
        xfrd = read(fd,rd_buf,RD_BUF_LEN);
        if (xfrd > 0)
        {
            U32     i;

            rd_buf_sc = (U32)xfrd;

            for (i = 0; i < rd_buf_sc; i++)
            {
                CHAR    c;

                c = rd_buf[i];
                if (c == 10)
                {
                    if (line_buf_error == 0)
                    {
                        // Zero-terminate the line buffer
                        line_buf[line_buf_si] = 0;

                        if (strcmp(line_buf,match_hw_asciiz) == 0)
                        {
                            io->res_detected = 1;
                        }
                        else
                        if (strncmp(line_buf,match_rev_asciiz,MATCH_REV_LEN) == 0)
                        {
                            CHAR   *s;
                            U32     val;

                            s = line_buf + MATCH_REV_LEN;

                            if (parse_hex_u32(&s,&val))
                            {
                                io->res_word = val;
                            }
                        }
                    }

                    // Reset the line buffer
                    line_buf_si    = 0;
                    line_buf_error = 0;
                }
                else
                {
                    if (line_buf_error == 0)
                    {
                        if (line_buf_si < LINE_BUF_LEN)
                        {
                            // Store the character in the line buffer
                            line_buf[line_buf_si] = c;
                            line_buf_si++;
                        }
                        else
                        {
                            // Line buffer overflow
                            line_buf_error = 1;
                        }
                    }
                    // else: Skip all remaining incoming characters of the
                    //       current line
                }
            }
        }
        else
        if (xfrd == 0)
        {
            // Reached the end-of-file

            break;
        }
        else
        {
            // File I/O error occured
            io_ok = 0;

            break;
        }
    }

    close(fd);

    return io_ok;
}

