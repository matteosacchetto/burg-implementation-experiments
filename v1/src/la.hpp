#ifndef __LA_HPP__
#define __LA_HPP__

#include <type_traits>
#include <utility>
#include <tuple>
#include <cmath>

namespace la
{
    namespace sum
    {
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        T sum_basic(T *x, std::size_t N)
        {
            T res = 0;

            for (std::size_t i = 0; i < N; i++)
            {
                res += x[i];
            }

            return res;
        }
    }

    namespace prod
    {
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        T dot_basic(T *x, T *y, std::size_t N)
        {
            T r = 0;

            for (std::size_t i = 0; i < N; i++)
            {
                r += x[i] * y[i];
            }

            return r;
        }
    }

}

#endif // __LA_HPP__