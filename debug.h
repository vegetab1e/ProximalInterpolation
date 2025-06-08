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
    // Поддержку __VA_OPT__ в MSVC можно включить с помощью флага компиляции /Zc:preprocessor,
    // указав его в опциях командной строки. С помощью макроса _MSVC_TRADITIONAL можно узнать,
    // какой будет использоваться препроцессор - традиционный или же соответствующий стандарту
    #if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
        #define DEBUG_INFO(...) printDebugInfo(__FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__)
    #elif __cplusplus >= 202002L
        #define DEBUG_INFO(...) printDebugInfo(__FUNCSIG__, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
    #else
        #error Failing compilation
    #endif
#elif defined(__GNUC__)
    #define DEBUG_INFO(...) printDebugInfo(__PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#elif __cplusplus >= 202002L
    #define DEBUG_INFO(...) printDebugInfo(__func__, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
    #error Failing compilation
#endif
