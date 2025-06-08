#pragma once

#include <cstring>

#include <array>
#include <string>

#include <tuple>
#include <utility>
#include <type_traits>

#include <stdexcept>

#include "helper_funcs.h"

class ConfigParams final
{
    //
    // Здесь порядок полей и методов важен!
    //

    // Имена этих полей важны, они используются как
    // есть для параметров в конфигурационном файле!
    std::string config_fn{"config.json"};
    std::string output_fn{"output.json"};
    std::string known_points_fn{"known_points.json"};
    std::string unknown_points_fn{"unknown_points.json"};
    std::size_t num_neighbors{100UL};
    bool reverse_search{false};
    double idw_power{2.0};
    int json_indent{4};

    std::tuple<std::pair<const char*, decltype(config_fn)&>,
               std::pair<const char*, decltype(output_fn)&>,
               std::pair<const char*, decltype(known_points_fn)&>,
               std::pair<const char*, decltype(unknown_points_fn)&>,
               std::pair<const char*, decltype(num_neighbors)&>,
               std::pair<const char*, decltype(reverse_search)&>,
               std::pair<const char*, decltype(idw_power)&>,
               std::pair<const char*, decltype(json_indent)&>>
    params_;

    ConfigParams() noexcept(isNoThrowConstructible<decltype(params_)>());

public:
    static ConfigParams& getInstance() noexcept(noexcept(ConfigParams()));

    //
    // Дальше порядок не имеет значения.
    //

    static constexpr std::array axis_names{"x", "y"};
    static constexpr const char* value_name{"value"};

    bool readConfig(const std::string& filename);

    template<class T, std::size_t N>
    const T& getParam(const char (&name)[N]) const;

private:
    bool readConfig(std::ifstream& file);

    template<class T, std::size_t... Is>
    const T& getParam(const char* name, std::index_sequence<Is...>) const;

    ConfigParams(const ConfigParams&) = delete;
    ConfigParams& operator=(const ConfigParams&) = delete;

    ~ConfigParams() = default;
};


template<class T, std::size_t N>
const T& ConfigParams::getParam(const char (&name)[N]) const
{
    if (name[N - 1] != '\0')
        throw std::invalid_argument("The parameter name must be a null-terminated string!");

    return getParam<T>(name, std::make_index_sequence<std::tuple_size_v<decltype(params_)>>());
}

template<class T, std::size_t... Is>
const T& ConfigParams::getParam(const char* name, std::index_sequence<Is...>) const
{
    void* param = nullptr;
    auto find = [&name, &param](auto&& param_){
        if (std::strcmp(param_.first, name) != 0)
            return;

        if constexpr (std::is_same_v<decltype(param_.second), T&> == false)
            throw std::invalid_argument("The parameter type is invalid!");

        param = &param_.second;
    };

    (find(std::get<Is>(params_)), ...);

    if (param == nullptr)
        throw std::invalid_argument("The parameter name is invalid!");

    return *static_cast<T*>(param);
}
