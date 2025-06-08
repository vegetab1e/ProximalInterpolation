#include <fstream>

#include <nlohmann/json.hpp>

#ifndef NDEBUG
#include <iostream>
#include "debug.h"
#endif

#include "config.h"

#define STRINGIFY(x) #x

ConfigParams::ConfigParams() noexcept(isNoThrowConstructible<decltype(params_)>())
    : params_{
        {STRINGIFY(config_fn), config_fn},
        {STRINGIFY(output_fn), output_fn},
        {STRINGIFY(known_points_fn), known_points_fn},
        {STRINGIFY(unknown_points_fn), unknown_points_fn},
        {STRINGIFY(max_neighbors), max_neighbors},
        {STRINGIFY(reverse_search), reverse_search},
        {STRINGIFY(idw_power), idw_power},
        {STRINGIFY(json_indent), json_indent}}
{
}

ConfigParams& ConfigParams::getInstance() noexcept(noexcept(ConfigParams()))
{
    static ConfigParams config_params;

    return config_params;
}

bool ConfigParams::readConfig(const std::string& filename)
{
    using json = nlohmann::json;

    std::ifstream file{!filename.empty() ? filename : config_fn};
    if (!file.is_open())
        return false;

    const json data = json::parse(file);

    file.close();

    if (!data.is_object() || data.empty())
        return false;

#ifndef NDEBUG
    DEBUG_INFO();
    for (const auto& [key, value] : data.items())
        std::cout << key << " : " << value << '\n';
    std::cout << std::endl;
#endif

    json::const_iterator iterator = data.find(STRINGIFY(output_fn));
    if (iterator != data.cend() && iterator->is_string())
    {
        auto string{iterator.value().template get<decltype(output_fn)>()};
        if (!string.empty())
            output_fn = std::move(string);
    }

    iterator = data.find(STRINGIFY(known_points_fn));
    if (iterator != data.cend() && iterator->is_string())
    {
        auto string{iterator.value().template get<decltype(known_points_fn)>()};
        if (!string.empty())
            known_points_fn = std::move(string);
    }

    iterator = data.find(STRINGIFY(unknown_points_fn));
    if (iterator != data.cend() && iterator->is_string())
    {
        auto string{iterator.value().template get<decltype(unknown_points_fn)>()};
        if (!string.empty())
            unknown_points_fn = std::move(string);
    }

    iterator = data.find(STRINGIFY(max_neighbors));
    if (iterator != data.cend() && iterator->is_number_unsigned())
        if (auto number = iterator.value().template get<decltype(max_neighbors)>())
            max_neighbors = number;

    iterator = data.find(STRINGIFY(reverse_search));
    if (iterator != data.cend() && iterator->is_boolean())
        iterator.value().get_to(reverse_search);

    iterator = data.find(STRINGIFY(idw_power));
    if (iterator != data.cend() && iterator->is_number_float())
        iterator.value().get_to(idw_power);

    iterator = data.find(STRINGIFY(json_indent));
    if (iterator != data.cend() && iterator->is_number_integer())
        iterator.value().get_to(json_indent);

    return true;
}
