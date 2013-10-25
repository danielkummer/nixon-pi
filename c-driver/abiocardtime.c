
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
//
//   2012-09-10  Peter S'heeren, Axiris
//
//      * Added detection of revision number to set the default BSC index.
//
//   2013-01-25  Peter S'heeren, Axiris
//
//      * Added support for the i2c-dev interface.
//
//   2013-09-17  Peter S'heeren, Axiris
//
//      * Adapted for the reworked abiocard driver.
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


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "bcm2835_detect.h"
#include "bsc_i2cbus.h"
#include "i2cdev_i2cbus.h"
#include "abiocard.h"


#define INTF_TYPE_BSC           0
#define INTF_TYPE_I2CDEV        1


static  BCM2835_DETECT_IO           detect_io = { 0 };
static  FLAG                        bsc_parsed = 0;
static  FLAG                        cmd_s = 0;
static  FLAG                        cmd_u = 0;
static  FLAG                        cmd_p = 0;

static  U8                          intf_type = INTF_TYPE_BSC;

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

static  ABIOCARD_INIT_IO            abiocard_init_io = { 0 };

static  CHAR                        dev_name[20];
static  struct  stat                stat_info;

static  ABIOCARD_RTC_POLL_INFO      poll_info;
static  ABIOCARD_RTC_TIME           rtc_time;
static  time_t                      seconds;

static  struct  tm                  new_local_t = { 0 };

static  ABIOCARD_HANDLE             abiocard_handle = 0;


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

            intf_type                      = INTF_TYPE_BSC;
            bsc_i2cbus_create_io.bsc_index = (U8)val;
            bsc_parsed                     = 1;

            if ((*s) != 0) return 0;

            arg_index++;
        }
        else
        if (strcmp(argv[arg_index],"-dev") == 0)
        {
            CHAR   *s;
            U32     val;

            arg_index++;
            if (arg_index == argc) return 0;

            intf_type                           = INTF_TYPE_I2CDEV;
            i2cdev_i2cbus_create_io.i2cdev_name = argv[arg_index];

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
        "-dev FILE  Specify the I2C device path\n" \
        "           Example FILE: /dev/i2c-0\n" \
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

    ok = abiocard_rtc_poll(abiocard_handle,&poll_info);
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


    if (intf_type == INTF_TYPE_BSC)
    {
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
            // The bsc parameter wasn't parsed

            // Determine the BSC index from the revision number:
            // * <0004:  Computer rev. 1, use BSC0
            // * >=0004: Computer rev. 2, use BSC1

            if (detect_io.res_revision >= 4) bsc_i2cbus_create_io.bsc_index = 1;

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


/*
    // DBG.
    printf("abiocardtime: intf %d, ",intf_type);
    if (intf_type == INTF_TYPE_BSC)
        printf("bsc %d\n",bsc_i2cbus_create_io.bsc_index);
    else
        printf("dev %s\n",i2cdev_i2cbus_create_io.i2cdev_name);
*/


    // Acquire root permission
    seteuid(0);

    // Check the effective user ID
    if (geteuid() != 0)
    {
        printf("Can't acquire root permission\n");
        goto Stop;
    }


    // Create the i2cbus interface

    if (intf_type == INTF_TYPE_BSC)
    {
        abiocard_init_io.i2cbus_intf = BSC_I2CBus_Create(&bsc_i2cbus_create_io);
    }
    else
    {
        abiocard_init_io.i2cbus_intf = I2CDev_I2CBus_Create(&i2cdev_i2cbus_create_io);
    }

    if (!abiocard_init_io.i2cbus_intf) goto Stop;


    // Initialise the AbioCard driver
    abiocard_handle = abiocard_init(&abiocard_init_io);
    if (!abiocard_handle)
    {
        if (abiocard_init_io.err == ABIOCARD_INIT_ERR_NO_CARD)
        {
            printf("AbioCard not present on ");

            if (intf_type == INTF_TYPE_BSC)
                printf("BSC%d\n",bsc_i2cbus_create_io.bsc_index);
            else
                printf("%s\n",i2cdev_i2cbus_create_io.i2cdev_name);
        }
        else
            printf("Can't initialise AbioCard\n");

        goto Stop;
    }


    if (cmd_s)
    {
        ok = abiocard_rtc_set_time(abiocard_handle,&rtc_time);
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

    // Deinitialise the AbioCard driver
    abiocard_deinit(abiocard_handle);

    // Destroy the i2cbus interface
    if (intf_type == INTF_TYPE_BSC)
    {
        BSC_I2CBus_Destroy(abiocard_init_io.i2cbus_intf);
    }
    else
    {
        I2CDev_I2CBus_Destroy(abiocard_init_io.i2cbus_intf);
    }

    return 0;
}

