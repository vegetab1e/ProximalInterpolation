#pragma once

#include <cstring>

#include <string>
#include <utility>
#include <type_traits>
#include <initializer_list>

#if __cplusplus >= 201703L
#include <tuple>
#endif

#include <ostream>

#include <stdexcept>

#include "utils.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

template<class, class, std::size_t>
class Point;

template<class C, class V, std::size_t N>
std::ostream& operator<<(std::ostream&, const Point<C, V, N>&);

template<class C, class V, std::size_t N>
class Point final
{
    static_assert(std::is_arithmetic_v<C>, "C must be an arithmetic type!");
    static_assert(std::is_arithmetic_v<V>, "V must be an arithmetic type!");

    friend
    std::ostream& operator<< <>(std::ostream& out, const Point& point);

public:
#if __cplusplus < 201703L
    // The dynamic exception specification, or throw(optional_type_list) specification,
    // was deprecated in C++11 and removed in C++17, except for throw(),
    // which is an alias for noexcept(true).
    Point() throw();
#else
    Point() = default;
#endif

    template<std::size_t M>
    explicit Point(const C (&coords)[M], V value = V()) noexcept;

    Point(std::initializer_list<C> coords, V value) noexcept;

#if __cplusplus < 201703L
    template<class T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    Point(std::initializer_list<T> coords, V value) noexcept;
#else
    template<class T>
    requires std::is_arithmetic_v<T>
    Point(std::initializer_list<T> coords, V value) noexcept;
#endif

    template<class T, class U>
    requires std::is_arithmetic_v<U>
    Point(std::initializer_list<T> coords, U value) noexcept;

    template<class T, class U, std::size_t M>
    requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
    explicit operator Point<T, U, M>() noexcept;

    static constexpr std::size_t getNumAxes() noexcept;

    bool compareEqual(const Point& point) const noexcept;

    bool compareExactlyEqual(const Point& point) const noexcept;

    [[deprecated("This function is outdated. " \
    "Use compareEqual()/compareExactlyEqual() instead.")]]
    bool operator==(const Point& point) const noexcept;

    bool compareLess(const Point& point) const noexcept;

    bool compareLess(const Point& point, std::size_t axis) const;

    auto getDistance(const Point& point, std::size_t axis) const;

    auto getDistance(const Point& point) const noexcept;

    C getCoord(std::size_t axis) const;

    V getValue() const noexcept;

    void setValue(V value) noexcept;

    template<class U>
    requires std::is_arithmetic_v<U>
    void setValue(U value) noexcept;

    template<class T, class U, std::size_t M>
    void setValue(const Point<T, U, M>& point) noexcept;

    std::string toString() const;

private:
// Это разделение связано с инициализаторами членов по умолчанию
// и их использованием для агрегатных классов, поэтому правильно
// было бы задать число 201402L, однако в одном из конструкторов
// есть structured bindings declaration, поэтому задано 201703L.
// Для MSVC необходимо указывать опцию сборки "/Zc:__cplusplus"!
#if __cplusplus < 201703L
    C coords_[N];
    V value_;
#else
    C coords_[N]{};
    V value_{};
#endif
};


#if __cplusplus < 201703L
#if __cplusplus < 201103L
#if 0
template<class C, class V, std::size_t N>
Point<C, V, N>::Point() throw()
    : value_(0)
{
    std::memset(coords_, 0, sizeof(coords_));
}
#else
// C++03, formally known as ISO/IEC 14882:2003, was a technical corrigendum to the C++98 standard.
// One of its key contributions was the formalization and introduction of value initialization.
template<class C, class V, std::size_t N>
Point<C, V, N>::Point() throw()
    : coords_()
    , value_()
{
}
#endif // 0
#else
template<class C, class V, std::size_t N>
Point<C, V, N>::Point() noexcept
    : coords_{}
    , value_{}
{
}
#endif

template<class C, class V, std::size_t N>
template<std::size_t M>
Point<C, V, N>::Point(const C (&coords)[M], V value) noexcept
    : value_(value)
{
    std::memcpy(coords_,
                coords,
                MIN(sizeof(coords_), sizeof(coords)));

    if (M < N)
        std::memset(coords_ + M, 0, sizeof(*coords_) * (N - M));
}

template<class C, class V, std::size_t N>
Point<C, V, N>::Point(std::initializer_list<C> coords, V value) noexcept
    : value_(value)
{
    if (coords.size() == 0)
    {
        std::memset(coords_, 0, sizeof(coords_));

        return;
    }

    std::memcpy(coords_,
                coords.begin(),
                MIN(sizeof(coords_), sizeof(*coords.begin()) * coords.size()));

    if (coords.size() < N)
        std::memset(coords_ + coords.size(), 0, sizeof(*coords_) * (N - coords.size()));
}

#pragma warning(push)
#pragma warning(disable : 26495)
template<class C, class V, std::size_t N>
template<class T, class>
Point<C, V, N>::Point(std::initializer_list<T> coords, V value) noexcept
    : value_(value)
{
    if (coords.size() == 0)
    {
        std::memset(coords_, 0, sizeof(coords_));

        return;
    }

    std::size_t i = 0;
    for (auto j = coords.begin(); i < N && j < coords.end(); ++i, ++j)
        if (std::is_unsigned<C>::value && std::is_signed<T>::value)
            coords_[i] = static_cast<C>(ABS_EX(*j));
        else
            coords_[i] = static_cast<C>(*j);

    if (i < N)
        std::memset(coords_ + i, 0, sizeof(*coords_) * (N - i));
}
#pragma warning(pop)
#else
template<class C, class V, std::size_t N>
template<std::size_t M>
Point<C, V, N>::Point(const C (&coords)[M], V value) noexcept
    : value_(value)
{
    std::memcpy(coords_,
                coords,
                MIN(sizeof(coords_), sizeof(coords)));
}

template<class C, class V, std::size_t N>
Point<C, V, N>::Point(std::initializer_list<C> coords, V value) noexcept
    : value_(value)
{
    if (std::empty(coords))
        return;

    std::memcpy(coords_,
                std::data(coords),
                MIN(sizeof(coords_), sizeof(*coords.begin()) * coords.size()));
}

template<class C, class V, std::size_t N>
template<class T>
requires std::is_arithmetic_v<T>
Point<C, V, N>::Point(std::initializer_list<T> coords, V value) noexcept
    : value_(value)
{
    if (std::empty(coords))
        return;

    for (auto [i, j] = std::tuple{0, coords.begin()}; i < N && j < coords.end(); ++i, ++j)
        if constexpr (std::is_unsigned_v<C> && std::is_signed_v<T>)
            coords_[i] = static_cast<C>(ABS(*j));
        else
            coords_[i] = static_cast<C>(*j);
}
#endif

template<class C, class V, std::size_t N>
template<class T, class U>
requires std::is_arithmetic_v<U>
Point<C, V, N>::Point(std::initializer_list<T> coords, U value) noexcept
    : Point(coords, V())
{
    if constexpr (std::is_unsigned_v<V> && std::is_signed_v<U>)
        value_ = static_cast<V>(ABS(value));
    else
        value_ = static_cast<V>(value);
}

template<class C, class V, std::size_t N>
template<class T, class U, std::size_t M>
requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
Point<C, V, N>::operator Point<T, U, M>() noexcept
{
    T coords[M]{};
    for (std::size_t i = 0; i < MIN(M, N); ++i)
        if constexpr (std::is_unsigned_v<T> && std::is_signed_v<C>)
            coords[i] = static_cast<T>(ABS(coords_[i]));
        else
            coords[i] = static_cast<T>(coords_[i]);

    U value{};
    if constexpr (std::is_unsigned_v<U> && std::is_signed_v<V>)
        value = static_cast<U>(ABS(value_));
    else
        value = static_cast<U>(value_);

    return Point<T, U, M>(coords, value);
}

template<class C, class V, std::size_t N>
constexpr std::size_t Point<C, V, N>::getNumAxes() noexcept
{
    return N;
}

template<class C, class V, std::size_t N>
bool Point<C, V, N>::compareEqual(const Point& point) const noexcept
{
    for (std::size_t i = 0; i < N; ++i)
        if (!isEqual(coords_[i], point.coords_[i]))
            return false;

    return true;
}

template<class C, class V, std::size_t N>
bool Point<C, V, N>::compareExactlyEqual(const Point& point) const noexcept
{
    return compareEqual(point) && isEqual(value_, point.value_);
}

template<class C, class V, std::size_t N>
[[deprecated("This function is outdated. " \
"Use compareEqual()/compareExactlyEqual() instead.")]]
bool Point<C, V, N>::operator==(const Point& point) const noexcept
{
    return compareEqual(point);
}

template<class C, class V, std::size_t N>
bool Point<C, V, N>::compareLess(const Point& point) const noexcept
{
    for (std::size_t i = 0; i < N; ++i)
        if (isEqual(coords_[i], point.coords_[i]))
            continue;
        else if (coords_[i] < point.coords_[i])
            return true;
        else
            return false;

    return false;
}

template<class C, class V, std::size_t N>
bool Point<C, V, N>::compareLess(const Point& point, std::size_t axis) const
{
    if (axis >= N)
        throw std::invalid_argument("The axis is invalid!");

    // Координаты с плавающей точкой могут быть
    // на самом деле равны между собой, то есть
    // отличаться не более чем на эпсилон - это
    // важно учитывать при использовании метода
    // вообще, а особенно при сборке с макросом
    // ALLOW_DUPLICATE_POINTS (см. метод выше).
    return coords_[axis] < point.coords_[axis];
}

template<class C, class V, std::size_t N>
auto Point<C, V, N>::getDistance(const Point& point, std::size_t axis) const
{
    if (axis >= N)
        throw std::invalid_argument("The axis is invalid!");

    return toSignedTwiceBiggerArithmeticType<C>(coords_[axis]) -
           toSignedTwiceBiggerArithmeticType<C>(point.coords_[axis]);
}

template<class C, class V, std::size_t N>
auto Point<C, V, N>::getDistance(const Point& point) const noexcept
{
    UnsignedType<BiggestType<C>> sum = decltype(sum)(0);
    for (std::size_t i = 0; i < N; ++i)
    {
        const auto diff = toSignedTwiceBiggerArithmeticType<C>(coords_[i]) -
                          toSignedTwiceBiggerArithmeticType<C>(point.coords_[i]);

        sum += static_cast<decltype(sum)>(static_cast<BiggestType<C>>(diff) * diff);
    }

    return std::sqrt(sum);
}

template<class C, class V, std::size_t N>
C Point<C, V, N>::getCoord(std::size_t axis) const
{
    if (axis >= N)
        throw std::invalid_argument("The axis is invalid!");

    return coords_[axis];
}

template<class C, class V, std::size_t N>
V Point<C, V, N>::getValue() const noexcept
{
    return value_;
}

template<class C, class V, std::size_t N>
void Point<C, V, N>::setValue(V value) noexcept
{
    value_ = value;
}

template<class C, class V, std::size_t N>
template<class U>
requires std::is_arithmetic_v<U>
void Point<C, V, N>::setValue(U value) noexcept
{
    setValue(static_cast<V>(value));
}

template<class C, class V, std::size_t N>
template<class T, class U, std::size_t M>
void Point<C, V, N>::setValue(const Point<T, U, M>& point) noexcept
{
    setValue(point.getValue());
}

template<class C, class V, std::size_t N>
std::string Point<C, V, N>::toString() const
{
    std::string out = '(' + std::to_string(coords_[0]);
    for (std::size_t i = 1; i < N; ++i)
        out += ", " + std::to_string(coords_[i]);
    out += ") = " + std::to_string(value_);

    return out;
}

template<class C, class V, std::size_t N>
std::ostream& operator<<(std::ostream& out, const Point<C, V, N>& point)
{
    return out << point.toString();
}


template<std::size_t I, class T>
std::tuple_element_t<I, std::enable_if_t<I < std::tuple_size_v<T>, T>>
parseTemplate() noexcept
{
    static_assert(false, "Calling parseTemplate() is ill-formed");
}

template<std::size_t I, class C, class V, std::size_t N>
decltype(parseTemplate<I, std::tuple<C, V, std::integral_constant<std::size_t, N>>>())
parseTemplate(Point<C, V, N>&&) noexcept
{
    static_assert(false, "Calling parseTemplate() is ill-formed");
}

template<std::size_t I, class T>
using GetParamAt = decltype(parseTemplate<I>(std::declval<std::decay_t<T>>()));

#undef MIN
