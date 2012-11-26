
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocardtime.c
//
// AbioCard time.
//
// This application sets the system date and time according to the RTC's date
// and time.
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
#include <time.h>
#include <errno.h>

#include "bcm2835_detect.h"
#include "abiocard.h"


static  BCM2835_DETECT_IO       detect_io = { 0 };
static  FLAG                    bsc_parsed = 0;

static  ABIOCARD_INIT_IO        abiocard_init_io = { 0 };

static  ABIOCARD_RTC_POLL_INFO  poll_info;
static  ABIOCARD_RTC_TIME       rtc_time;
static  time_t                  seconds;

static  struct  tm              new_local_t = { 0 };

static  FLAG                    cmd_s = 0;
static  FLAG                    cmd_u = 0;
static  FLAG                    cmd_p = 0;


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
        if (strcmp(argv[arg_index],"-u") == 0)
        {
            cmd_u = 1;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-p") == 0)
        {
            cmd_p = 1;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-s") == 0)
        {
            CHAR   *s;
            U32     val;

            // Parse the date and time
            //
            // Format: Y-M-D h:m:s

            arg_index++;
            if (arg_index == argc) return 0;
            s = argv[arg_index];

            // Year (2000..2099)
            if (!parse_u32(&s,&val,2000,2099)) return 0;
            val -= 2000;
            rtc_time.year = (U8)val;

            // Delimiter
            if ((*s) != '-') return 0;
            s++;

            // Month (1..12)
            if (!parse_u32(&s,&val,1,12)) return 0;
            rtc_time.month = (U8)val;

            // Delimiter
            if ((*s) != '-') return 0;
            s++;

            // Month (1..31)
            if (!parse_u32(&s,&val,1,31)) return 0;
            rtc_time.day = (U8)val;

            // Delimiter
            if ((*s) != ' ') return 0;
            s++;

            // Hour (0..23)
            if (!parse_u32(&s,&val,0,23)) return 0;
            rtc_time.hour = (U8)val;

            // Delimiter
            if ((*s) != ':') return 0;
            s++;

            // Minute (0..59)
            if (!parse_u32(&s,&val,0,59)) return 0;
            rtc_time.minute = (U8)val;

            // Delimiter
            if ((*s) != ':') return 0;
            s++;

            // Second (0..59)
            if (!parse_u32(&s,&val,0,59)) return 0;
            rtc_time.second = (U8)val;

            // End-of-string
            if ((*s) != 0) return 0;

            arg_index++;

            cmd_s = 1;
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

    return 1;
}


static
VOID  usage_cmdline (VOID)
{
    printf
    (
        "\n" \
        "AbioCard time\n" \
        "\n" \
        "Command line arguments:\n" \
        "-s STRING  Set date and time in the RTC\n" \
        "           Example STRING: \"2012-08-18 20:24:48\"\n" \
        "-u         Update the system time using the RTC\n" \
        "-p         Poll the RTC and print\n" \
        "-bsc n     Choose the BSC controller, n=0..1\n" \
        "\n" \
        "When multiple commands are specified, they're executed in the following\n" \
        "order: -s -u -p\n" \
        "\n"
    );
}


/*=============================================================================
Entry point
=============================================================================*/


static
FLAG  rtc_poll (FLAG print)
{
    FLAG    ok;

    ok = abiocard_rtc_poll(&poll_info);
    if (!ok)
    {
        printf("Can't poll the RTC\n");
        return 0;
    }

    if (print)
    {
        printf("Power-up...: %d\n",poll_info.power_up);
        printf("Battery low: %d\n",poll_info.battery_low);
        if (!poll_info.power_up)
            printf("Date & time: %04d-%02d-%02d %02d:%02d:%02d\n",
                   poll_info.time.year+2000,poll_info.time.month,poll_info.time.day,
                   poll_info.time.hour,poll_info.time.minute,poll_info.time.second);
    }

    return 1;
}


int  main (int argc, char *argv[])
{
    int     i;
    FLAG    ok;


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


    // Acquire root permission
    seteuid(0);

    // Check the effective user ID
    if (geteuid() != 0)
    {
        printf("Can't acquire root permission\n");
        goto Stop;
    }


    // Initialise the AbioCard driver
    if (!abiocard_init(&abiocard_init_io))
    {
        if (abiocard_init_io.err == ABIOCARD_INIT_ERR_NO_CARD)
            printf("AbioCard not present on BSC%d\n",abiocard_init_io.bsc_index);
        else
            printf("Can't initialise AbioCard\n");

        goto Stop;
    }


    if (cmd_s)
    {
        ok = abiocard_rtc_set_time(&rtc_time);
        if (!ok)
        {
            printf("Can't write the RTC, or invalid date and time specified\n");
            goto Stop;
        }
    }


    if (cmd_u)
    {
        ok = rtc_poll(0);
        if (!ok) goto Stop;

        if (poll_info.power_up)
        {
            printf("RTC power-up detected, RTC time not set\n");
        }
        else
        {
            // Get the timezone information. We need the value of the daylight
            // field specifically.
            //
            // Note: DON'T rely on the time() function:
            //
            //    time(&seconds);
            //    cur_local_t = localtime(&seconds);
            //    printf("tm_isdst %d\n",cur_local_t->tm_isdst);
            //
            // It turns out tm_isdst doesn't mean the same as the daylight
            // field. When the computer is powered up, it assumes the year 1970
            // and tm_isdst is zero, even if your timezone has daylight saving.
            // Then, when you set the correct date and time, and reboot,
            // tm_isdst is one. It turns out tm_isdst depends on the current
            // date and time, so we can't use this value.
            //
            tzset();


            printf("Setting system date & time...\n");

            // Set up the tm structure (a.k.a. broken-down time) based on the
            // RTC's date and time.
            //
            // Notes:
            // * Field tm_year values 0.. mean 1900.. while RTC field year
            //   values 0..99 mean 2000..2099.
            // * Field tm_month starts at zero while RTC field month starts at
            //   one.
            // * Field tm_isdst is copied from the current setting.
            //
            new_local_t.tm_year  = poll_info.time.year + 100;
            new_local_t.tm_mon   = poll_info.time.month - 1;
            new_local_t.tm_mday  = poll_info.time.day;
            new_local_t.tm_hour  = poll_info.time.hour;
            new_local_t.tm_min   = poll_info.time.minute;
            new_local_t.tm_sec   = poll_info.time.second;
            new_local_t.tm_isdst = daylight;

            seconds = mktime(&new_local_t);
            if (seconds == -1)
            {
                printf("Can't convert RTC time to seconds\n");
                goto Stop;
            }

            // Set the system date and time
            i = stime(&seconds);
            if (i == -1)
            {
                printf("Can't set system date and time, errno %d\n",errno);
                goto Stop;
            }
        }
    }


    if (cmd_p)
    {
        ok = rtc_poll(1);
        if (!ok) goto Stop;
    }


  Stop:

    // De-initialise the AbioCard driver
    abiocard_deinit();

    return 0;
}

