/*!
  \date
    5/31/2018
  \brief
    Define some macros and functions to make the linux code compile on windows, this header is force included in every other file.
*/
#ifndef WIN_MACRO
#define WIN_MACRO

#include <WinSock2.h> /*sockets*/
#include <time.h>
#include <Windows.h>

#define __attribute__(x) 
#define MSG_NOSIGNAL 0
#define CLOCK_PROCESS_CPUTIME_ID 0

/*!
  \brief
    used for clocl_gettime, copied off the internet
  \return
    I actually dont know
*/
LARGE_INTEGER getFILETIMEoffset();

/*!
  \brief
    windows clock_gettime, copied from stack overflow
  \param X
    Does nothing i think
  \param tv
    The timespec to save the data in
  \return
    the time
*/
int clock_gettime(int X, timespec *tv);

#endif