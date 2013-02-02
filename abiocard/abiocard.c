
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocard.c
//
// AbioCard driver.
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
//   2012-10-19  Peter S'heeren, Axiris
//
//      * Added support for AbioCard model B.
//      * Released.
//
//   2012-12-01  Peter S'heeren, Axiris
//
//      * Fixed setting of the prescaler value.
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
#include <fcntl.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include "abiocard.h"
#include "bsc.h"
#include "debug.h"


/*=============================================================================
I2C adapter interface
=============================================================================*/


static  I2C_CB_TABLE   *i2c_cb    = 0;


/*=============================================================================
BCM2835
=============================================================================*/


// GPIO registers

#define GPIO_REG_GPFSEL0    (0x00 >> 2)         // GPIO Function Select 0


/*=============================================================================
I/O card
=============================================================================*/


#define I2C_AD_RTC          0b1010001
#define I2C_AD_IOEXP        0b0100111
#define I2C_AD_ADC          0b0110011
#define I2C_AD_PWM          0b0001000
#define I2C_AD_PWM2         0b1001000


/*=============================================================================
RTC

NXP PCF2129A real-time clock with integrated quartz crystal

I2C address: 1010001b

Usage of the RTC:
* 24-hour mode only.
* Weekday not used.
* Power-up detection during polling.
* When the OSF flag is set, it remains set until the caller sets a valid date
  and time. Thus after a power-up of the RTC chip, rtc_poll() will return OSF
  set until rtc_set_time() programs a date and time.
=============================================================================*/


#define RTC_TIME            ABIOCARD_RTC_TIME
#define RTC_POLL_INFO       ABIOCARD_RTC_POLL_INFO


static  U8      rtc_init_data[] =
{
    // Start register index
    0,

    // Register 00h - Control_1:
    // * Bit 7: EXT_TEST - external clock test mode
    // * Bit 6: Unused
    // * Bit 5: STOP - stop RTC clock y/n
    // * Bit 4: TSF1 - timestamp 1 interrupt generated y/n (write zero to clear)
    // * Bit 3: POR_OVRD - Power-On Reset Override (PORO) facility disabled; set
    //          logic 0 for normal operation
    // * Bit 2: 12_24 - 24 hour mode selected (0) or 12 hour mode selected (1)
    // * Bit 1: MI - minute interrupt enabled y/n
    // * Bit 0: SI - second interrupt enabled y/n
    0b00000000,

    // Register 01h - Control_2:
    // * Bit 7: MSF - minute or second interrupt generated (write zero to clear)
    // * Bit 6: WDTF - watchdog timer interrupt or reset generated
    // * Bit 5: TSF2 - timestamp 2 interrupt generated y/n (write zero to clear)
    // * Bit 4: AF - alarm interrupt generated (write zero to clear)
    // * Bit 3: Unused
    // * Bit 2: TSIE - Timestamp interrupt enable
    // * Bit 1: AIE - Alarm interrupt enable
    // * Bit 0: Unused
    0b00000000,

    // Register 02h - Control_3:
    // * Bit 7..5: PWRMNG[2..0] - power management setting
    // * Bit    4: BTSE - enable timestamp when battery switch-over y/n
    // * Bit    3: BF - set when battery switch-over occurs (write zero to clear)
    // * Bit    2: BLF - Battery low y/n
    // * Bit    1: BIE - interrupt generated when BF is set
    // * Bit    0: BLIE - interrupt generated when BLF is set
    0b00000000
};

static  I2C_WRITE_IO    rtc_init_write_io = { (U8*)&rtc_init_data, sizeof(rtc_init_data), 0, I2C_AD_RTC };


static  U8              rtc_poll_buf[10];

static  I2C_READ_IO     rtc_poll_read_io = { (U8*)&rtc_poll_buf, sizeof(rtc_poll_buf), 0, I2C_AD_RTC };


static  U8      rtc_time_data[] =
{
    // Start register index
    3,

    // Register 03h - Seconds:
    // * Bit    7: OSF - write zero to clear
    // * Bit 6..4: SECONDS - tens digit (BCD)  - 00..59
    // * BIT 3..0: SECONDS - units digit (BCD) /
    0b00000000,

    // Register 04h - Minutes:
    // * Bit    7: Unused
    // * Bit 6..4: MINUTES - tens digit (BCD)  - 00..59
    // * BIT 3..0: MINUTES - units digit (BCD) /
    0b00000000,

    // Register 05h - Hours:
    // * 24-hour mode:
    //   * Bit 7..6: Unused
    //   * Bit 5..4: HOURS - tens digit (BCD)  - 00..23
    //   * BIT 3..0: HOURS - units digit (BCD) /
    // * 12-hour mode:
    //   * Bit 7..6: Unused
    //   * Bit    5: AMPM - AM (0) or PM (1)
    //   * Bit    4: HOURS - tens digit (BCD)  - 01..12
    //   * BIT 3..0: HOURS - units digit (BCD) /
    0b00000000,

    // Register 06h - Days: (day of the month)
    // * Bit 7..6: Unused
    // * Bit 5..4: DAYS - tens digit (BCD)  - 01..31
    // * BIT 3..0: DAYS - units digit (BCD) /
    0b00000000,

    // Register 07h - Weekdays: (day of the week)
    // * Bit 7..3: Unused
    // * BIT 2..0: WEEKDAYS - 0..6: 0=Sunday, 1=Monday, ...
    0b00000000,

    // Register 08h - Months:
    // * Bit 7..5: Unused
    // * Bit    4: MONTHS - tens digit (BCD)  - 01..12
    // * BIT 3..0: MONTHS - units digit (BCD) /
    0b00000000,

    // Register 09h - Years:
    // * Bit 7..4: YEARS - tens digit (BCD)  - 00..99
    // * BIT 3..0: YEARS - units digit (BCD) /
    0b00000000
};

static  I2C_WRITE_IO    rtc_time_write_io = { (U8*)&rtc_time_data, sizeof(rtc_time_data), 0, I2C_AD_RTC };


static
U8  pbcd_to_bin (U8 pbcd)
{
    return (((pbcd >> 4) * 10) + (pbcd & 0b00001111));
}


static
U8  bin_to_pbcd (U8 bin)
{
    return (bin % 10) | ((bin / 10) << 4);
}


static
FLAG  rtc_probe (VOID)
{
    // Set the start register to zero (for probing purposes only)
    return i2c_cb->write_reg_fn(0,I2C_AD_RTC,0);
}


static
FLAG  rtc_poll (RTC_POLL_INFO *info)
{
    FLAG    ok;

    // Read registers 00h..09h

    // Set the start register
    ok = i2c_cb->write_reg_fn(0,I2C_AD_RTC,0);
    if (!ok) return 0;

    // Read the registers
    ok = i2c_cb->read_fn(0,&rtc_poll_read_io);
    if (!ok) return 0;

    // Control_3 register, bit 2: BLF
    info->battery_low = (rtc_poll_buf[2] & 0b00000100) ? 1 : 0;

    // Check the OSF flag
    if (rtc_poll_buf[3] & 0b10000000)
    {
        // Mark power-up detected
        info->power_up = 1;

        // Date and time - clear
        memset(&info->time,0,sizeof(RTC_TIME));

        // Write the initialisation data. The data block includes the start
        // register index value.
        ok = i2c_cb->write_fn(0,&rtc_init_write_io);
        if (!ok) return 0;
    }
    else
    {
        // Power-up not detected
        info->power_up = 0;

        // Date and time
        info->time.second = pbcd_to_bin(rtc_poll_buf[3] & 0b01111111);
        info->time.minute = pbcd_to_bin(rtc_poll_buf[4] & 0b01111111);
        info->time.hour   = pbcd_to_bin(rtc_poll_buf[5] & 0b00111111);
        info->time.day    = pbcd_to_bin(rtc_poll_buf[6] & 0b00111111);
        info->time.month  = pbcd_to_bin(rtc_poll_buf[8] & 0b00011111);
        info->time.year   = pbcd_to_bin(rtc_poll_buf[9]);
    }

    return 1;
}


static
FLAG  rtc_set_time (RTC_TIME *time)
{
    U8      max_day;

    if (time->month == 2)
    {
        // February
        max_day = ((time->year & 3) == 0) ? 29 : 28;
    }
    else
    if ((time->month >= 1) & (time->month <= 7))
    {
        // January..July
        max_day = (time->month & 1) ? 31 : 30;
    }
    else
    if (time->month <= 12)
    {
        // August..December
        max_day = (time->month & 1) ? 30 : 31;
    }
    else
    {
        // Invalid month
        return 0;
    }

    if ((time->day < 1) || (time->day > max_day)) return 0;
    if (time->year > 99) return 0;
    if (time->hour > 23) return 0;
    if (time->minute > 59) return 0;
    if (time->second > 59) return 0;



    rtc_time_data[1] = bin_to_pbcd(time->second);   // Register 03h
    rtc_time_data[2] = bin_to_pbcd(time->minute);   // Register 04h
    rtc_time_data[3] = bin_to_pbcd(time->hour);     // Register 05h
    rtc_time_data[4] = bin_to_pbcd(time->day);      // Register 06h
    rtc_time_data[6] = bin_to_pbcd(time->month);    // Register 08h
    rtc_time_data[7] = bin_to_pbcd(time->year);     // Register 09h

    // Write the date & time data. The data block includes the start register
    // index value.
    return i2c_cb->write_fn(0,&rtc_time_write_io);
}







/*=============================================================================
8-bit I/O expander

NXP PCF8574

When using a pin as an input, you're supposed to set the output high (1), to
avoid damage to the chip in case 5V is connected to the pin.

I2C address: 0100111b
=============================================================================*/


static  U8              ioexp_word = 0;

static  I2C_WRITE_IO    ioexp_write_io = { &ioexp_word, 1, 0, I2C_AD_IOEXP };

static  I2C_READ_IO     ioexp_read_io = { &ioexp_word, 1, 0, I2C_AD_IOEXP };


static
FLAG  ioexp_write (U8 word)
{
    ioexp_word = word;
    return i2c_cb->write_fn(0,&ioexp_write_io);
}


static
FLAG  ioexp_read (U8 *word)
{
    FLAG    ok;

    ok = i2c_cb->read_fn(0,&ioexp_read_io);
    if (word) (*word) = ioexp_word;
    return ok;
}


/*=============================================================================
ADC

Maxim MAX11614EEE+

I2C address: 0110011b
=============================================================================*/


#define ADC_CNV_DATA    ABIOCARD_ADC_CNV_DATA


static  U8  adc_init_data[] =
{
    // Setup byte:
    // * Bit    7: REG=1: Setup byte.
    // * Bit 6..4: SEL[2..0]=101b: internal reference voltage turned on, REF
    //             not connected (note: the AbioCard decouples REF).
    // * Bit    3: CLK=0: Internal clock.
    // * Bit    2: BIP/UNI=0: Unipolar.
    // * Bit    1: /RST=0: No action.
    // * Bit    0: X=0: Don't care.
    0b11010000,

    // Configuration byte:
    // * Bit    7: REG=0: Configuration byte.
    // * Bit 6..5: SCAN[1..0]=00b: Scans up from AIN0 to the input selected by
    //             CS3–CS0.
    // * Bit 4..1: CS[3..0]=0111b: Scan up to AIN7.
    //             Note: CS3 is always set to zero for the MAX11614/MAX11615.
    // * Bit    0: SGL/DIF=1: Single-ended.
    0b00001111
};

static  I2C_WRITE_IO    adc_init_write_io = { (U8*)&adc_init_data, sizeof(adc_init_data), 0, I2C_AD_ADC };


static  I2C_READ_IO     adc_cnv_read_io = { 0, sizeof(ADC_CNV_DATA), 0, I2C_AD_ADC };


static
FLAG  adc_init (VOID)
{
    // Write the initialisation data
    return i2c_cb->write_fn(0,&adc_init_write_io);
}


static
FLAG  adc_convert (ADC_CNV_DATA *buf)
{
    adc_cnv_read_io.buf = (U8*)buf;

    return i2c_cb->read_fn(0,&adc_cnv_read_io);
}


/*=============================================================================
PWM

NXP PCA9635 LED driver

I2C address: 0001000b
=============================================================================*/


static  U8  pwm_init_data[] =
{
    // Control register:
    // * Bit 7..5: AI[2..0]=100b: Auto-increment all
    // * Bit 4..0: D[4..0]=00000b: Start register
    0b10000000,

    // MODE1:
    // * Bit 7..5: AI[2..0]=100b: Enable increment
    // * Bit    4: SLEEP=0: Enable oscillator
    // * Bit    3: SUB1
    // * Bit    2: SUB2
    // * Bit    1: SUB3
    // * Bit    0: ALCALL
    0b10000000,

    // MODE2:
    // * Bit 7..6: <reserved>
    // * Bit    5: DMBLNK
    // * Bit    4: INVRT
    // * Bit    3: OCH
    // * Bit    2: OUTDRV
    // * Bit 1..0: OUTNE[1..0]
    0b00010110
};

static  I2C_WRITE_IO    pwm_init_write_io = { (U8*)&pwm_init_data, sizeof(pwm_init_data), 0, I2C_AD_PWM };


static  U8  pwm_mode_2_data[] =
{
    // Control register:
    // * Bit 7..5: AI[2..0]=100b: Auto-increment all
    // * Bit 4..0: D[4..0]=10100b: Start register
    0b10010100,

    // LEDOUT[0..3]:
    // LDRx=00b: LED driver x is off (default power-up state).
    // LDRx=01b: LED driver x is fully on (individual brightness and group

    //           dimming/blinking not controlled).
    // LDRx=10b: LED driver x individual brightness can be controlled
    //           through its PWMx register.
    // LDRx=11b: LED driver x individual brightness and group dimming/
    //           blinking can be controlled through its PWMx register and
    //           the GRPPWM registers.
    0b10101010,     // LDR3 , LDR2 , LDR1 , LDR0
    0b10101010,     // LDR7 , LDR6 , LDR5 , LDR4
    0b10101010,     // LDR11, LDR10, LDR9 , LDR8
    0b10101010      // LDR15, LDR14, LDR13, LDR12
};

static  I2C_WRITE_IO    pwm_mode_2_write_io = { (U8*)&pwm_mode_2_data, sizeof(pwm_mode_2_data), 0, I2C_AD_PWM };


static  U8  pwm_mode_3_data[] =
{
    // Control register:
    // * Bit 7..5: AI[2..0]=100b: Auto-increment all
    // * Bit 4..0: D[4..0]=10100b: Start register
    0b10010100,

    // LEDOUT[0..3]:
    // LDRx=00b: LED driver x is off (default power-up state).
    // LDRx=01b: LED driver x is fully on (individual brightness and group
    //           dimming/blinking not controlled).
    // LDRx=10b: LED driver x individual brightness can be controlled
    //           through its PWMx register.
    // LDRx=11b: LED driver x individual brightness and group dimming/
    //           blinking can be controlled through its PWMx register and
    //           the GRPPWM registers.
    0b11111111,     // LDR3 , LDR2 , LDR1 , LDR0
    0b11111111,     // LDR7 , LDR6 , LDR5 , LDR4
    0b11111111,     // LDR11, LDR10, LDR9 , LDR8
    0b11111111      // LDR15, LDR14, LDR13, LDR12
};

static  I2C_WRITE_IO    pwm_mode_3_write_io = { (U8*)&pwm_mode_3_data, sizeof(pwm_mode_3_data), 0, I2C_AD_PWM };


static  U8  pwm_range_data[] =
{
    // Control register (set dynamically):
    // * Bit 7..5: AI[2..0]=100b: Auto-increment all
    // * Bit 4..0: D[4..0]=xxxxxb: Start register 2..18
    0b00000000,

    // Room for registers PWM[0..15] and GRPPWM
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00
};

static  I2C_WRITE_IO    pwm_range_write_io = { (U8*)&pwm_range_data, sizeof(pwm_range_data), 0, I2C_AD_PWM };

static  I2C_READ_IO     pwm_range_read_io = { 0, 0, 0, I2C_AD_PWM };


// Currently set PWM LED mode for all LED drivers:
// * =0: mode 3
// * =1: mode 2

static  FLAG    pwm_led_mode = 0;


static
FLAG  pwm_set_mode (FLAG mode)
{
    I2C_WRITE_IO   *write_io;

    pwm_led_mode = mode;

    write_io = (pwm_led_mode == 0) ? &pwm_mode_3_write_io
                                   : &pwm_mode_2_write_io;

    return i2c_cb->write_fn(0,write_io);
}


static
FLAG  pwm_write_range (U8 *buf, U8 start, U8 cnt)
{
    // Check the parameters
    if (start >= 17) return 0;
    if (cnt == 0) return 0;
    if (cnt > 17) return 0;
    if ((start + cnt) > 17) return 0;

    // Check whether the dim value is included
    if ((start + cnt) == 17)
    {
        FLAG    mode;

        mode = (buf[cnt-1] == 255) ? 1 : 0;

        if (mode != pwm_led_mode)
        {
            FLAG    ok;

            // Set the mode
            ok = pwm_set_mode(mode);
            if (!ok) return 0;
        }
    }

    pwm_range_data[0] = 0b10000010 + start;

    memcpy(&pwm_range_data[1],buf,cnt);

    pwm_range_write_io.len = cnt + 1;

    return i2c_cb->write_fn(0,&pwm_range_write_io);
}


static
FLAG  pwm_read_range (U8 *buf, U8 start, U8 cnt)
{
    FLAG    ok;
    U8      u;

    // Check the parameters
    if (start >= 17) return 0;
    if (cnt == 0) return 0;
    if (cnt > 17) return 0;
    if ((start + cnt) > 17) return 0;

    // Control register
    u = 0b10000010 + start;

    // Write the control register
    ok = i2c_cb->write_reg_fn(0,I2C_AD_PWM,u);
    if (!ok) return 0;

    pwm_range_read_io.buf = buf;
    pwm_range_read_io.len = cnt;

    return i2c_cb->read_fn(0,&pwm_range_read_io);
}


static  U8  pwm_dump_regs_data[28];

static  I2C_READ_IO     pwm_dump_regs_read_io = { (U8*)&pwm_dump_regs_data, sizeof(pwm_dump_regs_data), 0, I2C_AD_PWM };


static
FLAG  pwm_dump_regs (VOID)
{
    FLAG    ok;

    // Let's reuse this structure
    pwm_range_data[0] = 0b10000000;

    pwm_range_write_io.len = 1;

    ok = i2c_cb->write_fn(0,&pwm_range_write_io);
    if (!ok) return 0;

    ok = i2c_cb->read_fn(0,&pwm_dump_regs_read_io);
    if (!ok) return 0;

    dbg_hex_dump(pwm_dump_regs_data,sizeof(pwm_dump_regs_data),8);
    printf("\n");

    return 1;
}


static
FLAG  pwm_init (VOID)
{
    static  U8  dim;

    FLAG    mode;
    FLAG    ok;

    // Write the initialisation data
    ok = i2c_cb->write_fn(0,&pwm_init_write_io);
    if (!ok) return 0;

    // Read the current dim value (the GRPPWM register)
    ok = pwm_read_range(&dim,16,1);

    // Set the mode according to the current dim value
    mode = (dim == 255) ? 1 : 0;
    ok = pwm_set_mode(0);
    if (!ok) return 0;

    return 1;
}


/*=============================================================================
PWM2

NXP PCA9685 LED driver

Also capable of steering motors.

The code always set MODE1.AI to one thus auto-increment always applies.
 
I2C address: 1001000b
=============================================================================*/


static  U8  pwm2_init_data[] =
{
    // Control register:
    // * Bit 7..0: D[7..0]=00000000b: Start register
    0b00000000,

    // MODE1:
    // * Bit 7: RESTART=0: Don't clear
    // * Bit 4: EXTCLK=0: Use internal clock
    // * Bit 5: AI=1: Enable auto-increment
    // * Bit 4: SLEEP=0: Normal mode (oscillator on)
    // * Bit 3: SUB1=0: Don't respond to I2C subaddress 1
    // * Bit 2: SUB2=0: Don't respond to I2C subaddress 2
    // * Bit 1: SUB3=0: Don't respond to I2C subaddress 3
    // * Bit 0: ALLCALL=0: Don't respond to I2C address LED All Call
    0b00100000,

    // MODE2:
    // * Bit 7..5: <reserved>
    // * Bit    4: INVRT=0: Output logic state not inverted
    // * Bit    3: OCH=0: Ouputs change on I2C STOP command
    // * Bit    2: OUTDRV=1: The LED output are configured as a totem pole structure
    // * Bit 1..0: OUTNE[1..0]=10b: When /OE=1 the LED outputs are high-impedance
    0b00000110
};

static  I2C_WRITE_IO    pwm2_init_write_io = { (U8*)&pwm2_init_data, sizeof(pwm2_init_data), 0, I2C_AD_PWM2 };


static  U8  pwm2_range_data[] =
{
    // Control register (set dynamically):
    // * Bit 7..0: D[7..0]=xxxxxb: Start register
    0b00000000,

    // Room for registers 16 * 4 registers
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static  I2C_WRITE_IO    pwm2_range_write_io = { (U8*)&pwm2_range_data, 0, 0, I2C_AD_PWM2 };

static  I2C_READ_IO     pwm2_range_read_io = { (U8*)&pwm2_range_data, 0, 0, I2C_AD_PWM2 };


static
FLAG  pwm2_write_range (ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    U8     *dest;
    U8      u;
    U16     bytes;

    // Check the parameters
    if (start >= 16) return 0;
    if (cnt == 0) return 0;
    if (cnt > 16) return 0;
    if ((start + cnt) > 16) return 0;

    dest  = pwm2_range_data;
    bytes = 0;

    // Control register: start index 6 added with the index for the first channel
    dest[bytes++] = 0b00000110 + (start * 4);

    do
    {
        U8      val;

        // Register LEDx_ON_L
        val = buf->on_pos & 0xFF;
        dest[bytes++] = val;

        // Register LEDx_ON_H
        val = (buf->on_pos >> 8) & 0x0F;
        if (buf->always_on) val |= 0x10;
        dest[bytes++] = val;

        // Register LEDx_OFF_L
        val = buf->off_pos & 0xFF;
        dest[bytes++] = val;

        // Register LEDx_OFF_H
        val = (buf->off_pos >> 8) & 0x0F;
        if (buf->always_off) val |= 0x10;
        dest[bytes++] = val;

        buf++;
        cnt--;
    }
    while (cnt > 0);

    pwm2_range_write_io.len = bytes;

    return i2c_cb->write_fn(0,&pwm2_range_write_io);
}


static
FLAG  pwm2_read_range (ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    FLAG    ok;
    U8     *src;
    U8      u;

    // Check the parameters
    if (start >= 16) return 0;
    if (cnt == 0) return 0;
    if (cnt > 16) return 0;
    if ((start + cnt) > 16) return 0;

    // Control register: start index 6 added with the index for the first channel
    u = 0b00000110 + (start * 4);

    // Write the control register
    ok = i2c_cb->write_reg_fn(0,I2C_AD_PWM2,u);
    if (!ok) return 0;

    pwm2_range_read_io.len = cnt * 4;

    ok = i2c_cb->read_fn(0,&pwm2_range_read_io);
    if (!ok) return 0;

    src = pwm2_range_data;

    do
    {
        U16     val;

        // Registers LEDx_ON_L and LEDx_ON_H
        val = (*src);
        src++;
        val |= ((*src) << 8);
        src++;
        buf->on_pos = val & 0x0FFF;
        buf->always_on = (val & 0x1000) ? 1 : 0;

        // Register LEDx_OFF_L and LEDx_OFF_H
        val = (*src);
        src++;
        val |= ((*src) << 8);
        src++;
        buf->off_pos = val & 0x0FFF;
        buf->always_off = (val & 0x1000) ? 1 : 0;

        buf++;
        cnt--;
    }
    while (cnt > 0);

    return 1;
}


static  U8  pwm2_reg_data[] =
{
    // Control register (set dynamically):
    // * Bit 7..0: D[7..0]=xxxxxb: Start register
    0b00000000,

    // Register value
    0x00
};

static  I2C_WRITE_IO    pwm2_reg_write_io = { (U8*)&pwm2_reg_data, sizeof(pwm2_reg_data), 0, I2C_AD_PWM2 };


static
FLAG  pwm2_write_reg (U8 index, U8 val)
{
    pwm2_reg_data[0] = index;
    pwm2_reg_data[1] = val;

    // Write
    return i2c_cb->write_fn(0,&pwm2_reg_write_io);
}


static
FLAG  pwm2_read_reg (U8 index, U8 *val)
{
    FLAG    ok;

    // Write control register
    ok = i2c_cb->write_reg_fn(0,I2C_AD_PWM2,index);
    if (!ok) return 0;

    // Read register MODE1
    return i2c_cb->read_reg_fn(0,I2C_AD_PWM2,val);
}



static
FLAG  pwm2_write_prescaler (U8 val)
{
    FLAG    ok;
    U8      u;


    //
    // Go to sleep mode
    //


    // Read register MODE1
    ok = pwm2_read_reg(0,&u);
    if (!ok) return 0;

    // Set MODE1.SLEEP
    u |= 0b00010000;

    // Write register MODE1
    ok = pwm2_write_reg(0,u);
    if (!ok) return 0;


    //
    // Set the pre-scale value
    //


    // Write register PRE_SCALE
    ok = pwm2_write_reg(254,val);
    if (!ok) return 0;


    //
    // Restart
    //


    // Read register MODE1
    ok = pwm2_read_reg(0,&u);
    if (!ok) return 0;

    // If MODE1.RESTART is one, we've to perform some extra steps
    if (u & 0b10000000)
    {
        U8      v;

        // Clear flags MODE1.RESTART and MODE1.SLEEP
        v = u & ~0b10010000;

        // Write register MODE1
        ok = pwm2_write_reg(0,v);
        if (!ok) return 0;

        // Wait 500 us for the oscillator to stabilise. The SLEEP bit must be
        // logic 0 for at least 500 us, before a logic 1 is written into the
        // RESTART bit.
        usleep(500);
    }

    // Set MODE1.RESTART and clear MODE1.SLEEP
    u |=  0b10000000;
    u &= ~0b00010000;

    // Write register MODE1
    ok = pwm2_write_reg(0,u);
    if (!ok) return 0;

    return 1;
}


static
FLAG  pwm2_read_prescaler (U8 *val)
{
    // Read register PRE_SCALE
    return pwm2_read_reg(254,val);
}


// Registers 0..69

static  U8  pwm2_dump_regs_1_data[70];

static  I2C_READ_IO     pwm2_dump_regs_1_read_io = { (U8*)&pwm2_dump_regs_1_data, sizeof(pwm2_dump_regs_1_data), 0, I2C_AD_PWM2 };


// Registers 250..255

static  U8  pwm2_dump_regs_2_data[6];

static  I2C_READ_IO     pwm2_dump_regs_2_read_io = { (U8*)&pwm2_dump_regs_2_data, sizeof(pwm2_dump_regs_2_data), 0, I2C_AD_PWM2 };


static
FLAG  pwm2_dump_regs (VOID)
{
    FLAG    ok;

    // Write the start index to the control register
    ok = i2c_cb->write_reg_fn(0,I2C_AD_PWM2,0);
    if (!ok) return 0;

    // Read registers 0..69
    ok = i2c_cb->read_fn(0,&pwm2_dump_regs_1_read_io);
    if (!ok) return 0;

    // Write the start index to the control register
    ok = i2c_cb->write_reg_fn(0,I2C_AD_PWM2,250);
    if (!ok) return 0;

    // Read registers 250..255
    ok = i2c_cb->read_fn(0,&pwm2_dump_regs_2_read_io);
    if (!ok) return 0;

    printf("Registers 0..5:\n");
    dbg_hex_dump((U8*)&pwm2_dump_regs_1_data,6,8);
    printf("\n");

    printf("Registers 6..69:\n");
    dbg_hex_dump(((U8*)&pwm2_dump_regs_1_data) + 6,sizeof(pwm2_dump_regs_1_data) - 6,16);
    printf("\n");

    printf("Registers 250..255:\n");
    dbg_hex_dump((U8*)&pwm2_dump_regs_2_data,sizeof(pwm2_dump_regs_2_data),8);
    printf("\n");

    return 1;
}


static
FLAG  pwm2_init (VOID)
{
    FLAG    mode;
    FLAG    ok;

    // Write the initialisation data
    ok = i2c_cb->write_fn(0,&pwm2_init_write_io);
    if (!ok) return 0;

    //pwm2_dump_regs(p);

    return 1;
}


/*=============================================================================
AbioCard driver
=============================================================================*/


static  BSC_MEMIO_CTX   memio_gpio  = { 0x20200000, 4096, MAP_FAILED };

static  BSC_MEMIO_CTX   memio_bsc0  = { 0x20205000, 4096, MAP_FAILED };

static  BSC_MEMIO_CTX   memio_bsc1  = { 0x20804000, 4096, MAP_FAILED };

static  BSC_MEMIO_CTX  *memio_bsc   = 0;

static  int             mem_fd      = -1;

static  int             lock_fd     = -1;

static  int             i2cdev_fd   = -1;


static
FLAG  bsc_write_cb (POINTER *ctx, I2C_WRITE_IO *io)
{
    return bsc_write(memio_bsc,io);
}


static
FLAG  bsc_read_cb (POINTER *ctx, I2C_READ_IO *io)
{
    return bsc_read(memio_bsc,io);
}


static
FLAG  bsc_write_reg_cb (POINTER *ctx, U8 slave_ad, U8 reg)
{
    return bsc_write_reg(memio_bsc,slave_ad,reg);
}


static
FLAG  bsc_read_reg_cb (POINTER *ctx, U8 slave_ad, U8 *reg)
{
    return bsc_read_reg(memio_bsc,slave_ad,reg);
}


static  I2C_CB_TABLE    i2c_bsc_cb_table =
{
    .write_fn     = bsc_write_cb,
    .read_fn      = bsc_read_cb,

    .write_reg_fn = bsc_write_reg_cb,
    .read_reg_fn  = bsc_read_reg_cb
};


static
FLAG  i2cdev_write_cb (POINTER *ctx, I2C_WRITE_IO *io)
{
    int         res;
    ssize_t     xfrd;

    io->xfrd = 0;

    res = ioctl(i2cdev_fd,I2C_SLAVE,io->slave_ad);
    if (res < 0) return 0;

    xfrd = write(i2cdev_fd,io->buf,io->len);

    if (xfrd > 0) io->xfrd = (U16)xfrd;

    return (io->xfrd < io->len) ? 0 : 1;
}


static
FLAG  i2cdev_read_cb (POINTER *ctx, I2C_READ_IO *io)
{
    int         res;
    ssize_t     xfrd;

    io->xfrd = 0;

    res = ioctl(i2cdev_fd,I2C_SLAVE,io->slave_ad);
    if (res < 0) return 0;

    xfrd = read(i2cdev_fd,io->buf,io->len);

    if (xfrd > 0) io->xfrd = (U16)xfrd;

    return (io->xfrd < io->len) ? 0 : 1;
}


static
FLAG  i2cdev_write_reg_cb (POINTER *ctx, U8 slave_ad, U8 reg)
{
    int         res;
    ssize_t     xfrd;

    xfrd = 0;

    res = ioctl(i2cdev_fd,I2C_SLAVE,slave_ad);
    if (res < 0) return 0;

    xfrd = write(i2cdev_fd,&reg,1);

    return (xfrd == 1) ? 1 : 0;
}


static
FLAG  i2cdev_read_reg_cb (POINTER *ctx, U8 slave_ad, U8 *reg)
{
    int         res;
    ssize_t     xfrd;

    xfrd = 0;

    res = ioctl(i2cdev_fd,I2C_SLAVE,slave_ad);
    if (res < 0) return 0;

    xfrd = read(i2cdev_fd,reg,1);

    return (xfrd == 1) ? 1 : 0;
}


static  I2C_CB_TABLE    i2c_dev_cb_table =
{
    .write_fn     = i2cdev_write_cb,
    .read_fn      = i2cdev_read_cb,
    .write_reg_fn = i2cdev_write_reg_cb,
    .read_reg_fn  = i2cdev_read_reg_cb

};


FLAG  abiocard_rtc_poll (ABIOCARD_RTC_POLL_INFO *info)
{
    return rtc_poll(info);
}


FLAG  abiocard_rtc_set_time (ABIOCARD_RTC_TIME *time)
{
    return rtc_set_time(time);
}


FLAG  abiocard_ioexp_write (U8 word)
{
    return ioexp_write(word);
}


FLAG  abiocard_ioexp_read (U8 *word)
{
    return ioexp_read(word);
}


FLAG  abiocard_adc_convert (ABIOCARD_ADC_CNV_DATA *buf)
{
    return adc_convert(buf);
}


FLAG  abiocard_pwm_write_range (U8 *buf, U8 start, U8 cnt)
{
    return pwm_write_range(buf,start,cnt);
}


FLAG  abiocard_pwm_read_range (U8 *buf, U8 start, U8 cnt)
{
    return pwm_read_range(buf,start,cnt);
}


FLAG  abiocard_pwm2_write_range (ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    return pwm2_write_range(buf,start,cnt);
}


FLAG  abiocard_pwm2_read_range (ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    return pwm2_read_range(buf,start,cnt);
}


FLAG  abiocard_pwm2_write_prescaler (U8 val)
{
    return pwm2_write_prescaler(val);
}


FLAG  abiocard_pwm2_read_prescaler (U8 *val)
{
    return pwm2_read_prescaler(val);
}


VOID  abiocard_deinit (VOID)
{
    bsc_memio_deinit(memio_bsc);
    bsc_memio_deinit(&memio_gpio);

    if (i2cdev_fd != -1)
    {
        close(i2cdev_fd);
        i2cdev_fd = -1;
    }

    if (mem_fd != -1)
    {
        close(mem_fd);
        mem_fd = -1;
    }

    if (lock_fd != -1)
    {
        struct  flock   fl;

        fl.l_type   = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start  = 0;
        fl.l_len    = 0;

        if (fcntl(lock_fd,F_SETLK,&fl) == -1)
            printf("Unlock failed, errno %d\n",errno);

        close(lock_fd);
        lock_fd = -1;
    }

    memio_bsc = 0;
    i2c_cb    = 0;
}


FLAG  abiocard_init (ABIOCARD_INIT_IO *io)
{
    U32             u;
    U8              ioexp_word;
    int             i;
    FLAG            ok;
    struct  flock   fl;
    U8              dev_cnt;


    io->err           = ABIOCARD_INIT_ERR_NONE;
    io->rtc_present   = 0;
    io->ioexp_present = 0;
    io->adc_present   = 0;
    io->pwm_present   = 0;
    io->pwm2_present  = 0;


    if (io->intf == ABIOCARD_INIT_INTF_BSC)
    {
        // Check the given index of the BSC controller
        if (io->bsc_index == 0) memio_bsc = &memio_bsc0; else
        if (io->bsc_index == 1) memio_bsc = &memio_bsc1; else
        {
            printf("Invalid BSC controller index\n");

            io->err = ABIOCARD_INIT_ERR_PARAM;
            goto Err;
        }


        // Put a lock file in the /var/lock section of the file system.
        //
        lock_fd = open("/var/lock/abiocard.lock",O_WRONLY|O_CREAT,0666);
        if (lock_fd == -1)
        {
            printf("Can't create or open lock file, errno %d\n",errno);
            goto Err_Chk_ErrNo;
        }

        // Try to lock the file for write access. If this step succeeds then this
        // module wins exclusive access to the abiocard. If not, another module has
        // already taken ownership over the abiocard.

        fl.l_type   = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start  = 0;
        fl.l_len    = 0;

        i = fcntl(lock_fd,F_SETLK,&fl);
        if (i == -1)
        {
            printf("Can't lock the lock file, errno %d\n",errno);
            io->err = ABIOCARD_INIT_ERR_LOCKED;
            goto Err;
        }


        mem_fd = open("/dev/mem",O_RDWR|O_SYNC);
        if (mem_fd == -1)
        {
            printf("Can't open /dev/mem, errno %d\n",errno);
            goto Err_Chk_ErrNo;
        }

        ok = bsc_memio_init(&memio_gpio,mem_fd);
        if (!ok)
        {
            printf("Can't map GPIO, errno %d\n",errno);
            goto Err_Chk_ErrNo;
        }

        ok = bsc_memio_init(memio_bsc,mem_fd);
        if (!ok)
        {
            printf("Can't map BSC, errno %d\n",errno);
            goto Err_Chk_ErrNo;
        }

        // Set up the BSC controller
        //
        // Not relevant but nice to know: FSELx=000b sets a pin to general-purpose
        // INPUT, FSELx=001b sets a pin to general-purpose OUTPUT.
        //
        if (io->bsc_index == 0)
        {
            // Set up BSC0:
            // * FSEL0=100b (ALT0): route SDA0 to pin GPIO0, pull high
            // * FSEL1=100b (ALT0): route SCL0 to pin GPIO1, pull high
            //
            u = memio_gpio.va[GPIO_REG_GPFSEL0];
            u &= ~0b111111;
            u |=  0b100100;
            memio_gpio.va[GPIO_REG_GPFSEL0] = u;
        }
        else
        {
            // Set up BSC1:
            // * FSEL2=100b (ALT0): route SDA1 to pin GPIO0, pull high
            // * FSEL3=100b (ALT0): route SCL1 to pin GPIO1, pull high
            //
            u = memio_gpio.va[GPIO_REG_GPFSEL0];
            u &= ~0b111111 << 6;
            u |=  0b100100 << 6;
            memio_gpio.va[GPIO_REG_GPFSEL0] = u;
        }

        //printf("GPIO va %p\n",memio_gpio.va);
        //printf("BSC  va %p\n",memio_bsc->va);

        i2c_cb = &i2c_bsc_cb_table;
    }
    else
    if (io->intf == ABIOCARD_INIT_INTF_I2CDEV)
    {
        i2cdev_fd = open(io->i2cdev_name,O_RDWR);
        if (i2cdev_fd == -1)
        {
            printf("Can't open %s, errno %d\n",io->i2cdev_name,errno);
            goto Err_Chk_ErrNo;
        }

        i2c_cb = &i2c_dev_cb_table;
    }
    else
    {
        printf("Invalid interface value\n");

        io->err = ABIOCARD_INIT_ERR_PARAM;
        goto Err;
    }


    // Probe and initialise peripherals on the AbioCard

    dev_cnt = 0;

    ok = rtc_probe();
    if (ok)
    {
        io->rtc_present = 1;
        dev_cnt++;
    }

    ok = ioexp_read(&ioexp_word);
    if (ok)
    {
        io->ioexp_present = 1;
        dev_cnt++;
    }

    ok = adc_init();
    if (ok)
    {
        io->adc_present = 1;
        dev_cnt++;
    }

    ok = pwm_init();
    if (ok)
    {
        io->pwm_present = 1;
        dev_cnt++;
    }

    ok = pwm2_init();
    if (ok)
    {
        io->pwm2_present = 1;
        dev_cnt++;
    }

    if (dev_cnt == 0)
    {
        io->err = ABIOCARD_INIT_ERR_NO_CARD;
        goto Err;
    }

    return 1;

  Err_Chk_ErrNo:

    if (errno == EACCES)
        io->err = ABIOCARD_INIT_ERR_NO_PERM;
    else

        io->err = ABIOCARD_INIT_ERR_OTHER;

  Err:
  
    abiocard_deinit();

    return 0;
}

