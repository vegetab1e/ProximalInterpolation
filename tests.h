#pragma once

#include <vector>

#include <iostream>

#include <exception>

#include "kdtree.h"
#include "tools.h"

#include "helper_funcs.h"

#ifndef NDEBUG
#include "debug.h"
#endif

constexpr std::size_t NUM_DIMS = 2UL;

template<class T>
void printNeighbors(const T& neighbors)
{
    std::cout << "\x1b[1;42mNearest neighbors:\x1b[0m\n\x1b[1;32m";
    for (const auto& neighbor : neighbors)
        std::cout << neighbor << '\n';
    std::cout << "\x1b[0m\n";
}

template<class T>
void printTargetPoint(const T& point)
{
    std::cout << "\x1b[1;41mTarget point:\x1b[0m\n"
              << "\x1b[1;31m" << point << "\x1b[0m\n\n";
}

template<class C, class V, std::size_t N>
bool testNnsSearchAndIdwInterpolation1(const KdTree<Point<C, V, N>>& tree,
                                       Point<C, V, N>& point,
                                       std::size_t num_neighbors,
                                       bool reverse_search,
                                       double idw_power) noexcept
{
#ifndef NDEBUG
    DEBUG_INFO();
#endif

    std::vector<Point<C, V, N>> neighbors;

    try
    {
        neighbors = tree.searchNeighbors(point,
                                         num_neighbors,
                                         reverse_search);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;

        return false;
    }

    printNeighbors(neighbors);

    point.setValue(shepardInterpolation(point,
                                        neighbors,
                                        idw_power));

    printTargetPoint(point);

    return true;
}

template<class C, class V, std::size_t N>
bool testNnsSearchAndIdwInterpolation2(const KdTree<Point<C, V, N>>& tree,
                                       Point<C, V, N>& point,
                                       std::size_t num_neighbors,
                                       bool reverse_search,
                                       double idw_power) noexcept
{
#ifndef NDEBUG
    DEBUG_INFO();
#endif

    decltype(getReturnType(&KdTree<Point<C, V, N>>::shepardInterpolation)) neighbors;

    try
    {
        neighbors = tree.shepardInterpolation(point,
                                              num_neighbors,
                                              reverse_search,
                                              idw_power);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;

        return false;
    }

    printNeighbors(neighbors);
    printTargetPoint(point);

    return true;
}

bool unitTests() noexcept
{
#ifndef NDEBUG
    DEBUG_INFO();
#endif

    using Array = int[];
    KdTree tree = KdTree(std::vector<Point<int, double, NUM_DIMS>>{
        {{8, 34, 88}, 89.6548L},
        {{-3}, 58.3256},
        {{-9.0L, 8.0L}, 8.36633},
        {Array{45, 65}, 4.7921F},
        {Array{21, -12}, -5.81225},
        {Array{0, 77}, 13.03254185L},
        {Array{65, 42}, -69.00115},
        {Array{13, -24}, 80.41564},
        {Array{55, 33}, -22.1515F},
        {Array{94, -65}, 42.648955},
        {{-32, -11}, -3.5135F}
    });

    std::cout << tree << '\n';

    if (tree.isEmpty())
        return false;

    if (!tree.remove({{-3, 0}, 0.0}) ||
        !tree.insert({{1, 1}, -45.102548}) ||
        !tree.insert({{50, 75}, 10.201111}) ||
        !tree.remove({{45, 65}, 0.0}) ||
        !tree.insert({{60, 80}, 2.718281828459045}) ||
#ifndef ALLOW_DUPLICATE_POINTS
         tree.insert({{60, 80}, 0.0}) ||
#endif
         tree.remove({{99, 99}, 0.0}))
        return false;

    std::cout << tree << '\n';

    const double ref_value = -43.91734030;
    const std::size_t num_neighbors = 4UL;
    Point<int, double, NUM_DIMS> point{{0, 0}, 0.0};
    if (!testNnsSearchAndIdwInterpolation1(tree, point,
                                           num_neighbors,
                                           false,
                                           2.0)
        || !isEqual(point.getValue(), ref_value)
        || (point.setValue(0.0), !testNnsSearchAndIdwInterpolation1(tree, point,
                                                                    num_neighbors,
                                                                    true,
                                                                    2.0))
        || !isEqual(point.getValue(), ref_value)
        || (point.setValue(0.0), !testNnsSearchAndIdwInterpolation2(tree, point,
                                                                    num_neighbors,
                                                                    false,
                                                                    2.0))
        || !isEqual(point.getValue(), ref_value)
        || (point.setValue(0.0), !testNnsSearchAndIdwInterpolation2(tree, point,
                                                                    num_neighbors,
                                                                    true,
                                                                    2.0))
        || !isEqual(point.getValue(), ref_value))
        return false;

    return true;
}
