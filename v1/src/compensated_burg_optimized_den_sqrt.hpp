#ifndef __COMPENSATED_BURG_OPTIMIZED_DEN_SQRT_HPP__
#define __COMPENSATED_BURG_OPTIMIZED_DEN_SQRT_HPP__

#include <type_traits>
#include <vector>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <iomanip>
#include "type_details.hpp"
#include "logger.hpp"
#include "precise_la.hpp"

template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
class compensated_burg_optimized_den_sqrt
{
private:
    std::size_t max_size;
    std::size_t max_order;

    std::vector<T> f;
    std::vector<T> b;

public:
    compensated_burg_optimized_den_sqrt(const std::size_t max_size) : max_size{max_size}, max_order{max_size - 1}, f(max_size), b(max_size)
    {
#ifdef DEBUG
        assert(max_size > 0);

        {
            std::stringstream s;

            s << "[" << __FUNCTION__ << "] - "
              << "Initialization of BURG's AR model <" << type_name<T>() << ">:"
              << "\n"
              << "  - max size: " << max_size << "\n"
              << "  - f size:   " << f.size() << "\n"
              << "  - b size:   " << b.size() << std::endl;

            logger::info(s.str(), sizeof(__FUNCTION__) + 2);
        }
#endif
    };

    ~compensated_burg_optimized_den_sqrt()
    {
#ifdef DEBUG
        std::stringstream s;
        s << "[" << __FUNCTION__ << "] - "
          << "Destruction of BURG's AR model " << std::endl;

        logger::info(s.str(), sizeof(__FUNCTION__) + 2);

#endif
    }

    std::pair<std::vector<T>, T> fit(std::vector<T> &samples, std::size_t order)
    {
#ifdef DEBUG
        assert(order > 0);
        assert(samples.size() > 0);
#endif

        // Let's find the actual sample size and order
        std::size_t actual_size = std::min(samples.size(), max_size);
        std::size_t samples_start = samples.size() - actual_size;
        std::size_t actual_order = std::min(order, max_order);
        std::size_t switching_point = std::max((std::size_t) std::sqrt(actual_order), 8UL);

#ifdef DEBUG
        {
            std::stringstream s;

            s << "[" << __FUNCTION__ << "] - "
              << "Initialization of BURG's AR fit params: "
              << "\n"
              << "  - actual size:   " << actual_size << "\n"
              << "  - sample range:  [" << samples_start << ", " << samples_start + actual_size << ")"
              << "\n"
              << "  - actual order:  " << actual_order << std::endl;

            logger::info(s.str(), sizeof(__FUNCTION__) + 2);
        }
#endif

        // Initialize f and b
        std::copy(samples.cbegin() + samples_start, samples.cend(), f.begin());
        std::copy(samples.cbegin() + samples_start, samples.cend(), b.begin());

        // Alloc vector with AR coefficients
        std::vector<T> a(actual_order + 1);
        a[0] = 1.; // As per burg's specifications

        // Initialize burg methods variables
        T ki = 0.;  // K at i iteration
        T num = 0.; // Numerator
        T den = 0.; // Denominator
        T err = 0.; // Error

        err = precise_la::utils::sum_pair_elements(precise_la::prod::dot_2(&samples.data()[samples_start], &samples.data()[samples_start], actual_size));
        den = 2. * err;

#ifdef DEBUG
        std::stringstream ss1;
        ss1 << "[" << __FUNCTION__ << "] - "
            << "Main loop:"
            << "\n";
#endif

        // AR main loop
        for (std::size_t i = 1; i <= actual_order; ++i)
        {
            // Numerator
            num = precise_la::utils::sum_pair_elements(precise_la::prod::dot_2(&b.data()[0], &f.data()[i], actual_size - i));
            num = precise_la::utils::sum_pair_elements(precise_la::prod::two_product_FMA(num, static_cast<T>(-2.)));

            // Denominator
            if (i > switching_point)
            {
                auto f_2 = precise_la::prod::two_product_FMA(f[i - 1], -f[i - 1]);
                auto b_2 = precise_la::prod::two_product_FMA(b[actual_size - i], -b[actual_size - i]);
                auto den_1 = precise_la::prod::two_product_FMA(den, precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs({1, 0}, precise_la::prod::two_product_FMA(ki, -ki))));
                den = precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs(den_1, precise_la::utils::sum_pairs(f_2, b_2)));
            }
            else
            {
                den = precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs(precise_la::prod::dot_2(&f.data()[i], &f.data()[i], actual_size - i), precise_la::prod::dot_2(&b.data()[0], &b.data()[0], actual_size - i)));
            }

            if (den == 0)
            {
                den = std::numeric_limits<T>::epsilon();
            }

            ki = precise_la::utils::sum_pair_elements(precise_la::prod::two_product_FMA(num, 1 / den));

            for (std::size_t j = i; j < actual_size; j++)
            {
                T bj = b[j - i];
                T fj = f[j];

                b[j - i] = precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs({bj, 0}, precise_la::prod::two_product_FMA(ki, fj)));
                f[j] = precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs({fj, 0}, precise_la::prod::two_product_FMA(ki, bj)));
            }

            for (std::size_t j = 1; j <= i / 2; j++)
            {
                T aj = a[j];
                T anj = a[i - j];

                a[j] = precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs({aj, 0}, precise_la::prod::two_product_FMA(ki, anj)));
                a[i - j] = precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs({anj, 0}, precise_la::prod::two_product_FMA(ki, aj)));
            }
            a[i] = ki;

            err = precise_la::utils::sum_pair_elements(precise_la::prod::two_product_FMA(err, precise_la::utils::sum_pair_elements(precise_la::utils::sum_pairs({1, 0}, precise_la::prod::two_product_FMA(ki, -ki)))));

#ifdef DEBUG
            {
                if (ki >= 1)
                {
                    std::stringstream s;
                    s << "[" << __FUNCTION__ << "] - "
                      << "K >=1 !! (" << i << ")"
                      << "\n"
                      << std::setprecision(type_precision<T>()) << std::scientific
                      << "    - K:   " << ki << std::endl;

                    logger::error(s.str());
                }

                ss1 << "  - "
                    << "Partial results (" << i << ")"
                    << "\n"
                    << std::setprecision(type_precision<T>()) << std::scientific
                    << "    - K:   " << ki << "\n"
                    << "    - err: " << err << std::endl;
            }
#endif
        }

#ifdef DEBUG
        {
            logger::info(ss1.str(), sizeof(__FUNCTION__) + 2);

            std::stringstream s;

            s << "[" << __FUNCTION__ << "] - "
              << "BURG's AR fitted params: "
              << "\n"
              << std::setprecision(type_precision<T>()) << std::scientific
              << "  - A coefficients: [";

            for (std::size_t i = 0; i < a.size(); ++i)
            {
                s << (i > 0 ? ", " : "") << a[i];
            }

            s << "]" << std::endl;

            logger::info(s.str(), sizeof(__FUNCTION__) + 2);
        }

#endif

        // Return coefficients
        return {a, err};
    }

    std::vector<T> predict(std::vector<T> &samples, std::vector<T> &a, std::size_t n)
    {
        std::vector<T> predictions(n);
        std::vector<T> section(a.size() - 1);

        for (ssize_t i = 0; i < static_cast<ssize_t>(n); i++)
        {
            for (ssize_t j = 1; j < static_cast<ssize_t>(a.size()); j++)
            {
                section[j - 1] = -(i - j < 0 ? static_cast<T>(samples[samples.size() + i - j]) : predictions[i - j]);
            }

            predictions[i] = precise_la::utils::sum_pair_elements(precise_la::prod::dot_2(&section.data()[0], &a.data()[1], a.size() - 1));
        }

#ifdef DEBUG
        {
            std::stringstream s;

            s << "[" << __FUNCTION__ << "] - "
              << "BURG's AR predicted samples: "
              << "\n"
              << std::setprecision(type_precision<T>()) << std::scientific
              << "  - predicted samples: [";

            for (std::size_t i = 0; i < predictions.size(); ++i)
            {
                s << (i > 0 ? ", " : "") << predictions[i];
            }

            s << "]" << std::endl;

            logger::info(s.str(), sizeof(__FUNCTION__) + 2);
        }

#endif

        return predictions;
    }
};

#endif