#pragma once

#include <iostream>

inline void printDebugInfo(const char* func_sig,
                           const char* filename,
                           int line_no,
                           ...)
{
    std::cout << "\x1b[1mFUNC: \x1b[0m" << func_sig << '\n'
              << "\x1b[1mFILE: \x1b[0m" << filename << '\n'
              << "\x1b[1mLINE: \x1b[0m" << line_no
              << std::endl;
}

#if defined(_MSC_VER)
#define DEBUG_INFO(...) printDebugInfo(__FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__)
#elif defined(__GNUC__)
#define DEBUG_INFO(...) printDebugInfo(__PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#error Failing compilation
#endif
