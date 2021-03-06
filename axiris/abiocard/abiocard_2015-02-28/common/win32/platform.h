
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// platform.h
//
// Language: MSVC60
//
// History:
//
//   2012-05-12  Peter S'heeren, Axiris
//
//      * Created.
//
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


#ifndef __PLATFORM_H__
#define __PLATFORM_H__


typedef unsigned char       U8;
typedef unsigned short int  U16;
typedef unsigned int        U32;
typedef unsigned __int64    U64;

typedef char                S8;
typedef short int           S16;
typedef int                 S32;
typedef __int64             S64;

typedef char                CHAR;       // ASCII character set

// VOID type

#ifdef VOID
#undef VOID
#endif
typedef void                VOID;

typedef VOID               *POINTER;    // Pointer

typedef U8                  FLAG;       // 1 bit (assign 0 or 1 values only)

// Dynll

#define DynllCall   __stdcall
#define DynllExport
#define DynllImport


#endif  // __PLATFORM_H__

