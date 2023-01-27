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
#include <nlohmann/json.hpp>

// #define PRINT

using data_type = double;
using ar = burg_basic<data_type>;

int main()
{
    try
    {
        uint32_t test_size = 128;
        std::vector<uint32_t> train_sizes{512, 1024, 2048, 4096, 8192};
        std::vector<uint32_t> lag_values{1, 2, 4, 8, 16, 32, 64, 128};
        uint32_t num_positions = 100;

        // seed rand
        stats::initialize_random(1);

        // nlohmann::ordered_json results = nlohmann::ordered_json::array();
        uint64_t index{};

        for (const auto &entry : std::filesystem::recursive_directory_iterator("samples-convert"))
        {
            if (entry.is_regular_file() && utils::string::tolower(entry.path().extension()).compare(".wav") == 0)
            {
                const std::string filepath = entry.path();
                std::string processed_filepath = utils::string::change_first_dir(entry.path(), "samples-convert-processed");

                logger::info(filepath);
                // logger::info(processed_filepath);

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
                            auto a_coeff = ar_model.fit(train_set, lag);
                            ar_timer.stop();

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
                                                     {"ar_fit_time", ar_fit_time},
                                                     {"ar_predict_time", ar_predict_time},
                                                     {"total_count", num_positions}});
                    }
                }

                if (index == 0)
                {
                    std::cout << ",file,results,b0,b1" << std::endl;
                }

                std::cout << index << "," << std::string(result["file"]) << "," << result["results"].dump() << "," << result["b0"].dump() << "," << result["b1"].dump() << std::endl;
                index++;

                // const auto processed_path = std::filesystem::path(processed_filepath);
                // if (!processed_path.parent_path().empty())
                // {
                //     std::filesystem::create_directories(processed_path.parent_path());
                // }

                // wav_file<double> processed_wav{processed_filepath};
                // processed_wav.write_file(std::vector<std::vector<double>>{processed_samples}, wav.sample_rate, wav.sample_type);
            }
        }

        // Write output file
        // wav_file<double> wav1{"a1.wav"};
        // wav1.write_file(std::vector<std::vector<double>>{samples}, wav.sample_rate, sample_type_enum::FLOAT);
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
    //     compensated_burg_optimized_den_sqrt<double> ar(2048);
    //     std::vector<double> samples(2048);
    //     // for (std::size_t i = 0; i < samples.size(); ++i)
    //     // {
    //     //     samples[i] = 0.1 * i;
    //     // }
    //     measure::timer t;

    //     double sample_rate = 44100;
    //     double freq = 441;
    //     std::generate(samples.begin(), samples.end(), [sample_rate, freq, n = 0]() mutable
    //                   { auto r = std::sin((n * 2 * M_PI) / (sample_rate / freq));
    //                 n++;
    //                 return r; });

    // #ifdef PRINT
    //     {
    //         std::stringstream ss;
    //         ss << "[" << __FUNCTION__ << "] - "
    //            << "Samples: "
    //            << "\n"
    //            << std::setprecision(type_precision<double>()) << std::scientific
    //            << "  - samples: [";
    //         for (std::size_t i = 0; i < samples.size(); ++i)
    //         {
    //             ss << (i > 0 ? ", " : "") << samples[i];
    //         }

    //         ss << "]" << std::endl;

    //         logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    //     }
    // #endif

    //     t.start();
    //     auto a = ar.fit(samples, 64);
    //     t.stop();

    //     {
    //         std::stringstream ss;
    //         ss << "[" << __FUNCTION__ << "] - "
    //            << "Duration [fit]: " << t.get_duration_in_ns() << " ns";

    //         if (t.get_duration_in_ns() > 2000000)
    //         {
    //             ss << " [TOO SLOW, since > 2ms]" << std::endl;
    //             logger::warning(ss.str());
    //         }
    //         else
    //         {
    //             ss << std::endl;
    //             logger::info(ss.str());
    //         }
    //     }

    // #ifdef PRINT
    //     {
    //         std::stringstream ss;
    //         ss << "[" << __FUNCTION__ << "] - "
    //            << "BURG's AR fitted params: "
    //            << "\n"
    //            << std::setprecision(type_precision<double>()) << std::scientific
    //            << "  - A coefficients: [";
    //         for (std::size_t i = 0; i < a.size(); ++i)
    //         {
    //             ss << (i > 0 ? ", " : "") << a[i];
    //         }

    //         ss << "]" << std::endl;

    //         logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    //     }
    // #endif

    //     t.start();
    //     auto pred = ar.predict(samples, a, 2048);
    //     t.stop();

    //     {
    //         std::stringstream ss;
    //         ss << "[" << __FUNCTION__ << "] - "
    //            << "Duration [predict]: " << t.get_duration_in_ns() << " ns";

    //         if (t.get_duration_in_ns() > 2000000)
    //         {
    //             ss << " [TOO SLOW, since > 2ms]" << std::endl;
    //             logger::warning(ss.str());
    //         }
    //         else
    //         {
    //             ss << std::endl;
    //             logger::info(ss.str());
    //         }
    //     }

    // #ifdef PRINT
    //     {
    //         std::stringstream ss;
    //         ss << "[" << __FUNCTION__ << "] - "
    //            << "BURG's predicted elements: "
    //            << "\n"
    //            << std::setprecision(type_precision<double>()) << std::scientific
    //            << "  - predicted elements: [";
    //         for (std::size_t i = 0; i < pred.size(); ++i)
    //         {
    //             ss << (i > 0 ? ", " : "") << pred[i];
    //         }

    //         ss << "]" << std::endl;

    //         logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    //     }
    // #endif

    //     auto max_s = std::max_element(samples.begin(), samples.end(), [](auto a, auto b) {
    //         return std::fabs(a) < std::fabs(b);
    //     });

    //     auto max_p = std::max_element(pred.begin(), pred.end(), [](auto a, auto b) {
    //         return std::fabs(a) < std::fabs(b);
    //     });

    //     {
    //         std::stringstream ss;
    //         ss << "[" << __FUNCTION__ << "] - "
    //            << "Maximum (abs) values: "
    //            << "\n"
    //            << std::setprecision(type_precision<double>()) << std::scientific
    //            << "  - samples:   " << *max_s << "\n"
    //            << "  - prediction " << *max_p << std::endl;

    //         logger::info(ss.str(), sizeof(__FUNCTION__) + 2);
    //     }

    return 0;
}