
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocard.c
//
// AbioCard driver.
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
//   2013-09-17  Peter S'heeren, Axiris
//
//      * Added allocation of a context and mapping to handle.
//      * Moved i2c-dev and BSC back-ends to separate modules.
//
//   2014-07-07  Peter S'heeren, Axiris
//
//      * Adapted to use the new non-blocking I2CBUS interface.
//      * Added detached state.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2012-2014  Peter S'heeren, Axiris
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "abiocard.h"
#include "osrtl.h"


/*=============================================================================
Context
=============================================================================*/


// Forward declarations

typedef struct  _CTX            CTX;


// Maximum number of I2C transfers scheduled simultaneously

#define XFR_CNT     2


struct  _CTX
{
    I2CBUS_INTF        *i2cbus_intf;

    I2CBUS_XFR         *xfr_list[XFR_CNT];

    // Currently set PWM LED mode for all LED drivers:
    // * =0: mode 3
    // * =1: mode 2
    FLAG                pwm_led_mode;

    FLAG                detached;
};


// I2C addresses of all supported chips

#define I2C_AD_RTC          0x51    // 0b1010001
#define I2C_AD_IOEXP        0x27    // 0b0100111
#define I2C_AD_ADC          0x33    // 0b0110011
#define I2C_AD_PWM          0x08    // 0b0001000
#define I2C_AD_PWM2         0x48    // 0b1001000


/*=============================================================================
Blocking transfer I/O with non-blocking I2C interface
=============================================================================*/


static
FLAG  I2C_Xfr_Sync (CTX *ctx, U32 xfr_cnt)
{
    U32                 start_ticks;
    U32                 delta_ticks;
    U32                 xfr_index;
    I2CBUS_XFR         *xfr;
    I2CBUS_XFR_STATUS   st;
    FLAG                pending;
    FLAG                ok;


    // Schedule the transfer(s)

    for (xfr_index = 0; xfr_index < xfr_cnt; xfr_index++)
    {
        ok = ctx->i2cbus_intf->fn_table->xfr_schedule_fn(ctx->i2cbus_intf,ctx->xfr_list[xfr_index]);
        if (!ok) goto Err;
    }


    // Wait for completion of the transfer(s)

    start_ticks = OSRTL_GetTickCount();

    for (;;)
    {
        pending = 0;
        for (xfr_index = 0; xfr_index < xfr_cnt; xfr_index++)
        {
            st = ctx->i2cbus_intf->fn_table->xfr_get_status_fn(ctx->i2cbus_intf,ctx->xfr_list[xfr_index]);
            if (st == I2CBUS_XFR_STATUS_SCHEDULED)
            {
                pending = 1;
                break;
            }
        }

        if (!pending) break;

        OSRTL_Yield();

        // Time out after a certain amount of milliseconds.
        //
        // Important: Do not disable the timeout. In case of a removal of the
        // device (for example, AxiCat with AbioCard), the timeout will be the
        // only catch for the removal event.

        delta_ticks = OSRTL_GetTickCount() - start_ticks;

        if (delta_ticks > 2000) goto Err;
    }


    // Check the results of the completed transfer(s)

    for (xfr_index = 0; xfr_index < xfr_cnt; xfr_index++)
    {
        // Transfer object
        xfr = ctx->xfr_list[xfr_index];

        // All bytes must have been transferred
        if (xfr->xfrd < xfr->sc) return 0;

        // If data bytes were written, the slave must have responded ACK to all
        // data bytes
        if ((xfr->dir == I2CBUS_DIR_WRITE) && (xfr->nack_rsp)) return 0;
    }

    return 1;


  Err:

    // Cancel the transfer(s)
    for (xfr_index = 0; xfr_index < xfr_cnt; xfr_index++)
    {
        ctx->i2cbus_intf->fn_table->xfr_cancel_fn(ctx->i2cbus_intf,ctx->xfr_list[xfr_index]);
    }

    // Mark the device as detached
    ctx->detached = 1;

    return 0;
}


static
FLAG  I2C_Write_Block (CTX *ctx, U8 slave_ad, U8 *buf, U32 cnt)
{
    I2CBUS_XFR     *xfr;

    // Transfer: START, SLA+W, bytes, STOP

    xfr = ctx->xfr_list[0];

    xfr->buf        = buf;
    xfr->sc         = cnt;
    xfr->dir        = I2CBUS_DIR_WRITE;
    xfr->force_stop = 1;
    xfr->slave_ad   = slave_ad;

    return I2C_Xfr_Sync(ctx,1);
}


static
FLAG  I2C_Write_Reg (CTX *ctx, U8 slave_ad, U8 reg)
{
    // Transfer: START, SLA+W, register, STOP

    return I2C_Write_Block(ctx,slave_ad,&reg,1);
}


static
FLAG  I2C_Read_Block (CTX *ctx, U8 slave_ad, U8 *buf, U32 cnt)
{
    I2CBUS_XFR     *xfr;

    // Transfer: START, SLA+R, bytes, STOP

    xfr = ctx->xfr_list[0];

    xfr->buf        = buf;
    xfr->sc         = cnt;
    xfr->dir        = I2CBUS_DIR_READ;
    xfr->force_stop = 1;
    xfr->slave_ad   = slave_ad;

    return I2C_Xfr_Sync(ctx,1);
}


static
FLAG  I2C_Write_Reg_Read_Block (CTX *ctx, U8 slave_ad, U8 reg, U8 *buf, U32 cnt)
{
    I2CBUS_XFR     *xfr;

    // Transfer: START, SLA+W, register, STOP

    xfr = ctx->xfr_list[0];

    xfr->buf        = &reg;
    xfr->sc         = 1;
    xfr->dir        = I2CBUS_DIR_WRITE;
    xfr->force_stop = 1;
    xfr->slave_ad   = slave_ad;

    // Transfer: START, SLA+R, bytes, STOP

    xfr = ctx->xfr_list[1];

    xfr->buf        = buf;
    xfr->sc         = cnt;
    xfr->dir        = I2CBUS_DIR_READ;
    xfr->force_stop = 1;
    xfr->slave_ad   = slave_ad;

    return I2C_Xfr_Sync(ctx,2);
}


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
    // -> 0b00000000
    0x00,

    // Register 01h - Control_2:
    // * Bit 7: MSF - minute or second interrupt generated (write zero to clear)
    // * Bit 6: WDTF - watchdog timer interrupt or reset generated
    // * Bit 5: TSF2 - timestamp 2 interrupt generated y/n (write zero to clear)
    // * Bit 4: AF - alarm interrupt generated (write zero to clear)
    // * Bit 3: Unused
    // * Bit 2: TSIE - Timestamp interrupt enable
    // * Bit 1: AIE - Alarm interrupt enable
    // * Bit 0: Unused
    // -> 0b00000000
    0x00,

    // Register 02h - Control_3:
    // * Bit 7..5: PWRMNG[2..0] - power management setting
    // * Bit    4: BTSE - enable timestamp when battery switch-over y/n
    // * Bit    3: BF - set when battery switch-over occurs (write zero to clear)
    // * Bit    2: BLF - Battery low y/n
    // * Bit    1: BIE - interrupt generated when BF is set
    // * Bit    0: BLIE - interrupt generated when BLF is set
    // -> 0b00000000
    0x00
};


static
U8  pbcd_to_bin (U8 pbcd)
{
    return (((pbcd >> 4) * 10) + (pbcd & 15));
}


static
U8  bin_to_pbcd (U8 bin)
{
    return (bin % 10) | ((bin / 10) << 4);
}


static
FLAG  rtc_probe (CTX *ctx)
{
    // Set the start register to zero (for probing purposes only)
    return I2C_Write_Reg(ctx,I2C_AD_RTC,0);
}


static
FLAG  rtc_poll (CTX *ctx, RTC_POLL_INFO *info)
{
    U8      rtc_poll_buf[10];
    FLAG   ok;


    // Read registers 00h..09h
    ok = I2C_Write_Reg_Read_Block(ctx,I2C_AD_RTC,0,rtc_poll_buf,sizeof(rtc_poll_buf));
    if (!ok) return 0;

    // Control_3 register, bit 2: BLF
    info->battery_low = (rtc_poll_buf[2] & 0x04) ? 1 : 0;

    // Check the OSF flag
    if (rtc_poll_buf[3] & 0x80)
    {
        // Mark power-up detected
        info->power_up = 1;

        // Resulting date and time - clear
        memset(&info->time,0,sizeof(RTC_TIME));

        // Write the initialization data. The data block includes the start
        // register index value.
        ok = I2C_Write_Block(ctx,I2C_AD_RTC,rtc_init_data,sizeof(rtc_init_data));
        if (!ok) return 0;
    }
    else
    {
        // Power-up not detected
        info->power_up = 0;

        // Date and time
        info->time.second = pbcd_to_bin((U8)(rtc_poll_buf[3] & 0x7F));
        info->time.minute = pbcd_to_bin((U8)(rtc_poll_buf[4] & 0x7F));
        info->time.hour   = pbcd_to_bin((U8)(rtc_poll_buf[5] & 0x3F));
        info->time.day    = pbcd_to_bin((U8)(rtc_poll_buf[6] & 0x3F));
        info->time.month  = pbcd_to_bin((U8)(rtc_poll_buf[8] & 0x1F));
        info->time.year   = pbcd_to_bin(rtc_poll_buf[9]);
    }

    return 1;
}


static
FLAG  rtc_set_time (CTX *ctx, RTC_TIME *time)
{
    U8      rtc_time_data[8];
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

    rtc_time_data[0] = 3;                           // Start register index
    rtc_time_data[1] = bin_to_pbcd(time->second);   // Register 03h
    rtc_time_data[2] = bin_to_pbcd(time->minute);   // Register 04h
    rtc_time_data[3] = bin_to_pbcd(time->hour);     // Register 05h
    rtc_time_data[4] = bin_to_pbcd(time->day);      // Register 06h
    rtc_time_data[5] = 0;                           // Register 07h: Day of the week
    rtc_time_data[6] = bin_to_pbcd(time->month);    // Register 08h
    rtc_time_data[7] = bin_to_pbcd(time->year);     // Register 09h

    // Write the date & time data. The data block includes the start register
    // index value.
    return I2C_Write_Block(ctx,I2C_AD_RTC,rtc_time_data,sizeof(rtc_time_data));
}


/*=============================================================================
8-bit I/O expander

NXP PCF8574

When using a pin as an input, you're supposed to set the output high (1), to
avoid damage to the chip in case 5V is connected to the pin.

I2C address: 0100111b
=============================================================================*/


static
FLAG  ioexp_write (CTX *ctx, U8 word)
{
    return I2C_Write_Reg(ctx,I2C_AD_IOEXP,word);
}


static
FLAG  ioexp_read (CTX *ctx, U8 *word)
{
    U8      ioexp_word;
    FLAG    ok;

    ok = I2C_Read_Block(ctx,I2C_AD_IOEXP,&ioexp_word,1);
    if (!ok) return 0;

    // Report the result and return successfully
    if (word) (*word) = ioexp_word;
    return 1;
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
    // -> 0b11010000
    0xD0,

    // Configuration byte:
    // * Bit    7: REG=0: Configuration byte.
    // * Bit 6..5: SCAN[1..0]=00b: Scans up from AIN0 to the input selected by
    //             CS3–CS0.
    // * Bit 4..1: CS[3..0]=0111b: Scan up to AIN7.
    //             Note: CS3 is always set to zero for the MAX11614/MAX11615.
    // * Bit    0: SGL/DIF=1: Single-ended.
    // -> 0b00001111
    0x0F
};


static
FLAG  adc_init (CTX *ctx)
{
    return I2C_Write_Block(ctx,I2C_AD_ADC,adc_init_data,sizeof(adc_init_data));
}


static
FLAG  adc_convert (CTX *ctx, ADC_CNV_DATA *buf)
{
    return I2C_Read_Block(ctx,I2C_AD_ADC,(U8*)buf,sizeof(ADC_CNV_DATA));
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
    // -> 0b10000000
    0x80,

    // MODE1:
    // * Bit 7..5: AI[2..0]=100b: Enable increment
    // * Bit    4: SLEEP=0: Enable oscillator
    // * Bit    3: SUB1
    // * Bit    2: SUB2
    // * Bit    1: SUB3
    // * Bit    0: ALCALL
    // -> 0b10000000
    0x80,

    // MODE2:
    // * Bit 7..6: <reserved>
    // * Bit    5: DMBLNK
    // * Bit    4: INVRT
    // * Bit    3: OCH
    // * Bit    2: OUTDRV
    // * Bit 1..0: OUTNE[1..0]
    // -> 0b00010110
    0x16
};


static  U8  pwm_mode_2_data[] =
{
    // Control register:
    // * Bit 7..5: AI[2..0]=100b: Auto-increment all
    // * Bit 4..0: D[4..0]=10100b: Start register
    // -> 0b10010100
    0x94,

    // LEDOUT[0..3]:
    // LDRx=00b: LED driver x is off (default power-up state).
    // LDRx=01b: LED driver x is fully on (individual brightness and group

    //           dimming/blinking not controlled).
    // LDRx=10b: LED driver x individual brightness can be controlled
    //           through its PWMx register.
    // LDRx=11b: LED driver x individual brightness and group dimming/
    //           blinking can be controlled through its PWMx register and
    //           the GRPPWM registers.
    0xAA,   // 0b10101010 - LDR3 , LDR2 , LDR1 , LDR0
    0xAA,   // 0b10101010 - LDR7 , LDR6 , LDR5 , LDR4
    0xAA,   // 0b10101010 - LDR11, LDR10, LDR9 , LDR8
    0xAA    // 0b10101010 - LDR15, LDR14, LDR13, LDR12
};


static  U8  pwm_mode_3_data[] =
{
    // Control register:
    // * Bit 7..5: AI[2..0]=100b: Auto-increment all
    // * Bit 4..0: D[4..0]=10100b: Start register
    // -> 0b10010100
    0x94,

    // LEDOUT[0..3]:
    // LDRx=00b: LED driver x is off (default power-up state).
    // LDRx=01b: LED driver x is fully on (individual brightness and group
    //           dimming/blinking not controlled).
    // LDRx=10b: LED driver x individual brightness can be controlled
    //           through its PWMx register.
    // LDRx=11b: LED driver x individual brightness and group dimming/
    //           blinking can be controlled through its PWMx register and
    //           the GRPPWM registers.
    0xFF,   // 0b11111111 - LDR3 , LDR2 , LDR1 , LDR0
    0xFF,   // 0b11111111 - LDR7 , LDR6 , LDR5 , LDR4
    0xFF,   // 0b11111111 - LDR11, LDR10, LDR9 , LDR8
    0xFF    // 0b11111111 - LDR15, LDR14, LDR13, LDR12
};


static
FLAG  pwm_set_mode (CTX *ctx, FLAG mode)
{
    U8     *buf;
    U32     cnt;

    // Store the mode in the context
    ctx->pwm_led_mode = mode;

    if (mode == 0)
    {
        buf = pwm_mode_3_data;
        cnt = sizeof(pwm_mode_3_data);
    }
    else
    {
        buf = pwm_mode_2_data;
        cnt = sizeof(pwm_mode_2_data);
    }

    return I2C_Write_Block(ctx,I2C_AD_PWM,buf,cnt);
}


static
FLAG  pwm_write_range (CTX *ctx, U8 *buf, U8 start, U8 cnt)
{
    // I2C write buffer:
    // +00: Control register:
    //      * Bit 7..5: AI[2..0]=100b: Auto-increment all
    //      * Bit 4..0: D[4..0]=xxxxxb: Start register 2..18
    // +01: Room for registers PWM[0..15] and GRPPWM
    // +18: ---
    //
    U8      pwm_range_data[1+16+1];

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

        if (mode != ctx->pwm_led_mode)
        {
            FLAG    ok;

            // Set the mode
            ok = pwm_set_mode(ctx,mode);
            if (!ok) return 0;
        }
    }

    pwm_range_data[0] = 0x82 + start;

    memcpy(&pwm_range_data[1],buf,cnt);

    return I2C_Write_Block(ctx,I2C_AD_PWM,pwm_range_data,cnt + 1);
}


static
FLAG  pwm_read_range (CTX *ctx, U8 *buf, U8 start, U8 cnt)
{
    U8      reg;

    // Check the parameters
    if (start >= 17) return 0;
    if (cnt == 0) return 0;
    if (cnt > 17) return 0;
    if ((start + cnt) > 17) return 0;

    // Control register:
    // * Bit 7..5: AI[2..0]=100b: Auto-increment all
    // * Bit 4..0: D[4..0]=xxxxxb: Start register 2..18
    //
    reg = 0x82 + start;

    return I2C_Write_Reg_Read_Block(ctx,I2C_AD_PWM,reg,buf,cnt);
}


static
FLAG  pwm_init (CTX *ctx)
{
    U8      dim;
    FLAG    mode;
    FLAG    ok;

    // Write the initialization data
    ok = I2C_Write_Block(ctx,I2C_AD_PWM,pwm_init_data,sizeof(pwm_init_data));
    if (!ok) return 0;

    // Read the current dim value (the GRPPWM register)
    ok = pwm_read_range(ctx,&dim,16,1);

    // Set the mode according to the current dim value
    mode = (dim == 255) ? 1 : 0;
    ok = pwm_set_mode(ctx,mode);
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
    // -> 0b00000000
    0x00,

    // MODE1:
    // * Bit 7: RESTART=0: Don't clear
    // * Bit 4: EXTCLK=0: Use internal clock
    // * Bit 5: AI=1: Enable auto-increment
    // * Bit 4: SLEEP=0: Normal mode (oscillator on)
    // * Bit 3: SUB1=0: Don't respond to I2C subaddress 1
    // * Bit 2: SUB2=0: Don't respond to I2C subaddress 2
    // * Bit 1: SUB3=0: Don't respond to I2C subaddress 3
    // * Bit 0: ALLCALL=0: Don't respond to I2C address LED All Call
    // -> 0b00100000
    0x20,

    // MODE2:
    // * Bit 7..5: <reserved>
    // * Bit    4: INVRT=0: Output logic state not inverted
    // * Bit    3: OCH=0: Ouputs change on I2C STOP command
    // * Bit    2: OUTDRV=1: The LED output are configured as a totem pole structure
    // * Bit 1..0: OUTNE[1..0]=10b: When /OE=1 the LED outputs are high-impedance
    // -> 0b00000110
    0x06
};


static
FLAG  pwm2_write_range (CTX *ctx, ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    // I2C write buffer:
    // +00: Control register:
    //      * Bit 7..0: D[7..0]=xxxxxxxxb: Start register
    // +01: Room for registers 16 * 4 registers
    // +65: ---
    //
    U8      pwm2_range_data[1+64];

    U8     *dest;
    U16     bytes;

    // Check the parameters
    if (start >= 16) return 0;
    if (cnt == 0) return 0;
    if (cnt > 16) return 0;
    if ((start + cnt) > 16) return 0;

    dest  = pwm2_range_data;
    bytes = 0;

    // Control register: start index 6 added with the index for the first channel
    dest[bytes++] = 0x06 + (start * 4);

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

    // Write the PWM data
    return I2C_Write_Block(ctx,I2C_AD_PWM2,pwm2_range_data,bytes);
}


static
FLAG  pwm2_read_range (CTX *ctx, ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    // +00: Control register:
    //      * Bit 7..0: D[7..0]=xxxxxxxxb: Start register
    // +01: Room for registers 16 * 4 registers
    // +65: ---
    //
    U8      pwm2_range_data[1+64];

    U8     *src;
    U8      reg;
    FLAG    ok;

    // Check the parameters
    if (start >= 16) return 0;
    if (cnt == 0) return 0;
    if (cnt > 16) return 0;
    if ((start + cnt) > 16) return 0;

    // Control register: start index 6 added with the index for the first channel
    reg = 0x06 + (start * 4);

    ok = I2C_Write_Reg_Read_Block(ctx,I2C_AD_PWM2,reg,pwm2_range_data,cnt * 4);
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



static
FLAG  pwm2_write_prescaler (CTX *ctx, U8 val)
{
    FLAG    ok;
    U8      buf[2];
    U8      u;


    //
    // Go to sleep mode
    //


    // Read register MODE1
    ok = I2C_Write_Reg_Read_Block(ctx,I2C_AD_PWM2,0,&u,1);
    if (!ok) return 0;

    // Set MODE1.SLEEP (bit 4)
    u |= 0x10;

    // Write register MODE1
    buf[0] = 0;
    buf[1] = u;
    ok = I2C_Write_Block(ctx,I2C_AD_PWM2,buf,2);
    if (!ok) return 0;


    //
    // Set the pre-scale value
    //


    // Write register PRE_SCALE
    buf[0] = 254;
    buf[1] = val;
    ok = I2C_Write_Block(ctx,I2C_AD_PWM2,buf,2);
    if (!ok) return 0;


    //
    // Restart
    //


    // Read register MODE1
    ok = I2C_Write_Reg_Read_Block(ctx,I2C_AD_PWM2,0,&u,1);
    if (!ok) return 0;

    // If MODE1.RESTART (bit 7) is one, we've to perform some extra steps
    if (u & 0x80)
    {
        U8      v;

        // Clear flags MODE1.RESTART (bit 7) and MODE1.SLEEP (bit 4)
        v = u & ~0x90;

        // Write register MODE1
        buf[0] = 0;
        buf[1] = v;
        ok = I2C_Write_Block(ctx,I2C_AD_PWM2,buf,2);
        if (!ok) return 0;

        // Wait 500 us for the oscillator to stabilize. The SLEEP bit must be
        // logic 0 for at least 500 us, before a logic 1 is written into the
        // RESTART bit.
        OSRTL_SleepMs(500);
    }

    // Set MODE1.RESTART (bit 7) and clear MODE1.SLEEP (bit 4)
    u |=  0x80;
    u &= ~0x10;

    // Write register MODE1
    buf[0] = 0;
    buf[1] = u;
    ok = I2C_Write_Block(ctx,I2C_AD_PWM2,buf,2);
    if (!ok) return 0;

    return 1;
}


static
FLAG  pwm2_read_prescaler (CTX *ctx, U8 *val)
{
    // Read register PRE_SCALE
    return I2C_Write_Reg_Read_Block(ctx,I2C_AD_PWM2,254,val,1);
}


static
FLAG  pwm2_init (CTX *ctx)
{
    FLAG    ok;

    // Write the initialization data
    ok = I2C_Write_Block(ctx,I2C_AD_PWM2,pwm2_init_data,sizeof(pwm2_init_data));
    if (!ok) return 0;

    //pwm2_dump_regs(p);

    return 1;
}


/*=============================================================================
AbioCard driver
=============================================================================*/


FLAG  abiocard_rtc_poll (ABIOCARD_HANDLE handle, ABIOCARD_RTC_POLL_INFO *info)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return rtc_poll(ctx,info);
}


FLAG  abiocard_rtc_set_time (ABIOCARD_HANDLE handle, ABIOCARD_RTC_TIME *time)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return rtc_set_time(ctx,time);
}


FLAG  abiocard_ioexp_write (ABIOCARD_HANDLE handle, U8 word)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return ioexp_write(ctx,word);
}


FLAG  abiocard_ioexp_read (ABIOCARD_HANDLE handle, U8 *word)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return ioexp_read(ctx,word);
}


FLAG  abiocard_adc_convert (ABIOCARD_HANDLE handle, ABIOCARD_ADC_CNV_DATA *buf)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return adc_convert(ctx,buf);
}


FLAG  abiocard_pwm_write_range (ABIOCARD_HANDLE handle, U8 *buf, U8 start, U8 cnt)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return pwm_write_range(ctx,buf,start,cnt);
}


FLAG  abiocard_pwm_read_range (ABIOCARD_HANDLE handle, U8 *buf, U8 start, U8 cnt)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return pwm_read_range(ctx,buf,start,cnt);
}


FLAG  abiocard_pwm2_write_range (ABIOCARD_HANDLE handle, ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return pwm2_write_range(ctx,buf,start,cnt);
}


FLAG  abiocard_pwm2_read_range (ABIOCARD_HANDLE handle, ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return pwm2_read_range(ctx,buf,start,cnt);
}


FLAG  abiocard_pwm2_write_prescaler (ABIOCARD_HANDLE handle, U8 val)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return pwm2_write_prescaler(ctx,val);
}


FLAG  abiocard_pwm2_read_prescaler (ABIOCARD_HANDLE handle, U8 *val)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return pwm2_read_prescaler(ctx,val);
}


FLAG  abiocard_is_detached (ABIOCARD_HANDLE handle)
{
    CTX    *ctx;

    // Init
    ctx = (CTX*)handle;

    return ctx->detached;
}


static
VOID  deinit (CTX *ctx)
{
    U32     u;

    for (u = 0; u < XFR_CNT; u++)
    {
        ctx->i2cbus_intf->fn_table->xfr_destroy_fn(ctx->i2cbus_intf,ctx->xfr_list[u]);
    }

    free(ctx);
}


VOID  abiocard_deinit (ABIOCARD_HANDLE handle)
{
    CTX    *ctx;

    if (!handle) return;

    // Init
    ctx = (CTX*)handle;

    deinit(ctx);
}


ABIOCARD_HANDLE  abiocard_init (ABIOCARD_INIT_IO *io)
{
    CTX    *ctx;
    U8      dev_cnt;
    U32     u;
    FLAG    ok;


    io->err           = ABIOCARD_INIT_ERR_NONE;
    io->rtc_present   = 0;
    io->ioexp_present = 0;
    io->adc_present   = 0;
    io->pwm_present   = 0;
    io->pwm2_present  = 0;


    // Allocate a cleared context

    ctx = calloc(sizeof(CTX),1);
    if (!ctx)
    {
        io->err = ABIOCARD_INIT_ERR_OOM;
        return 0;
    }


    // Set up the context

    ctx->i2cbus_intf = io->i2cbus_intf;


    for (u = 0; u < XFR_CNT; u++)
    {
        ctx->xfr_list[u] = ctx->i2cbus_intf->fn_table->xfr_create_fn(ctx->i2cbus_intf);
        if (!ctx->xfr_list[u])
        {
            io->err = ABIOCARD_INIT_ERR_OOM;
            goto Err;
        }
    }


    // Probe and initialize peripherals on the AbioCard

    dev_cnt = 0;

    ok = rtc_probe(ctx);
    if (ok)
    {
        io->rtc_present = 1;
        dev_cnt++;
    }

    ok = ioexp_read(ctx,0);
    if (ok)
    {
        io->ioexp_present = 1;
        dev_cnt++;
    }

    ok = adc_init(ctx);
    if (ok)
    {
        io->adc_present = 1;
        dev_cnt++;
    }

    ok = pwm_init(ctx);
    if (ok)
    {
        io->pwm_present = 1;
        dev_cnt++;
    }

    ok = pwm2_init(ctx);
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

    return (ABIOCARD_HANDLE)ctx;

  Err:
  
    deinit(ctx);

    return 0;
}
