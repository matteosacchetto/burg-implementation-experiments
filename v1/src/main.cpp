#include "burg_basic.hpp"
#include "burg_optimized_den.hpp"
#include "burg_optimized_den_sqrt.hpp"
#include "compensated_burg_basic.hpp"
#include "compensated_burg_optimized_den.hpp"
#include "compensated_burg_optimized_den_sqrt.hpp"
#include "timer.hpp"
#include <algorithm>

// #define PRINT

int main()
{
    compensated_burg_optimized_den_sqrt<double> ar(2048);
    std::vector<double> samples(2048);
    // for (std::size_t i = 0; i < samples.size(); ++i)
    // {
    //     samples[i] = 0.1 * i;
    // }
    measure::timer t;

    double sample_rate = 44100;
    double freq = 441;
    std::generate(samples.begin(), samples.end(), [sample_rate, freq, n = 0]() mutable
                  { auto r = std::sin((n * 2 * M_PI) / (sample_rate / freq));
                n++;
                return r; });

#ifdef PRINT
    {
        std::stringstream ss;
        ss << "[" << __FUNCTION__ << "] - "
           << "Samples: "
           << "\n"
           << std::setprecision(type_precision<double>()) << std::scientific
           << "  - samples: [";
        for (std::size_t i = 0; i < samples.size(); ++i)
        {
            ss << (i > 0 ? ", " : "") << samples[i];
        }

        ss << "]" << std::endl;

        logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    }
#endif

    t.start();
    auto a = ar.fit(samples, 64);
    t.stop();

    {
        std::stringstream ss;
        ss << "[" << __FUNCTION__ << "] - "
           << "Duration [fit]: " << t.get_duration_in_ns() << " ns";

        if (t.get_duration_in_ns() > 2000000)
        {
            ss << " [TOO SLOW, since > 2ms]" << std::endl;
            logger::warning(ss.str());
        }
        else
        {
            ss << std::endl;
            logger::info(ss.str());
        }
    }

#ifdef PRINT
    {
        std::stringstream ss;
        ss << "[" << __FUNCTION__ << "] - "
           << "BURG's AR fitted params: "
           << "\n"
           << std::setprecision(type_precision<double>()) << std::scientific
           << "  - A coefficients: [";
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            ss << (i > 0 ? ", " : "") << a[i];
        }

        ss << "]" << std::endl;

        logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    }
#endif

    t.start();
    auto pred = ar.predict(samples, a, 2048);
    t.stop();

    {
        std::stringstream ss;
        ss << "[" << __FUNCTION__ << "] - "
           << "Duration [predict]: " << t.get_duration_in_ns() << " ns";

        if (t.get_duration_in_ns() > 2000000)
        {
            ss << " [TOO SLOW, since > 2ms]" << std::endl;
            logger::warning(ss.str());
        }
        else
        {
            ss << std::endl;
            logger::info(ss.str());
        }
    }

#ifdef PRINT
    {
        std::stringstream ss;
        ss << "[" << __FUNCTION__ << "] - "
           << "BURG's predicted elements: "
           << "\n"
           << std::setprecision(type_precision<double>()) << std::scientific
           << "  - predicted elements: [";
        for (std::size_t i = 0; i < pred.size(); ++i)
        {
            ss << (i > 0 ? ", " : "") << pred[i];
        }

        ss << "]" << std::endl;

        logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    }
#endif

    auto max_s = std::max_element(samples.begin(), samples.end(), [](auto a, auto b) {
        return std::fabs(a) < std::fabs(b);
    });

    auto max_p = std::max_element(pred.begin(), pred.end(), [](auto a, auto b) {
        return std::fabs(a) < std::fabs(b);
    });

    {
        std::stringstream ss;
        ss << "[" << __FUNCTION__ << "] - "
           << "Maximum (abs) values: "
           << "\n"
           << std::setprecision(type_precision<double>()) << std::scientific
           << "  - samples:   " << *max_s << "\n"
           << "  - prediction " << *max_p << std::endl;


        logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    }

    return 0;
}