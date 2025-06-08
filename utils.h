#pragma once

#include <cmath>

#include <type_traits>

#include "type_cast.h"

constexpr long double EPSILON = 1.0E-8L;

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if_t<std::is_integral_v<Type>, bool>
isZero(Type x) noexcept
{
    return x == Type(0);
}

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if_t<std::is_floating_point_v<Type>, bool>
isZero(Type x) noexcept
{
    return std::fabs(x) < EPSILON;
}

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if_t<std::is_integral_v<Type>, bool>
isEqual(Type x, Type y) noexcept
{
    return x == y;
}

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if_t<std::is_floating_point_v<Type>, bool>
isEqual(Type x, Type y) noexcept
{
    // Сравнивать переменные между собой и с нулём напрямую можно, если их
    // значения получены прямым присвоением одинаковых литералов, без иных
    // манипуляций, таких как математические операции или привидение типов
    // (иногда приведение допустимо, а в случае с нулём допустимо всегда).
    return std::fabs(static_cast<TwiceBiggerType<Type>>(x) - y) < EPSILON;
}

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if_t<std::is_arithmetic_v<Type>,
SignedType<TwiceBiggerType<Type>>>
toSignedTwiceBiggerArithmeticType(Type x) noexcept
{
    return static_cast<SignedType<TwiceBiggerType<Type>>>(static_cast<TwiceBiggerType<Type>>(x));
}

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if_t<std::is_arithmetic_v<Type>,
UnsignedType<BiggestType<Type>>>
toUnsignedBiggestArithmeticType(Type x) noexcept
{
    return static_cast<UnsignedType<BiggestType<Type>>>(static_cast<UnsignedType<Type>>(x < 0 ? -x : 0));
}

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if<std::is_signed<Type>::value, Type>::type
negateValue(Type value) noexcept
{
    return -value;
}

template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if<std::is_unsigned<Type>::value, Type>::type
negateValue(Type value) noexcept
{
    return value;
}
