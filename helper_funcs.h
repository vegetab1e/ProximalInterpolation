#pragma once

#include <type_traits>

// Если в контексте вызова есть неявный указатель (this pointer), то,
// очевидно, эта функция не будет constexpr, то есть при её вызове из
// нестатических функций-членов результат не будет constexpr.
template<template<class...> class Tuple, class... Types>
constexpr bool isNoThrowConstructible(const Tuple<Types...>&) noexcept
{
    return std::is_nothrow_constructible_v<Tuple<Types...>, Types...>;
}

template<class Tuple, std::size_t... Is>
constexpr bool isNoThrowConstructible(std::index_sequence<Is...>) noexcept
{
    return std::is_nothrow_constructible_v<Tuple, std::tuple_element_t<Is, Tuple>...>;
}

template<class Tuple>
constexpr bool isNoThrowConstructible() noexcept
{
    return isNoThrowConstructible<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>());
}


//
// Все функции ниже можно заменить шаблоном
// структуры и его частичной специализацией
//

template<class Type, class... Types>
Type getReturnType(Type(*)(Types...)) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)&&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)const) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)const&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)const&&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)volatile) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)volatile&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)volatile&&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)const volatile) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)const volatile&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}

template<class Class, class Type, class... Types>
Type getReturnType(Type(Class::*)(Types...)const volatile&&) noexcept
{
    static_assert(false, "Calling getReturnType() is ill-formed");
}
