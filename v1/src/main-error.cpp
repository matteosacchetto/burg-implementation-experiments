#include "burg_basic.hpp"
#include "burg_optimized_den.hpp"
#include "burg_optimized_den_sqrt.hpp"
#include "compensated_burg_basic.hpp"
#include "compensated_burg_optimized_den.hpp"
#include "compensated_burg_optimized_den_sqrt.hpp"
#include "utils.hpp"
#include "statistic.hpp"
#include "timer.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <nlohmann/json.hpp>

// #define PRINT
// #define SAVE_FILE

#if defined(USE_DOUBLE)
using data_type = double;
#elif defined(USE_LONG_DOUBLE)
using data_type = long double;
#else
using data_type = double;
#endif

#if defined(BURG_BASIC)
using ar = burg_basic<data_type>;
#elif defined(BURG_OPT_DEN)
using ar = burg_optimized_den<data_type>;
#elif defined(BURG_OPT_DEN_SQRT)
using ar = burg_optimized_den_sqrt<data_type>;
#elif defined(BURG_COMP_BASIC)
using ar = compensated_burg_basic<data_type>;
#elif defined(BURG_COMP_OPT_DEN)
using ar = compensated_burg_optimized_den<data_type>;
#elif defined(BURG_COMP_OPT_DEN_SQRT)
using ar = compensated_burg_optimized_den_sqrt<data_type>;
#else
using ar = burg_basic<data_type>;
#endif

int main()
{
    try
    {
        uint32_t test_size = 128*20;
        std::vector<uint32_t> train_sizes{512, 1024, 2048, 4096, 8192};
        std::vector<uint32_t> lag_values{1, 2, 4, 8, 16, 32, 64, 128};

        uint32_t max_train_size = *std::max_element(train_sizes.begin(), train_sizes.end());
        uint32_t pos = max_train_size; // Have each model predict starting from the same position
        uint64_t size = max_train_size + test_size;

        data_type frequency = 2000;
        data_type sample_rate = 44100;
        std::vector<data_type> samples(size);

        // Generate sinewave
        for (uint i = 0; i < size; ++i)
        {
            samples[i] = std::sin(M_PI * 2 * i / (sample_rate / frequency));
        }

        nlohmann::ordered_json result = nlohmann::ordered_json::array();

        // For each train size
        for (auto train_size : train_sizes)
        {
            // For each lag value
            for (auto lag : lag_values)
            {
                std::vector<data_type> train_set(samples.begin() + pos - train_size, samples.begin() + pos);
                std::vector<data_type> test_set(samples.begin() + pos, samples.begin() + pos + test_size);

                ar ar_model{train_size};
                auto [a_coeff, err] = ar_model.fit(train_set, lag);
                auto predictions = ar_model.predict(train_set, a_coeff, test_size);

                std::vector<data_type> ar_ae = stats::ae(test_set, predictions);

                result.push_back({{"train_size", train_size},
                                  {"lag", lag},
                                  {"ar_ae", ar_ae},
                                  {"prediction", predictions},
                                  {"max", std::abs(*std::max_element(predictions.begin(), predictions.end(), [](data_type a, data_type b)
                                                                     { return std::abs(a) < std::abs(b); }))}});
            }
        }

        std::cout << result.dump() << std::endl;
    }
    catch (std::exception &e)
    {
        logger::error(e.what());
    }
    return 0;
}
