#include <clocale>

#include <string>
#include <vector>

#include <fstream>
#include <iostream>

#include <filesystem>

#ifdef UNDER_CONSTRUCTION
#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <iomanip>
#include <limits>

#include "perf_prof.h"
#endif

#include "config.h"
#include "kdtree.h"
#include "point.h"
#include "tools.h"
#include "io.h"

#ifndef NDEBUG
#include "debug.h"
#include "tests.h"
#endif

int main(int argc, char* argv[])
{
    std::setlocale(LC_ALL, "");

#ifndef NDEBUG
    if (!unitTests())
    {
        std::cout << "\x1b[1;31mОшибка при выполнении тестов!\x1b[0m\n";

        return 1;
    }
#endif

    auto& config_params = ConfigParams::getInstance();

    std::cout << "Рабочий каталог: \x1b[4m"
              << std::filesystem::current_path() << "\x1b[0m\n";

    std::cout << "\x1b[1mВведите путь к конфигурационному файлу " \
                 "(без кавычек, пустая строка = \x1b[0m" \
                 "\x1b[1;4m\"" << config_params.getParam<std::string>("config_fn") << "\"\x1b[0m" \
                 "\x1b[1m):\x1b[0m\n";

    std::string config_fn;
    std::cin >> std::noskipws >> config_fn;
    if (!config_params.readConfig(config_fn))
    {
        std::cout << "\x1b[1;31mОшибка при чтении конфигурации!\x1b[0m\n";

        return 1;
    }

    auto points = readPoints<int, double>(config_params.getParam<std::string>("known_points_fn"),
                                          config_params.axis_names,
                                          config_params.value_name);
    if (points.empty())
    {
        std::cout << "\x1b[1;31mНет опорных точек!\x1b[0m\n";

        return 1;
    }

    KdTree tree{std::move(points)};
    if (tree.isEmpty())
    {
        std::cout << "\x1b[1;31mПустое дерево!\x1b[0m\n";

        return 1;
    }

    points = readPoints<decltype(points[0])>(config_params.getParam<std::string>("unknown_points_fn"),
                                             config_params.axis_names,
                                             config_params.value_name);
    if (points.empty())
    {
        std::cout << "\x1b[1;31mНет искомых точек!\x1b[0m\n";

        return 1;
    }

#if defined(UNDER_CONSTRUCTION) && defined(_MSC_VER)
    CONST MMRESULT result = timeBeginPeriod(1);
    if (result == TIMERR_NOCANDO)
        std::cout << "\x1b[1;31mОшибка установки интервала прерывания таймера: "
                  << result
                  << ".\x1b[0m\n";

    CONST HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION,
                                      FALSE,
                                      GetCurrentProcessId());
    if (!handle)
        printLastError();

    // Системное время - это время, прошедшее с момента запуска Windows
    // (в миллисекундах), которое обновляется в соответствии с заданным
    // интервалом прерывания таймера (см. функцию timeBeginPeriod()).
    CONST DWORD sys_time0 = timeGetTime();
    // Общее время выполнения процесса, т.е. суммарное количество времени,
    // которое процесс выполнялся в пользовательском режиме и режиме ядра
    // (по 100 наносекундных единиц времени в каждом отсчёте).
    CONST ULARGE_INTEGER exec_time0 = getProcTime(handle);
#endif

    const auto serialized_points = shepardInterpolation(tree, points,
                                                        config_params.getParam<std::size_t>("num_neighbors"),
                                                        config_params.getParam<bool>("reverse_search"),
                                                        config_params.getParam<double>("idw_power"),
                                                        config_params.getParam<int>("json_indent"),
                                                        config_params.axis_names,
                                                        config_params.value_name);

#if defined(UNDER_CONSTRUCTION) && defined(_MSC_VER)
    CONST ULARGE_INTEGER exec_time1 = getProcTime(handle);
    CONST DWORD sys_time1 = timeGetTime();
    CONST ULARGE_INTEGER exec_time{
        .QuadPart = exec_time1.QuadPart > exec_time0.QuadPart ?
                    exec_time1.QuadPart - exec_time0.QuadPart :
                    0
    };

    if (handle)
        CloseHandle(handle);

    if (result == TIMERR_NOERROR)
        timeEndPeriod(1);

    const auto precision{std::cout.precision()};
    std::cout << std::fixed << std::setprecision(std::numeric_limits<double>::digits10)
              << "\x1b[1;34mВремя выполнения: "
              << (exec_time.QuadPart / 10.0) / 1000.0
              << " (" << sys_time1 - sys_time0 << ") мс.\x1b[0m\n"
              << std::defaultfloat << std::setprecision(precision);
#endif

    if (serialized_points.empty())
    {
        std::cout << "\x1b[1;31mПустой результат!\x1b[0m\n";

        return 1;
    }

#ifndef NDEBUG
    DEBUG_INFO();
    std::cout << serialized_points << "\n\n";
#endif

    std::ofstream out{config_params.getParam<std::string>("output_fn")};
    if (!out.is_open())
    {
        std::cout << "\x1b[1;31mОшибка при записи результата!\x1b[0m\n";

        return 1;
    }

    out << serialized_points;
    out.close();

    std::cout << "\x1b[1;32mВыполнено успешно.\x1b[0m\n";

    return 0;
}
