#ifndef __TYPE_PRECISION_HPP__
#define __TYPE_PRECISION_HPP__

#include <cctype>
#include <type_traits>

template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
const auto type_precision(){
    if constexpr (std::is_same_v<T, float>) return 9;
    else if constexpr (std::is_same_v<T, double>) return 17;
    else if constexpr (std::is_same_v<T, long double>) return 33;
    else return 9;
}

template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
const auto type_name() {
    if constexpr (std::is_same_v<T, float>) return "float";
    else if constexpr (std::is_same_v<T, double>) return "double";
    else if constexpr (std::is_same_v<T, long double>) return "long double";
    else return "";
}

#endif