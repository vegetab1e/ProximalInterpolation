#pragma once

#include <cmath>

#include <array>
#include <vector>

#ifndef ZERO_DISTANCE_HANDLING
#include <algorithm>
#endif

#include <fstream>

#include <stdexcept>

#include <nlohmann/json.hpp>

#include "point.h"
#include "utils.h"

template<class C, class V, std::size_t N>
V shepardInterpolation(const Point<C, V, N>& point,
                       const std::vector<Point<C, V, N>>& neighbors,
                       double idw_power = 2.0) noexcept
{
    long double num = 0.0L, den = 0.0L;
    for (const auto& neighbor : neighbors)
    {
#ifdef ZERO_DISTANCE_HANDLING
        const auto distance = neighbor.getDistance(point);
        if (isZero(distance))
            return neighbor.getValue();

        const auto weight = 1.0 / std::pow(distance, idw_power);
#else
        const auto weight = 1.0 / std::pow(std::max(distance, EPSILON), idw_power);
#endif
        num += weight * neighbor.getValue();
        den += weight;
    }

    if constexpr (!std::is_same_v<decltype(num), V>)
        return static_cast<V>(num / den);
    else
        return num / den;
}

template<class C, class V, std::size_t N>
std::string shepardInterpolation(const KdTree<Point<C, V, N>>& tree,
                                 std::vector<Point<C, V, N>>& points,
                                 std::size_t max_neighbors,
                                 bool reverse_search,
                                 double idw_power,
                                 int json_indent,
                                 const std::array<const char*, N>& axis_names,
                                 const char* value_name) noexcept
try
{
    using json = nlohmann::json;

    json array = json::array();
    for (auto& point : points)
    {
        tree.shepardInterpolation(point,
                                  max_neighbors,
                                  reverse_search,
                                  idw_power);

        json object = json::object();
        for (std::size_t i = 0; i < N; ++i)
            object[axis_names[i]] = point.getCoord(i);
        object[value_name] = point.getValue();

        array.emplace_back(std::move(object));
    }

    return array.dump(json_indent);
}
catch (const std::exception& e)
{
    std::cout << e.what() << std::endl;

    return {};
}

template<class C, class V, std::size_t N>
std::vector<Point<C, V, N>> readPoints(const std::string& filename,
                                       const std::array<const char*, N>& axis_names,
                                       const char* value_name) noexcept
try {
    using json = nlohmann::json;

    std::ifstream file{filename};
    if (!file.is_open())
        return {};

    const json data = json::parse(file);

    file.close();

    if (!data.is_array() || data.empty())
        return {};

    std::vector<Point<C, V, N>> points;
    points.reserve(data.size());

    C coords[N]{};
    for (const json& object : data)
    {
        if (!object.is_object() || object.size() < N)
            continue;

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

    return points;
}
catch (const std::exception& e)
{
    std::cout << e.what() << std::endl;

    return {};
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
auto readPoints(Types&&... arguments) noexcept
{
    return readPoints<GetParamAt<0, PointType>,
                      GetParamAt<1, PointType>,
                      GetParamAt<2, PointType>::value>(
                          std::forward<Types>(arguments)...);
}
