#include <clocale>

#include <string>

#include <fstream>
#include <iostream>

#include <filesystem>

#include <stdexcept>

#include "kdtree.h"
#include "point.h"
#include "tools.h"

#include "config.h"

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

    try
    {
        std::string config_fn;
        std::cin >> std::noskipws >> config_fn;
        if (!config_params.readConfig(config_fn))
        {
            std::cout << "\x1b[1;31mОшибка при чтении конфигурации!\x1b[0m\n";

            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;

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

    const auto serialized_points = shepardInterpolation(tree, points,
                                                        config_params.getParam<std::size_t>("max_neighbors"),
                                                        config_params.getParam<bool>("reverse_search"),
                                                        config_params.getParam<double>("idw_power"),
                                                        config_params.getParam<int>("json_indent"),
                                                        config_params.axis_names,
                                                        config_params.value_name);
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
