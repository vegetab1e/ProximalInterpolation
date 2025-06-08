#pragma once

#include <cmath>

#include <limits>
#include <algorithm>
#include <type_traits>

#include "type_cast.h"

template<class T>
inline constexpr T
MACHINE_EPSILON = std::numeric_limits<T>::epsilon();

template<class T>
inline constexpr
std::enable_if_t<std::is_floating_point_v<T>, T>
EPSILON = std::max(T(1.0E-8L), MACHINE_EPSILON<T>);

#define ABS(x) ((x) < 0 ? -(x) : (x))
// Этот макрос нужен, чтобы избежать ошибки C4146:
// "unary minus operator applied to unsigned type,
// result still unsigned", когда других вариантов,
#define ABS_EX(x) ((x) < 0 ? negateValue(x) : (x))
// например, constexpr if statement, просто нет.

#if __cplusplus < 201703L
template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
typename std::enable_if<std::is_signed<Type>::value, Type>::type
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
typename std::enable_if<std::is_unsigned<Type>::value, Type>::type
negateValue(Type value) noexcept
{
    return value;
}
#else
template<class Type>
inline
#ifdef __GNUC__
__attribute__((always_inline))
#else
__forceinline
#endif
std::enable_if_t<std::is_arithmetic_v<Type>, Type>
negateValue(Type value) noexcept
{
    if constexpr (std::is_signed_v<Type>)
        return -value;
    else
        return value;
}
#endif

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
    return ABS(x) < EPSILON<Type>;
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
    // Для чисел с плавающей точкой переполнение невозможно, вместо
    // этого можно получить бесконечность со знаком (как и у нуля).
    // Операции сравнения, а также унарный минус для бесконечностей
    // определены, поэтому в приведении типов с увеличением размера
    // (расширяющем преобразовании) здесь нет необходимости.
    const Type diff = x - y;
    // Сравнивать переменные между собой и с нулём напрямую можно, если их
    // значения получены прямым присвоением одинаковых литералов, без иных
    // манипуляций, таких как математические операции или привидение типов
    // (иногда приведение допустимо, а в случае с нулём допустимо всегда).
    return ABS(diff) < EPSILON<Type>;
}

// Всё в этом пространстве имён написано исключительно из академического
// интереса, однако имеет в основе практическое обоснование. Далее нужно
// реализовать полноценное сравнение чисел с плавающей точкой, используя
// ULPs (units in the last place). Текущая реализация учитывает порядок,
// но максимально проста и имеет ограниченную область применения.
namespace experimental_features
{

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
    // В отличии от функции выше, получение бесконечностей здесь
    // недопустимо по логике, но так же возможно на практике.
    const auto sum  = static_cast<TwiceBiggerType<Type>>(x) + y;
    const auto diff = static_cast<TwiceBiggerType<Type>>(x) - y;
    // Значение эпсилон нужно получить именно для Type, а не для
    // вдвое большего типа. При последующем умножении полученное
    // значение будет приведено к большему типу автоматически.
    return ABS(diff) < (MACHINE_EPSILON<Type> * (! std::isinf(sum)
                                                 ? ABS(sum)
                                                 : std::numeric_limits<decltype(sum)>::max()));
}

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
    return static_cast<UnsignedType<BiggestType<Type>>>(static_cast<UnsignedType<Type>>(ABS_EX(x)));
}
