#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <type_traits>

namespace utils
{
    std::string uint32_to_string(uint32_t num)
    {
        char id[5] = {0};
        id[0] = (num & 0x000000FF);
        id[1] = (num & 0x0000FF00) >> 8;
        id[2] = (num & 0x00FF0000) >> 16;
        id[3] = (num & 0xFF000000) >> 24;

        return id;
    }

    namespace audio
    {
        /**
         * @brief Any type to floating point conversion
         * int32_t requires double
         * int64_t requires long double
         */
        template <typename T, typename U, std::enable_if_t<true == std::is_floating_point_v<T> && !std::is_same<T, bool>::value && true == std::is_integral_v<U> && !std::is_same<U, bool>::value, bool> = true>
        T convert(U source, uint32_t source_bytes_per_sample = sizeof(U))
        {
            uint32_t source_bits_per_sample = 8 * source_bytes_per_sample;
            if (std::is_signed<U>())
            {
                return static_cast<T>(source) / ((static_cast<U>(1) << (source_bits_per_sample - 1)) - 1);
            }
            else
            {
                return (static_cast<T>(source - (static_cast<U>(1) << (source_bits_per_sample - 1)))) / ((1 << (source_bits_per_sample - 1)));
            }
        }

        /**
         * @brief Floating point to any type conversion
         * int32_t requires double
         * int64_t requires long double
         */
        template <typename T, typename U, std::enable_if_t<true == std::is_integral_v<T> && !std::is_same<T, bool>::value && true == std::is_floating_point_v<U> && !std::is_same<U, bool>::value, bool> = true>
        T convert(U source, uint32_t dest_bytes_per_sample = sizeof(T))
        {
            uint32_t dest_bits_per_samples = dest_bytes_per_sample * 8;
            if (std::is_signed<T>())
            {
                return static_cast<T>(source * static_cast<U>((static_cast<T>(1) << (dest_bits_per_samples - 1)) - 1));
            }
            else
            {
                return static_cast<T>(source * static_cast<U>((static_cast<T>(1) << (dest_bits_per_samples - 1)) - 1) + static_cast<U>(static_cast<T>(1) << (dest_bits_per_samples - 1)));
            }
        }

        /**
         * @brief Floating point to floating point
         */
        template <typename T, typename U, std::enable_if_t<true == std::is_floating_point_v<T> && !std::is_same<T, bool>::value && true == std::is_floating_point_v<U> && !std::is_same<U, bool>::value, bool> = true>
        T convert(U source, [[maybe_unused]] uint32_t dest_bytes_per_sample = sizeof(T))
        {
            return static_cast<T>(source);
        }
    }
}

#endif