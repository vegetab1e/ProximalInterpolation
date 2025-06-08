#pragma once

#include <cstdint>
#include <type_traits>

template<class T, bool>
struct MakeSignedType
{
    using Type = std::conditional_t<std::is_arithmetic_v<T>, T, void>;
};

#ifdef _MSC_VER
// Только MSVC пропустит подобную
// конструкцию, в ней нет смысла.
template<class Type>
struct MakeSignedType<Type, true>
{
    typedef std::make_signed_t<Type> Type;
};
#else
template<class T>
struct MakeSignedType<T, true>
{
    using Type = std::make_signed_t<T>;
};
#endif

template<class Type>
using SignedType = typename MakeSignedType<Type,
                                           std::is_integral_v<Type>
                                           && !std::is_same_v<Type, bool>
                                           && std::is_unsigned_v<Type>>::Type;

template<class T, bool>
struct MakeUnsignedType
{
    using Type = std::conditional_t<std::is_arithmetic_v<T>, T, void>;
};

template<class T>
struct MakeUnsignedType<T, true>
{
    using Type = std::make_unsigned_t<T>;
};

template<class Type>
using UnsignedType = typename MakeUnsignedType<Type,
                                               std::is_integral_v<Type>
                                               && !std::is_same_v<Type, bool>
                                               && std::is_signed_v<Type>>::Type;


template<class Type>
std::enable_if_t<std::is_same_v<Type, float>,
double>
getTwiceBiggerType() noexcept
{
#ifdef _MSC_VER
    // Только MSVC пропустит подобную конструкцию, в ней нет смысла.
    static_assert(false, "Calling '" __FUNCSIG__ "' is ill-formed");
#else
    static_assert(false, "Calling getTwiceBiggerType() is ill-formed");
#endif
}

template<class Type>
std::enable_if_t<std::is_same_v<Type, double>,
long double>
getTwiceBiggerType() noexcept
{
    static_assert(false, "Calling getTwiceBiggerType() is ill-formed");
}

template<class Type>
std::enable_if_t<std::is_integral_v<Type>
                 && (sizeof(Type) == sizeof(std::uint8_t)),
std::conditional_t<std::is_unsigned_v<Type>, std::uint16_t, std::int16_t>>
getTwiceBiggerType() noexcept
{
    static_assert(false, "Calling getTwiceBiggerType() is ill-formed");
}

template<class Type>
std::enable_if_t<std::is_integral_v<Type>
                 && (sizeof(Type) == sizeof(std::uint16_t)),
std::conditional_t<std::is_unsigned_v<Type>, std::uint32_t, std::int32_t>>
getTwiceBiggerType() noexcept
{
    static_assert(false, "Calling getTwiceBiggerType() is ill-formed");
}

template<class Type>
std::enable_if_t<std::is_integral_v<Type>
                 && (sizeof(Type) == sizeof(std::uint32_t)),
std::conditional_t<std::is_unsigned_v<Type>, std::uint64_t, std::int64_t>>
getTwiceBiggerType() noexcept
{
    static_assert(false, "Calling getTwiceBiggerType() is ill-formed");
}

template<class Type>
std::enable_if_t<std::is_same_v<Type, long double>
                 || (std::is_integral_v<Type>
                     && (sizeof(Type) == sizeof(std::uint64_t))),
Type>
getTwiceBiggerType() noexcept
{
    static_assert(false, "Calling getTwiceBiggerType() is ill-formed");
}

template<class Type>
std::enable_if_t<!std::is_arithmetic_v<Type>,
void>
getTwiceBiggerType() noexcept
{
    static_assert(false, "Calling getTwiceBiggerType() is ill-formed");
}

template<class Type>
using TwiceBiggerType = decltype(getTwiceBiggerType<Type>());

template<class Type>
std::enable_if_t<std::is_floating_point_v<Type>,
long double>
getBiggestType() noexcept
{
    static_assert(false, "Calling getBiggestType() is ill-formed");
}

template<class Type>
std::enable_if_t<std::is_integral_v<Type>,
std::conditional_t<std::is_unsigned_v<Type>, std::uintmax_t, std::intmax_t>>
getBiggestType() noexcept
{
    static_assert(false, "Calling getBiggestType() is ill-formed");
}

template<class Type>
std::enable_if_t<!std::is_arithmetic_v<Type>,
void>
getBiggestType() noexcept
{
    static_assert(false, "Calling getBiggestType() is ill-formed");
}

template<class Type>
using BiggestType = decltype(getBiggestType<Type>());
