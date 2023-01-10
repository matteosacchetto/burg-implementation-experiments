#ifndef __PRECISE_LA_HPP__
#define __PRECISE_LA_HPP__

#include <type_traits>
#include <utility>
#include <tuple>
#include <cmath>

namespace precise_la
{
    namespace sum
    {
        /**
         * @brief implementation of the TwoSum algorithm. Returns a pair with [result, error]
         * @cite Ogita-2005
         *
         * \f[
         *   function [r, e] = TwoSum(a, b)
         *                 r  = fl(a + b)
         *                 _z = fl(r - a)
         *                 e  = fl((a - (r - _z)) + (b - _z))
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param a the first number to sum
         * @param b the second number to sum
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> two_sum(T a, T b)
        {
            T r = a + b;
            T _z = r - a;
            T e = ((a - (r - _z)) + (b - _z));

            return {r, e};
        }

        /**
         * @brief implementation of the FastTwoSum algorithm. Returns a pair with [result, error].
         * As per FastTwoSum specification, this only works if |a| >= |b|
         * @cite Ogita-2005
         *
         * \f[
         *   function [r, e] = FastTwoSum(a, b)
         *                 r = fl(a + b)
         *                 e = fl((a - r) + b)
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param a the first number to sum
         * @param b the second number to sum
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> fast_two_sum(T a, T b)
        {
            T r = a + b;
            T e = ((a - r) + b);

            return {r, e};
        }

        /**
         * @brief safe implementation of the FastTwoSum algorithm. Returns a pair with [result, error].
         * As per FastTwoSum specification, this works in all cases
         * @cite Ogita-2005
         *
         * \f[
         *   function [r, e] = FastTwoSum(a, b)
         *                 r = fl(a + b)
         *                 e = fl((a - r) + b)
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param a the first number to sum
         * @param b the second number to sum
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> fast_two_sum_safe(T a, T b)
        {
            if (std::abs(a) >= std::abs(b))
            {
                return fast_two_sum(a, b);
            }
            else
            {
                return fast_two_sum(b, a);
            }
        }

        /**
         * @brief implementation of the basic sum of the elements of the array.
         *
         * \f[
         *   function [r, e] = SumBasic(x, N)
         *                r = 0
         *                e = 0
         *                for i = 0:N
         *                    r += x[i]
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param x the vector containing the elements to sum
         * @param N the number of elements in the vector
         * @param impl used to specify which implementation to use, among the available ones
         * @return std::pair<T, T>
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> sum_basic(T *x, std::size_t N)
        {
            T res = 0;

            for (std::size_t i = 0; i < N; i++)
            {
                res += x[i];
            }

            return {res, 0};
        }

        /**
         * @brief implementation of the Sum2s algorithm
         * @cite Ogita-2005
         *
         * \f[
         *   function [r, e] = Sum2s(x, N)
         *                _p = x[0]
         *                _s = 0
         *                for i = 1:N
         *                    [_p, _q] = TwoSum(_p, x[i])
         *                    _s = fl(_s + _q)
         *                r  = _p
         *                e  = _s
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param x the vector containing the elements to sum
         * @param N the number of elements to sum
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> sum_2s(T *x, std::size_t N)
        {
            T _p = x[0];
            T _s = 0;

            T _q;
            for (std::size_t i = 1; i < N; i++)
            {
                std::tie(_p, _q) = two_sum(_p, x[i]);
                _s = _s + _q;
            }

            return {_p, _s};
        }

        /**
         * @brief implementation of the SumXBLAS algorithm
         * @cite Ogita-2005
         *
         * \f[
         *   function [r, e] = Sum2s(x, N)
         *                _s = 0
         *                _t = 0
         *                for i = 0:N
         *                    [_t1, _t2] = TwoSum(_s, x[i])
         *                    _t2 = _t2 + _t
         *                    [_s, _t] = FastTwoSum(_t1, _t2)
         *                r  = _s
         *                e  = _t
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param x the vector containing the elements to sum
         * @param N the number of elements to sum
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> sum_xblas(T *x, std::size_t N)
        {
            T _s = 0;
            T _t = 0;

            for (std::size_t i = 0; i < N; i++)
            {
                auto [_t1, _t2] = two_sum(_s, x[i]);
                _t2 = _t2 + _t;
                std::tie(_s, _t) = fast_two_sum(_t1, _t2);
            }

            return {_s, _t};
        }

        enum IMPL
        {
            SUM_BASIC,
            SUM_2S,
            SUM_XBLAS
        };

        /**
         * @brief implementation of the sum the elements of the array.
         * This is a wrapper which is meat to be used to simplify the use of the specific functions,
         * since it deals with possible allocations needed by the various implementations
         *
         * @tparam T a float/double/long double type
         * @param x the vector containing the elements to sum
         * @param N the number of elements in the vector
         * @param impl used to specify which implementation to use, among the available ones
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> sum(T *x, std::size_t N, IMPL impl = IMPL::SUM_2S)
        {
            switch (impl)
            {
            case IMPL::SUM_BASIC:
                return sum_basic(x, N);
            case IMPL::SUM_2S:
                return sum_2s(x, N);
            case IMPL::SUM_XBLAS:
                return sum_xblas(x, N);
            }

            return {0, 0};
        }
    }

    namespace prod
    {
        /**
         * @brief impelemtation of the TwoProdFMA algorithm.
         * @cite Ogita-2005
         *
         * \f[
         *   function [r, e] = TwoProductFMA(a, b)
         *                 r = fl(a * b)
         *                 e = FMA(a, b, -r)
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param a the first number to multiply
         * @param b the second number to multiply
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> two_product_FMA(T a, T b)
        {
            T r = (a * b);
            T e = std::fma(a, b, -r);

            return {r, e};
        }

        /**
         * @brief Returns the result of the basic dot product as a pair [result, error]
         *
         * \f[
         *   function [r, e] = DotBasic(x, y, N)
         *                 r = 0
         *                 e = 0
         *                 for i = 0:N
         *                     r += x[i] * y[i]
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param x the array with the x elements
         * @param y the array with the y elements
         * @param N the size of both arrays (needs to be the same)
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> dot_basic(T *x, T *y, std::size_t N)
        {
            T r = 0;

            for (std::size_t i = 0; i < N; i++)
            {
                r += x[i] * y[i];
            }

            return {r, 0};
        }

        /**
         * @brief Returns the result of the dot product as a pair [result, error].
         * It implements the dot_2 algorithm
         *
         * \f[
         *   function [r, e] = Dot2(x, y, N)
         *                [_p, _s] = TwoProductFMA(x[0], y[0])
         *                for i = 1:N
         *                    [_h, _r] = TwoProductFMA(x[i], y[i])
         *                    [_p, _q] = TwoSum(_p, _h)
         *                    _s = fl(_s + (_q + _r))
         *                r = _p
         *                e = _s
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param x the array with the x elements
         * @param y the array with the y elements
         * @param N the size of both arrays (needs to be the same)
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> dot_2(T *x, T *y, std::size_t N)
        {
            auto [_p, _s] = two_product_FMA(x[0], y[0]);

            T _q;
            for (std::size_t i = 1; i < N; i++)
            {
                auto [_h, _r] = two_product_FMA(x[i], y[i]);
                std::tie(_p, _q) = sum::two_sum(_p, _h);
                _s = (_s + (_q + _r));
            }

            return {_p, _s};
        }

        /**
         * @brief Returns the result of the dot product as a pair [result, error].
         * It implements the dot_xblas algorithm
         *
         * \f[
         *   function [r, e] = DotXBLAS(x, y, N)
         *                _s = 0
         *                _t = 0
         *                for i = 0:N
         *                    [_h, _r]   = TwoProductFMA(x[i], y[i])
         *                    [_s1, _s2] = TwoSum(_s, _h)
         *                    [_t1, _t2] = TwoSum(_t, _r)
         *                    _s2 = _s2 + _t1
         *                    [_t1, _s2] = FastTwoSum(_s1, _s2)
         *                    _t2 = _t2 + _s2
         *                    [_s, _t]   = FastTwoSum(_t1, _t2)
         *
         *                r = _s
         *                e = _t
         * \f]
         *
         * @tparam T a float/double/long double type
         * @param x the array with the x elements
         * @param y the array with the y elements
         * @param N the size of both arrays (needs to be the same)
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> dot_xblas(T *x, T *y, std::size_t N)
        {
            T _s = 0, _t = 0;

            for (std::size_t i = 0; i < N; i++)
            {
                auto [_h, _r] = two_product_FMA(x[i], y[i]);
                auto [_s1, _s2] = sum::two_sum(_s, _h);
                auto [_t1, _t2] = sum::two_sum(_t, _r);
                _s2 = _s2 + _t1;
                std::tie(_t1, _s2) = sum::fast_two_sum(_s1, _s2);
                _t2 = _t2 + _s2;
                std::tie(_s, _t) = sum::fast_two_sum(_t1, _t2);
            }

            return {_s, _t};
        }

        enum IMPL
        {
            DOT_BASIC,
            DOT_2,
            DOT_XBLAS
        };

        /**
         * @brief Returns the result of the dot product as a pair [result, error].
         * This is a wrapper which is meat to be used to simplify the use of the specific functions,
         * since it deals with possible allocations needed by the various implementations
         *
         * @tparam T a float/double/long double type
         * @param x the array with the x elements
         * @param y the array with the y elements
         * @param N the size of both arrays (needs to be the same)
         * @return [result, error]
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> dot(T *x, T *y, std::size_t N, IMPL impl = IMPL::DOT_2)
        {
            switch (impl)
            {
            case IMPL::DOT_BASIC:
                return dot_basic(x, y, N);
            case IMPL::DOT_2:
                return dot_2(x, y, N);
            case IMPL::DOT_XBLAS:
                return dot_xblas(x, y, N);
            }

            return {0, 0};
        }
    }

    namespace utils
    {
        /**
         * @brief
         *
         * @tparam T a float/double/long double type
         * @param p the tuple [result, error]
         * @return result + error
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        T sum_pair_elements(const std::pair<T, T> &p)
        {
            return p.first + p.second;
        }

        /**
         * @brief
         *
         * @tparam T a float/double/long double type
         * @param p the tuple [result, error]
         * @return result + error
         */
        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> sum_pairs(const std::pair<T, T> &a, const std::pair<T, T> &b)
        {
            auto [s, e] = sum::two_sum(a.first + a.second, b.first + b.second);
            return {s, e};
        }

        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> scalar_prod_pair(T k, const std::pair<T, T> &a)
        {
            return {k * a.first, k * a.second};
        }

        template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
        std::pair<T, T> invert_pair(const std::pair<T, T> &a)
        {
            return {-a.first, -a.second};
        }
    }
}

#endif