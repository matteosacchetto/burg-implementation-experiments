#ifndef __STATISTICS_HPP__
#define __STATISTICS_HPP__

#include <vector>
#include <cstdint>
#include <random>
#include <set>
#include <stdexcept>
#include <cmath>

namespace stats
{
    static std::mt19937 gen;

    void initialize_random(uint64_t seed)
    {
        gen.seed(seed);
    }

    /**
     * @brief Generate n position
     *
     * @tparam T
     * @param min
     * @param max
     * @param n
     * @return std::vector<T>
     */
    template <typename T, std::enable_if_t<true == std::is_integral<T>(), bool> = true>
    std::vector<T> get_n_positions(uint64_t min, uint64_t max, uint64_t n, uint64_t mul_of = 1)
    {

        min = min + (mul_of - (min % mul_of)) % mul_of; // Minimum value needs to be a multiple of mul_of
        max = max - (max % mul_of);                     // Maximum value needs to be a multiple of mul_of

        uint64_t range_min = min / mul_of;
        uint64_t range_max = max / mul_of;

        std::uniform_int_distribution<> distr(range_min, range_max - 1); // define the range

        std::set<T> random_num_no_repetition;

        if (max <= min || n > range_max - range_min)
        {
            throw std::runtime_error("can not generate " + std::to_string(n) + " numbers");
        }

        for (uint64_t i = 0; i < n; ++i)
        {
            uint64_t _random_number = distr(stats::gen);
            while (random_num_no_repetition.find(_random_number * mul_of) != random_num_no_repetition.end())
            {
                _random_number = (_random_number + 1 - range_min) % (range_max - range_min) + range_min;
            }
            random_num_no_repetition.insert(_random_number * mul_of);
        }

        std::vector<T> generated_numbers;
        std::copy(random_num_no_repetition.begin(), random_num_no_repetition.end(), std::back_inserter(generated_numbers));

        return generated_numbers;
    }

    template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
    std::vector<T> ae(const std::vector<T> &v1, const std::vector<T> &v2)
    {
        std::size_t n = v1.size();
        std::vector<T> res(n);
        for (std::size_t i = 0; i < n; i++)
        {
            res[i] = std::abs(v2[i] - v1[i]);
        }

        return res;
    }

    template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
    T mae(const std::vector<T> &v1, const std::vector<T> &v2)
    {
        std::size_t n = v1.size();
        T res{};
        for (std::size_t i = 0; i < n; i++)
        {
            res += std::abs(v2[i] - v1[i]);
        }

        return res / n;
    }

    template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
    T rmse(const std::vector<T> &v1, const std::vector<T> &v2, bool squared = false)
    {
        std::size_t n = v1.size();
        T res{};
        for (std::size_t i = 0; i < n; i++)
        {
            res += std::pow((v2[i] - v1[i]), 2);
        }

        return squared ? res / n : std::sqrt(res / n);
    }
}

#endif