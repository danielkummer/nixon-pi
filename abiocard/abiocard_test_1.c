
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocard_test_1.c
//
// AbioCard test 1.
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
//      * Added choice of BSC.
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

#include "bcm2835_detect.h"
#include "abiocard.h"


static
VOID  test_rtc (VOID)
{
    ABIOCARD_RTC_POLL_INFO  info;
    FLAG                    ok;

    ok = abiocard_rtc_poll(&info);
    if (!ok) return;

    printf("Power-up...: %d\n",info.power_up);
    printf("Battery low: %d\n",info.battery_low);

    if (!info.power_up)
    {
        printf("Date & time: %04d-%02d-%02d %02d:%02d:%02d\n",
               info.time.year+2000,info.time.month,info.time.day,
               info.time.hour,info.time.minute,info.time.second);
    }
}


static
VOID  test_ioexp (VOID)
{
    U8      u;

    // Read the I/O lines
    abiocard_ioexp_read(&u);
    printf("ioexp: read  %02Xh\n",u);

    // Write back
    abiocard_ioexp_write(u);
    printf("ioexp: wrote %02Xh\n",u);

    // Read again
    abiocard_ioexp_read(&u);
    printf("ioexp: read  %02Xh\n",u);
}


static
VOID  test_adc (VOID)
{
    static  ABIOCARD_ADC_CNV_DATA    adc_cnv_data_1;

    U32     n;
    U16     val;

    // Perform an analog-to-digital conversion of all eight analog inputs
    abiocard_adc_convert(&adc_cnv_data_1);

    // Print all eight sample values
    for (n = 0; n < 8; n++)
    {
        val = ((adc_cnv_data_1[n][0] << 8) | adc_cnv_data_1[n][1]) & 4095;

        printf("AIN%d: %04d\n",
               n,
               val);
    }
}


static
VOID  test_pwm (VOID)
{
    static  U8	pwm_values[] =
    {
       15,  30,  45,  60,  75,  90, 105, 120,
      135, 150, 165, 180, 195, 210, 225, 240,
      255
    };

    abiocard_pwm_write_range((U8*)&pwm_values,0,17);
}


static
VOID  test_pwm2 (VOID)
{
    static  ABIOCARD_PWM2_CH    pwm_values[16] =
    {
       {    0,  200, 0, 0 },
       {  250,  600, 0, 0 },
       {  500, 1000, 0, 0 },
       {  750, 1400, 0, 0 },
       { 1000, 1800, 0, 0 },
       { 1250, 2200, 0, 0 },
       { 1500, 2600, 0, 0 },
       { 1750, 3000, 0, 0 },
       { 2000, 3400, 0, 0 },
       { 2250, 3800, 0, 0 },
       { 2500,  200, 0, 0 },
       { 2750,  600, 0, 0 },
       { 3000, 1000, 0, 0 },
       { 3250, 1400, 1, 0 },
       { 3500, 1800, 0, 1 },
       { 3750, 2200, 1, 1 }
    };

    U8      val;

    // Read the prescaler value
    abiocard_pwm2_read_prescaler(&val);
    printf("prescaler value: %d\n",val);

    // Toggle the prescaler value by calculating the one-complement and write
    // back
    val = ~val;
    abiocard_pwm2_write_prescaler(val);

    // Read the prescaler value
    abiocard_pwm2_read_prescaler(&val);
    printf("prescaler value: %d\n",val);

    abiocard_pwm2_write_range((ABIOCARD_PWM2_CH*)&pwm_values,0,16);
}


int  main (int argc, char *argv[])
{
    BCM2835_DETECT_IO    detect_io;
    ABIOCARD_INIT_IO     abiocard_init_io;

    if (!bcm2835_detect(&detect_io))
    {
        printf("Error detecting BCM2835 hardware\n");
        return -1;
    }

    if (!detect_io.res_detected)
    {
        printf("BCM2835 hardware not detected\n");
        return -1;
    }

    // Determine the BSC index from the detected revision number:
    // * <0004:  Computer rev. 1, use BSC0
    // * >=0004: Computer rev. 2, use BSC1
    if (detect_io.res_revision >= 4) abiocard_init_io.bsc_index = 1;

    if (!abiocard_init(&abiocard_init_io))
    {
        printf("initialisation error %d\n",abiocard_init_io.err);

        return -1;
    }

    printf("RTC present : %d\n",abiocard_init_io.rtc_present);
    printf("I/O present : %d\n",abiocard_init_io.ioexp_present);
    printf("ADC present : %d\n",abiocard_init_io.adc_present);
    printf("PWM present : %d\n",abiocard_init_io.pwm_present);
    printf("PWM2 present: %d\n",abiocard_init_io.pwm2_present);


    if (abiocard_init_io.rtc_present) test_rtc();

    if (abiocard_init_io.ioexp_present) test_ioexp();

    if (abiocard_init_io.adc_present) test_adc();

    if (abiocard_init_io.pwm_present) test_pwm();

    if (abiocard_init_io.pwm2_present) test_pwm2();

    abiocard_deinit();

    return 0;
}

