
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// osrtl.c
//
// Run-time library functions specific to the operating system.
//
// Language: GCC4 gnu89
//
// History:
//
//   2014-07-05  Peter S'heeren, Axiris
//
//      * Created.
//
// ----------------------------------------------------------------------------
//
// Copyright (c) 2014  Peter S'heeren, Axiris
//
// This source text is provided as-is without any implied or expressed
// warranty. The authors don't accept any liability for damages that arise from
// using this source text or from using any software generated from this source
// text.
//
// You're free to copy, modify, and use this source text for any purpose.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#include "osrtl.h"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>


VOID  OSRTL_SleepMs (U32 ms)
{
    usleep(ms * 1000);
}


U32  OSRTL_GetTickCount (VOID)
{
    struct  timespec    ts;
    struct timeval      tv;
    int                 res;
    time_t              u;

    res = clock_gettime(CLOCK_MONOTONIC,&ts);
    if (res == 0)
    {
        u = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
        return (U32)u;
    }
    else
    {
        // No monotonic clock support. Use the old method. Note that the old
        // method is susceptible to changes to the system date and time.

        gettimeofday(&tv,0);
        u = (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
        return (U32)u;
    }
}


VOID  OSRTL_Yield (VOID)
{
    sched_yield();
}
