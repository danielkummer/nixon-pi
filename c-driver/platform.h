
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// platform.h
//
// Language: GCC4 gnu89
// Platform: Linux armv6l (little endian)
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


#ifndef __PLATFORM_H__
#define __PLATFORM_H__


typedef unsigned char       U8;
typedef unsigned short int  U16;
typedef unsigned int        U32;
typedef unsigned long long  U64;

typedef char                S8;
typedef short int           S16;
typedef int                 S32;
typedef long long           S64;

typedef char                CHAR;       // ASCII character set

typedef void                VOID;

typedef VOID               *POINTER;    // Pointer

typedef U8                  FLAG;       // 1 bit (assign 0 or 1 values only)

// Dynll

#define DynllCall
#define DynllExport __attribute__((visibility("default")))
#define DynllImport


#endif  // __PLATFORM_H__

