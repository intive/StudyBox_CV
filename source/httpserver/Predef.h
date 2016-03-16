#ifndef PATR_PREDEF_H
#define PATR_PREDEF_H

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#    define PATR_OS_WINDOWS
#endif

#if defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE)
#    define PATR_OS_UNIX
#endif

#endif // PATR_PREDEF_H