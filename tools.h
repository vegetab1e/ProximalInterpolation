#pragma once

#include <cmath>

#include <array>
#include <vector>
#include <string>

#include <exception>

#include <nlohmann/json.hpp>

#ifndef NDEBUG
#include <filesystem>
#include "io.h"
#endif

#include "kdtree.h"
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
        const auto distance = neighbor.getDistance(point);
#ifdef ZERO_DISTANCE_HANDLING
        if (isZero(distance)) [[unlikely]]
            return neighbor.getValue();

        const auto weight = 1.0 / std::pow(distance, idw_power);
#else
        const auto weight = 1.0 / std::pow(isZero(distance) ? EPSILON<decltype(distance)>
                                                            : distance,
                                           idw_power);
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
                                 std::size_t num_neighbors,
                                 bool reverse_search,
                                 double idw_power,
                                 int json_indent,
                                 const std::array<const char*, N>& axis_names,
                                 const char* value_name) noexcept
try
{
    using json = nlohmann::json;

#ifndef NDEBUG
    std::string path{"out/"};
    path += reverse_search ? "rnns/" : "nns/";
    std::filesystem::create_directories(path);
#endif

    json array = json::array();
    for (auto& point : points)
    {
#ifndef NDEBUG
        auto neighbors = 
#endif
        tree.shepardInterpolation(point,
                                  num_neighbors,
                                  reverse_search,
                                  idw_power);
#ifndef NDEBUG
        writePoints(path + point.toString() + ".json",
                    neighbors,
                    json_indent,
                    axis_names,
                    value_name);
#endif

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
