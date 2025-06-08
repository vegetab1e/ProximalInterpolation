#pragma once

#include <string>
#include <iostream>

#ifdef _MSC_VER
#include <Windows.h>

inline void printLastError(const char* func_name = nullptr)
{
    using namespace std::string_literals;

    static CHAR buffer[1024];
    CONST DWORD error = GetLastError();
    CONST DWORD num_chars = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                                           nullptr,
                                           error,
                                           LANG_SYSTEM_DEFAULT,
                                           buffer,
                                           sizeof(buffer),
                                           nullptr);

    std::cout << "\x1b[1;31m"
              << (func_name ? "Функция "s + func_name + " завершилась с ошибкой: "
                            : "Ошибка: "s)
              << (num_chars ? buffer : (std::to_string(error) + ".\n").c_str())
              << "\x1b[0m";
}

inline ULARGE_INTEGER getProcTime(HANDLE handle = nullptr)
{
    FILETIME creation_time, exit_time, kernel_time, user_time;
    if (!GetProcessTimes(handle ? handle : GetCurrentProcess(),
                         &creation_time,
                         &exit_time,
                         &kernel_time,
                         &user_time))
    {
        printLastError(__func__);

        return {0};
    }

    CONST ULARGE_INTEGER total_time{
        .QuadPart = reinterpret_cast<ULARGE_INTEGER*>(&kernel_time)->QuadPart
                  + reinterpret_cast<ULARGE_INTEGER*>(&user_time)->QuadPart
    };

    return total_time;
}

#endif
