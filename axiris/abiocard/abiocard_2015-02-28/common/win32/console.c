
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// console.c
//
// Win32 console.
//
// Language: MSVC60
//
// History:
//
//   2014-08-26  Peter S'heeren
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


#include "console.h"

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>


FLAG  Console_Create (U32 columns, U32 lines)
{
    CONSOLE_SCREEN_BUFFER_INFO  coninfo;
    HANDLE                      stdout_handle;
    HANDLE                      stdin_handle;
    HANDLE                      stderr_handle;
    int                         crtout_handle;
    int                         crtin_handle;
    int                         crterr_handle;
    FILE                       *fout;
    FILE                       *fin;
    FILE                       *ferr;

    if (!AllocConsole()) return 0;

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    crtout_handle = _open_osfhandle((long)stdout_handle,_O_TEXT);
    fout          = _fdopen(crtout_handle,"w");

    stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    crtin_handle = _open_osfhandle((long)stdin_handle,_O_TEXT);
    fin          = _fdopen(crtin_handle,"r");

    stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
    crterr_handle = _open_osfhandle((long)stderr_handle,_O_TEXT);
    ferr          = _fdopen(crterr_handle,"w");

    GetConsoleScreenBufferInfo(stdout_handle,&coninfo);

    if (columns > 0) coninfo.dwSize.X = columns;

    if (lines > 0) coninfo.dwSize.Y = lines;

    SetConsoleScreenBufferSize(stdout_handle,coninfo.dwSize);

    setvbuf(fout,NULL,_IONBF,0);
    (*stdout) = (*fout);

    setvbuf(fin,NULL,_IONBF,0);
    (*stdin) = (*fin);

    setvbuf(ferr,NULL,_IONBF,0);
    (*stderr) = (*ferr);

    return 1;
}
