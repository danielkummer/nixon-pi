
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocard_data.h
//
// AbioCard driver - public header with only data declarations.
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
//   2012-10-19  Peter S'heeren, Axiris
//
//      * Added support for AbioCard model B.
//
//   2013-01-25  Peter S'heeren, Axiris
//
//      * Added support for the i2c-dev interface.
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


#ifndef __ABIOCARD_DATA_H__
#define __ABIOCARD_DATA_H__


#include "platform.h"


typedef struct  _ABIOCARD_INIT_IO               ABIOCARD_INIT_IO;
typedef struct  _ABIOCARD_RTC_TIME              ABIOCARD_RTC_TIME;
typedef struct  _ABIOCARD_RTC_POLL_INFO         ABIOCARD_RTC_POLL_INFO;
typedef struct  _ABIOCARD_PWM2_CH               ABIOCARD_PWM2_CH;


typedef U8      ABIOCARD_INIT_ERR;

#define ABIOCARD_INIT_ERR_NONE          0
#define ABIOCARD_INIT_ERR_OTHER         1
#define ABIOCARD_INIT_ERR_LOCKED        2
#define ABIOCARD_INIT_ERR_NO_PERM       3
#define ABIOCARD_INIT_ERR_NO_CARD       4
#define ABIOCARD_INIT_ERR_PARAM         5


typedef U8      ABIOCARD_INIT_INTF;

#define ABIOCARD_INIT_INTF_BSC          0
#define ABIOCARD_INIT_INTF_I2CDEV       1


// Initialisation I/O

struct  _ABIOCARD_INIT_IO
{
    ABIOCARD_INIT_INTF  intf;           // [IN]  Interface type
    U8                  bsc_index;      // [IN]  Index of the BSC controller (0, 1)
    CHAR               *i2cdev_name;    // [IN]  Device path

    // Valid when abiocard_init() returns 0
    //
    ABIOCARD_INIT_ERR   err;            // [OUT] Error code

    // Valid when abiocard_init() returns 1
    //
    FLAG                rtc_present;    // [OUT] NXP PCF2129A - realtime clock present y/n
    FLAG                ioexp_present;  // [OUT] NXP PCF8574 - I/O expander present y/n
    FLAG                adc_present;    // [OUT] Maxim MAX11614EEE+ - ADC present y/n
    FLAG                pwm_present;    // [OUT] NXP PCA9635 - PWM present y/n
    FLAG                pwm2_present;   // [OUT] NXP PCA9685 - PWM2 present y/n
};


// RTC date & time information

struct  _ABIOCARD_RTC_TIME
{
    U8                  year;           // 0..99 meaning 2000..2099
    U8                  month;          // 1..12
    U8                  day;            // 1..31
    U8                  hour;           // 0..23
    U8                  minute;         // 0..59
    U8                  second;         // 0..59
};


// RTC poll information

struct  _ABIOCARD_RTC_POLL_INFO
{
    ABIOCARD_RTC_TIME   time;           // [OUT] Date & time
    FLAG                power_up;       // [OUT] Power-up detected y/n
    FLAG                battery_low;    // [OUT] Battery low y/n
};


// ADC conversion data

typedef U8      ABIOCARD_ADC_CNV_DATA[8][2];


// PWM2

struct  _ABIOCARD_PWM2_CH
{
    U16                 on_pos;         // 0..4095
    U16                 off_pos;        // 0..4095
    FLAG                always_on;      // Always on y/n
    FLAG                always_off;     // Always off y/n
};


#endif  // __ABIOCARD_DATA_H__

