#pragma once

#include <array>
#include <vector>
#include <string>

#ifndef ALLOW_DUPLICATE_POINTS
#include <set>
#endif

#include <fstream>

#include <exception>

#include <nlohmann/json.hpp>

#include "point.h"

template<class C, class V, std::size_t N>
bool readPoints(std::ifstream& file,
                std::vector<Point<C, V, N>>& points,
                const std::array<const char*, N>& axis_names,
                const char* value_name)
{
    using json = nlohmann::json;

    const json data = json::parse(file);
    if (!data.is_array() || data.empty())
    {
        std::cerr << "The file is ill-formed!\n";

        return false;
    }

    points.reserve(data.size());

#ifndef ALLOW_DUPLICATE_POINTS
    struct CompareLess
    {
        bool operator()(const Point<C, V, N>* lhs,
                        const Point<C, V, N>* rhs) const noexcept
        {
            return lhs->compareLess(*rhs);
        }
    };

    std::set<const Point<C, V, N>*, CompareLess> unique_points;
#endif

    C coords[N]{};
    for (const json& object : data)
    {
        if (!object.is_object() || object.size() < N)
        {
            std::cerr << "The array is invalid!\n";

            points.clear();

            return false;
        }

        json::const_iterator iterator;
        for (std::size_t i = 0; i < N; ++i)
        {
            iterator = object.find(axis_names[i]);
            if (iterator == object.cend() || !iterator->is_number())
            {
                std::cerr << "The coordinate is missing!\n";

                points.clear();

                return false;
            }

            coords[i] = iterator.value().template get<C>();
        }

        V value{};
        iterator = object.find(value_name);
        if (iterator != object.cend() && iterator->is_number())
            value = iterator.value().template get<V>();

        [[maybe_unused]]
        auto const*const point = &points.emplace_back(coords, value);

#ifndef ALLOW_DUPLICATE_POINTS
        if (!unique_points.insert(point).second)
            points.pop_back();
#endif
    }

    return true;
}

template<class C, class V, std::size_t N>
std::vector<Point<C, V, N>> readPoints(const std::string& filename,
                                       const std::array<const char*, N>& axis_names,
                                       const char* value_name)
{
    std::ifstream file{filename};
    if (!file.is_open())
        return {};

    std::vector<Point<C, V, N>> points;

    try
    {
        readPoints(file, points, axis_names, value_name);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;

        points.clear();
    }

    file.close();

    return points;
}

// Вызов данной функции выполняется так:
// readPoints<decltype(points[0])>(...);
// decltype(points[0]) — это тип объекта, например, Point<int, double, 2>,
// а не сам шаблон Point, поэтому невозможно принять его подобным образом:
// template<template<class, class, std::size_t> class Point, class C, class V, std::size_t N>
// Есть два способа решения проблемы: разбирать тип объекта с помощью вспомогательных функций
// или структур с соответствующими параметрами шаблонов, как здесь, или же, что будет гораздо
// проще, добавить соответствующий аргумент со значением по умолчанию в функцию readPoints().
template<class PointType, class... Types>
auto readPoints(Types&&... arguments)
{
    return readPoints<GetParamAt<0, PointType>,
                      GetParamAt<1, PointType>,
                      GetParamAt<2, PointType>::value>(
                          std::forward<Types>(arguments)...);
}

template<class C, class V, std::size_t N>
void writePoints(const std::string& filename,
                 const std::vector<Point<C, V, N>>& points,
                 int json_indent,
                 const std::array<const char*, N>& axis_names,
                 const char* value_name)
{
    std::ofstream file{filename};
    if (!file.is_open())
        return;

    try
    {
        using json = nlohmann::json;

        json array = json::array();
        for (auto& point : points)
        {
            json object = json::object();
            for (std::size_t i = 0; i < N; ++i)
                object[axis_names[i]] = point.getCoord(i);
            object[value_name] = point.getValue();

            array.emplace_back(std::move(object));
        }

        file << array.dump(json_indent);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    file.close();
}
