
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// axicat_al_conf.h
//
// AxiCat AL Configuration.
//
// Language: GCC4 gnu89, MSVC60
//
// History:
//
//   2014-07-04  Peter S'heeren, Axiris
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


#ifndef __AXICAT_AL_CONF_H__
#define __AXICAT_AL_CONF_H__


#include "osrtl.h"

#include <stdlib.h>


#define AXICAT_AL_Alloc             malloc
#define AXICAT_AL_Alloc_Clear(n)    calloc(n,1)
#define AXICAT_AL_Free              free
#define AXICAT_AL_Get_Ticks         OSRTL_GetTickCount


#endif  // __AXICAT_AL_CONF_H__
