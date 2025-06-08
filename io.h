#pragma once

#include <array>
#include <vector>
#include <string>

#include <fstream>

#include <stdexcept>

#include <nlohmann/json.hpp>

#include "point.h"

template<class C, class V, std::size_t N>
std::vector<Point<C, V, N>> readPoints(const std::string& filename,
                                       const std::array<const char*, N>& axis_names,
                                       const char* value_name)
{
    using json = nlohmann::json;

    std::ifstream file{filename};
    if (!file.is_open())
        return {};

    std::vector<Point<C, V, N>> points;

    try
    {
        const json data = json::parse(file);
        if (!data.is_array() || data.empty())
            throw std::logic_error("The file is ill-formed!");

        points.reserve(data.size());

        C coords[N]{};
        for (const json& object : data)
        {
            if (!object.is_object() || object.size() < N)
                throw std::logic_error("The array is invalid!");

            json::const_iterator iterator;
            for (std::size_t i = 0; i < N; ++i)
            {
                iterator = object.find(axis_names[i]);
                if (iterator == object.cend() || !iterator->is_number())
                    throw std::logic_error("The coordinate is missing!");

                coords[i] = iterator.value().template get<C>();
            }

            V value{};
            iterator = object.find(value_name);
            if (iterator != object.cend() && iterator->is_number())
                value = iterator.value().template get<V>();

            points.emplace_back(coords, value);
        }
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
// Варианта с передачей вектора для заполнения внутрь функции readPoints() не рассматривался,
// потому что тогда волшебство исчезнет и карета превратится в тыкву=(
template<class PointType, class... Types>
auto readPoints(Types&&... arguments)
{
    return readPoints<GetParamAt<0, PointType>,
                      GetParamAt<1, PointType>,
                      GetParamAt<2, PointType>::value>(
                          std::forward<Types>(arguments)...);
}

template<class C, class V, std::size_t N>
void writePoints(const std::vector<Point<C, V, N>>& points,
                 const std::string& filename,
                 int json_indent,
                 const std::array<const char*, N>& axis_names,
                 const char* value_name)
{
    using json = nlohmann::json;

    std::ofstream file{filename};
    if (!file.is_open())
        return;

    try
    {
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
