
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// rpidetect.c
//
// Detect the presence of the RPi SoC.
//
// Language: GCC4 gnu89
//
// History:
//
//   2015-02-22  Peter S'heeren, Axiris
//
//      * Created.
//
//   2015-02-23  Peter S'heeren, Axiris
//
//      * Released.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2015  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#include "rpidetect.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>


// Strings

static  const   CHAR    match_hw_asciiz[]      = { "Hardware\t: " };
static  const   CHAR    match_rev_asciiz[]     = { "Revision\t: " };
static  const   CHAR    match_bcm2708_asciiz[] = { "BCM2708" };
static  const   CHAR    match_bcm2709_asciiz[] = { "BCM2709" };


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


VOID  rpidetect (RPIDETECT_IO *io)
{
    int     fd;
    size_t  xfrd;
    U32     rev_word;
    FLAG    rev_word_parsed;

    // Init
    rev_word_parsed = 0;

    // Set up the I/O structure
    io->chip = RPIDETECT_CHIP_UNKNOWN;

    fd = open("/proc/cpuinfo",O_RDONLY);
//    fd = open("cpuinfo_rpi_b.txt",O_RDONLY);    // TEST.
//    fd = open("rpi2_cpuinfo.txt",O_RDONLY);    // TEST.
    if (fd == -1) goto Err;

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

                        if (strncmp(line_buf,match_hw_asciiz,sizeof(match_hw_asciiz)-1) == 0)
                        {
                            CHAR   *s;

                            if (io->chip != RPIDETECT_CHIP_UNKNOWN) goto Err;

                            s = line_buf + (sizeof(match_hw_asciiz)-1);

                            // Look up chip name (this name should appear as
                            // the last part of the line)
                            if (strcmp(s,match_bcm2708_asciiz) == 0)
                            {
                                io->chip = RPIDETECT_CHIP_BCM2835;
                            }
                            else
                            if (strcmp(s,match_bcm2709_asciiz) == 0)
                            {
                                io->chip = RPIDETECT_CHIP_BCM2836;
                            }
                            else
                            {
                                // Unknown chip - the program is running on a
                                // system other than RPi
                                goto Success;
                            }
                        }
                        else
                        if (strncmp(line_buf,match_rev_asciiz,sizeof(match_rev_asciiz)-1) == 0)
                        {
                            CHAR   *s;

                            if (rev_word_parsed) goto Err;

                            s = line_buf + (sizeof(match_rev_asciiz)-1);

                            if (!parse_hex_u32(&s,&rev_word)) goto Err;
                            rev_word_parsed = 1;
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

            // Check the resulting values
            if (!rev_word_parsed) goto Err;

            goto Success;
        }
        else
        {
            // File I/O error occured
            goto Err;
        }
    }


    // Successful parsing of the cpuinfo text. At this point:
    // - io->chip is non-zero.
    // - rev_word is valid (rev_word_valid=1).

  Success:

    if (io->chip != RPIDETECT_CHIP_UNKNOWN)
    {
        if ((rev_word & 0x800000) == 0)
        {
            // Formatting of revision field: (RPi 1 style)
            // - Bit 15.. 0: Revision
            // -     22..16: <reserved>
            // -         23: Encoded flag (0)
            // -         24: Warranty void y/n
            // -     31..25: <reserved>

            io->bsc_index     = ((rev_word & 0xFFFF) < 4) ? 0 : 1;
            io->warranty_void = (rev_word & 0x1000000) ? 1 : 0;
        }
        else
        {
            // Formatting of revision field: (RPi 2 style)
            // - Bit  3.. 0: Revision (PCB revision)
            // -     11.. 4: Model (A, B, A+, B+, ...)
            // -     15..12: Processor (BCM2835, BCM2836)
            // -     19..16: Manufacturer
            // -     22..20: Memory size
            // -         23: Encoded flag (1)
            // -         24: <reserved>
            // -         25: Warranty void y/n
            // -     31..26: <reserved>

            // The encoded flag must be one
            if ((rev_word & 0x800000) == 0) goto Err;

            io->bsc_index     = 1;
            io->warranty_void = (rev_word & 0x2000000) ? 1 : 0;
        }
    }
    // else: no "Hardware" parameter was found

    goto Done;


  Err:

    io->chip = RPIDETECT_CHIP_UNKNOWN;
    goto Done;


  Done:

    if (fd != -1) close(fd);
}

