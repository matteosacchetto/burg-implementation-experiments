#ifndef __BURG_BASIC_HPP__
#define __BURG_BASIC_HPP__

#include <type_traits>
#include <vector>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <iomanip>
#include "type_details.hpp"
#include "logger.hpp"
#include "la.hpp"

template <typename T, std::enable_if_t<true == std::is_floating_point<T>(), bool> = true>
class burg_basic
{
private:
    std::size_t max_size;
    std::size_t max_order;

    std::vector<T> f;
    std::vector<T> b;

public:
    burg_basic(const std::size_t max_size) : max_size{max_size}, max_order{max_size - 1}, f(max_size), b(max_size)
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

    ~burg_basic()
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
        T ki = 0.;                                                                                                // K at i iteration
        T num = 0.;                                                                                               // Numerator
        T den = 0.;                                                                                               // Denominator
        T err = la::prod::dot_basic(&samples.data()[samples_start], &samples.data()[samples_start], actual_size); // Error

#ifdef DEBUG
        std::stringstream ss1;
        ss1 << "[" << __FUNCTION__ << "] - "
            << "Main loop:"
            << "\n";
#endif

        // AR main loop
        for (std::size_t i = 1; i <= actual_order; ++i)
        {
            num = -2 * la::prod::dot_basic(&b.data()[0], &f.data()[i], actual_size - i);
            den = la::prod::dot_basic(&f.data()[i], &f.data()[i], actual_size - i) + la::prod::dot_basic(&b.data()[0], &b.data()[0], actual_size - i);

            if (den == 0)
            {
                den = std::numeric_limits<T>::epsilon();
            }

            ki = num / den;

            for (std::size_t j = i; j < actual_size; j++)
            {
                T bj = b[j - i];
                T fj = f[j];

                b[j - i] = bj + ki * fj;
                f[j] = fj + ki * bj;
            }

            for (std::size_t j = 1; j <= i / 2; j++)
            {
                T aj = a[j];
                T anj = a[i - j];

                a[j] = aj + ki * anj;
                a[i - j] = anj + ki * aj;
            }
            a[i] = ki;

            err = err * (1 - ki * ki);

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

            predictions[i] = la::prod::dot_basic(&section.data()[0], &a.data()[1], a.size() - 1);
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