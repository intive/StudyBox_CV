#ifndef PATR_GETEXEPATH_UTILITY_H
#define PATR_GETEXEPATH_UTILITY_H

#include <boost/predef.h>

#if BOOST_OS_LINUX
#include <string>
#include <limits.h>
#include <unistd.h>

    inline std::string GetExePath()
    {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        auto str = std::string(result, (count > 0) ? count : 0);
        return std::string(str.begin(), str.begin() + str.find_last_of('/') + 1);
    }
#elif BOOST_OS_WINDOWS
#include <string>
#include <windows.h>

    inline std::string GetExePath()
    {
        char result[MAX_PATH];
        auto str = std::string(result, GetModuleFileNameA(NULL, result, MAX_PATH));
        return std::string(str.begin(), str.begin() + str.find_last_of('\\') + 1);
    }
#endif // BOOST_OS...

#endif // PATR_GETEXEPATH_UTILITY_H