#include "burg_basic.hpp"
#include "burg_optimized_den.hpp"
#include "burg_optimized_den_sqrt.hpp"
#include "compensated_burg_basic.hpp"
#include "compensated_burg_optimized_den.hpp"
#include "compensated_burg_optimized_den_sqrt.hpp"
#include "timer.hpp"
#include "wav.hpp"
#include "utils.hpp"
#include "statistic.hpp"
#include "timer.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <regex>
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
        uint32_t test_size = 128;
        std::vector<uint32_t> train_sizes{512, 1024, 2048, 4096, 8192};
        std::vector<uint32_t> lag_values{1, 2 , 4, 8, 16, 32, 64, 128};
        uint32_t num_positions = 100;

        // seed rand
        stats::initialize_random(1);

        uint64_t index{};

        for (const auto &entry : std::filesystem::recursive_directory_iterator("dataset"))
        {
            if (entry.is_regular_file() && utils::string::tolower(entry.path().extension()).compare(".wav") == 0)
            {
                const std::string filepath = entry.path();
#ifdef SAVE_FILE
                std::string processed_filepath = utils::string::change_first_dir(entry.path(), "samples-convert-processed");
#endif

                logger::info(filepath);

#ifdef SAVE_FILE
                logger::info(processed_filepath);
#endif

                wav_file<data_type> wav{filepath};
                wav.read_file();

                std::vector<data_type> samples = wav.data_samples[0];
                std::vector<data_type> processed_samples(wav.data_samples[0]);

                std::vector<uint64_t> positions = stats::get_n_positions<uint64_t>(*std::max_element(train_sizes.begin(), train_sizes.end()), samples.size() - test_size, num_positions, test_size);

#ifdef PRINT
                logger::info(utils::io::vector_to_string(positions));
#endif
                nlohmann::ordered_json result = {
                    {"file", filepath},
                    {"results", std::vector<nlohmann::ordered_json>()},
                    {"positions", positions},
                    {"b0", {{"mae", std::vector<data_type>()}, {"rmse", std::vector<data_type>()}}},
                    {"b1", {{"mae", std::vector<data_type>()}, {"rmse", std::vector<data_type>()}}},
                };

                // Benchmark loop
                for (auto pos : positions)
                {
                    std::vector<data_type> test_set(samples.begin() + pos, samples.begin() + pos + test_size);
                    std::vector<data_type> silence(test_size, 0);
                    std::vector<data_type> previous_packet(samples.begin() + pos - test_size, samples.begin() + pos);

                    // Benchmark 0
                    data_type b0_mae = stats::mae(test_set, silence);
                    result["b0"]["mae"].push_back(b0_mae);

                    data_type b0_rmse = stats::rmse(test_set, silence);
                    result["b0"]["rmse"].push_back(b0_rmse);

                    // Benchmark 1
                    data_type b1_mae = stats::mae(test_set, previous_packet);
                    result["b1"]["mae"].push_back(b1_mae);

                    data_type b1_rmse = stats::rmse(test_set, previous_packet);
                    result["b1"]["rmse"].push_back(b1_rmse);
                }

                // For each train size
                for (auto train_size : train_sizes)
                {
                    // For each lag value
                    for (auto lag : lag_values)
                    {
                        std::vector<data_type> ar_mae;
                        std::vector<data_type> ar_rmse;
                        std::vector<data_type> ar_err;
                        std::vector<double> ar_fit_time;
                        std::vector<double> ar_predict_time;
                        measure::timer ar_timer{};

                        // For each position
                        for (auto pos : positions)
                        {
                            std::vector<data_type> train_set(samples.begin() + pos - train_size, samples.begin() + pos);
                            std::vector<data_type> test_set(samples.begin() + pos, samples.begin() + pos + test_size);

                            ar ar_model{train_size};
                            ar_timer.start();
                            auto [a_coeff, err] = ar_model.fit(train_set, lag);
                            ar_timer.stop();

                            ar_err.push_back(err);
                            ar_fit_time.push_back(ar_timer.get_duration_in_ns());

                            ar_timer.start();
                            auto predictions = ar_model.predict(train_set, a_coeff, test_size);
                            ar_timer.stop();

                            ar_predict_time.push_back(ar_timer.get_duration_in_ns());

                            std::copy(predictions.begin(), predictions.end(), processed_samples.begin() + pos);

                            data_type predictions_mae = stats::mae(test_set, predictions);
                            ar_mae.push_back(predictions_mae);

                            data_type predictions_rmse = stats::rmse(test_set, predictions);
                            ar_rmse.push_back(predictions_rmse);
                        }

                        result["results"].push_back({{"train_size", train_size},
                                                     {"lag", lag},
                                                     {"ar_mae", ar_mae},
                                                     {"ar_rmse", ar_rmse},
                                                     {"ar_error", ar_err},
                                                     {"ar_fit_time", ar_fit_time},
                                                     {"ar_predict_time", ar_predict_time},
                                                     {"total_count", num_positions}});
                    }
                }

                if (index == 0)
                {
                    std::cout << ",file,results,b0,b1" << std::endl;
                }

                std::cout << index << "," << std::string(result["file"]) << ","
                          << "\"" << std::regex_replace(result["results"].dump(), std::regex("\""), "\'") << "\""
                          << ","
                          << "\"" << std::regex_replace(result["b0"].dump(), std::regex("\""), "\'") << "\""
                          << ","
                          << "\"" << std::regex_replace(result["b1"].dump(), std::regex("\""), "\'") << "\"" << std::endl;
                index++;

#ifdef SAVE_FILE
                const auto processed_path = std::filesystem::path(processed_filepath);
                if (!processed_path.parent_path().empty())
                {
                    std::filesystem::create_directories(processed_path.parent_path());
                }

                wav_file<double> processed_wav{processed_filepath};
                processed_wav.write_file(std::vector<std::vector<double>>{processed_samples}, wav.sample_rate, wav.sample_type);
#endif
            }
        }
    }
    catch (std::exception &e)
    {
        logger::error(e.what());
    }
    return 0;
}