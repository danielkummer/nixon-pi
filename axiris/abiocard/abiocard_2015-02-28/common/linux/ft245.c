
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// ft245.c
//
// Interface for FT245 attached to serial device path.
//
// Language: GCC4 gnu89
//
// History:
//
//   2014-07-04  Peter S'heeren, Axiris
//
//      * Created.
//
//   2014-08-24  Peter S'heeren, Axiris
//
//      * Extended FT245_Read() to detect USB surprise-removal.
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


#include "ft245.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


FLAG  FT245_Write (FT245_HANDLE handle, U8 *buf, U32 len, U32 *xfrd)
{
    int     fd;
    int     result;

    // Init
    fd = (int)handle - 1;

    if (len == 0)
    {
        *xfrd = 0;
        return 1;
    }

    result = write(fd,buf,len);
    if (result < 0)
    {
        *xfrd = 0;
        return 0;
    }
    else
    {
        *xfrd = (U32)result;
        return 1;
    }
}


FLAG  FT245_Read (FT245_HANDLE handle, U8 *buf, U32 len, U32 *xfrd)
{
    struct  timeval     tv;
    fd_set              read_fds;
    int                 fd;
    int                 result;

    // Init
    fd = (int)handle - 1;

    if (len == 0)
    {
        *xfrd = 0;
        return 1;
    }

    // The following code seems overcomplicated for reading bytes, but this
    // complexity enables the code to detect USB surprise-removal of the AxiCat
    // device.

    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    FD_ZERO(&read_fds);
    FD_SET(fd,&read_fds);

    result = select(fd+1,&read_fds,NULL,NULL,&tv);
    if (result < 0)
    {
        *xfrd = 0;
        return 0;
    }
    else
    if (result == 0)
    {
        *xfrd = 0;
        return 1;
    }

    result = read(fd,buf,len);
    if (result < 0)
    {
        *xfrd = 0;
        if (errno == EAGAIN)
            return 1;
        else
            return 0;
    }
    else
    if (result == 0)
    {
        *xfrd = 0;
        return 0;
    }
    else
    {
        *xfrd = (U32)result;
        return 1;
    }
}


VOID  FT245_Close (FT245_HANDLE handle)
{
    int     fd;

    if (!handle) return;

    // Init
    fd = (int)handle - 1;

    close(fd);
}


FT245_HANDLE  FT245_Open (U8 *path)
{
    int                 fd;
    struct  termios     t;
    speed_t             baudrate;

    // Open a file descriptor for the given path. Creation flags:
    // * O_RDWR: Access mode is read/write.
    // * O_NOCTTY: Avoid the given path to become the process's controlling
    //     terminal.
    // * O_NONBLOCK: Non-blocking mode.
    //
    // The open() function returns a non-negative value in case of success.
    //
    fd = open(path,O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return 0;

    // Get the terminal attributes for the file descriptor
    tcgetattr(fd,&t);

    // Set non-canonical input. All input characters (unless they're special for
    // signal or flow-control purposes) are passed to the application.
    cfmakeraw(&t);

    // Time to wait for input before read() returns (0.1 s units)
    t.c_cc[VTIME]=0;
    
    // Minimum number of bytes available in input queue for read() to return.
    t.c_cc[VMIN]=0;

    // Set baudrate
    cfsetspeed(&t,B9600);

    // Data bits: 8
    t.c_cflag &= ~CSIZE;
    t.c_cflag |= CS8;

    // Handshaking
    t.c_cflag &= ~CRTSCTS;

    // Set the terminal attributes
    tcsetattr(fd,TCSANOW,&t);

    // Purge Tx and Rx buffers
    tcflush(fd,TCIOFLUSH);

    // Return successfully
    return (FT245_HANDLE)(fd + 1);
}
