
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// abiocard.h
//
// AbioCard driver - public header.
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


#ifndef __ABIOCARD_H__
#define __ABIOCARD_H__


#include "abiocard_data.h"


FLAG  abiocard_init (ABIOCARD_INIT_IO *io);

VOID  abiocard_deinit (VOID);


FLAG  abiocard_rtc_poll (ABIOCARD_RTC_POLL_INFO *info);

FLAG  abiocard_rtc_set_time (ABIOCARD_RTC_TIME *time);


FLAG  abiocard_ioexp_write (U8 word);

FLAG  abiocard_ioexp_read (U8 *word);


FLAG  abiocard_adc_convert (ABIOCARD_ADC_CNV_DATA *buf);


FLAG  abiocard_pwm_write_range (U8 *buf, U8 start, U8 cnt);

FLAG  abiocard_pwm_read_range (U8 *buf, U8 start, U8 cnt);


FLAG  abiocard_pwm2_write_range (ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt);

FLAG  abiocard_pwm2_read_range (ABIOCARD_PWM2_CH *buf, U8 start, U8 cnt);

FLAG  abiocard_pwm2_write_prescaler (U8 val);

FLAG  abiocard_pwm2_read_prescaler (U8 *val);


#endif  // __ABIOCARD_H__

