#ifndef __WAV_HPP__
#define __WAV_HPP__

// Reference: http://soundfile.sapp.org/doc/WaveFormat/
//            https://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
//            https://tech.ebu.ch/docs/tech/tech3285.pdf

// Wave formats
// int:
//   - 8bit  -> uint8_t [0, 255]
//   - 16bit -> int16_t [-32768, 32767]
//   - 16bit -> int32_t [-16777216, 16777215]
//   - 32bit -> int32_t [-2147483648, 2147483647]
//   - 64bit -> int64_t [-9223372036854775808, 9223372036854775807]
//
// float
//   - 32bit -> float [-1.0, 1.0]
//   - 64bit -> double [-1.0, 1.0]

// NOTE: does not perform accurate validation

#include <utility>
#include <string>
#include <vector>
#include <optional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "utils.hpp"
#include "logger.hpp"

enum sample_type_enum
{
    UINT8,
    SINT16,
    SINT24,
    SINT32,
    SINT64,
    FLOAT,
    DOUBLE
};

enum audio_format_enum
{
    PCM_DATA = 1,
    FLOAT_DATA = 3,
};

template <typename T, std::enable_if_t<true == std::is_floating_point_v<T> && !std::is_same<T, bool>::value, bool> = true>
class wav_file
{
private:
    struct riff
    {
        uint32_t chunk_id{};   // "RIFF" in ASCI (0x46464952 in 'little-endian')
        uint32_t chunk_size{}; // Size in bytes (does not include chunk_id and chunk_size in the count) 4 + (8 + subchunk1_size) + (8 + subchunk2_size)
        uint32_t format{};     // "WAVE" in ASCI (0x45564157 in 'little-ednian')

        riff() {}
        riff(uint32_t chunk_size) : chunk_id{0x46464952}, chunk_size{chunk_size}, format{0x45564157} {}

        std::string str() const
        {
            std::stringstream out;

            out << "RIFF Header:"
                << '\n'
                << "  - ChunkID:     " << utils::uint32_to_string(this->chunk_id) << '\n'
                << "  - ChunkSize:   " << this->chunk_size << '\n'
                << "  - Format:      " << utils::uint32_to_string(this->format) << '\n'
                << std::endl;

            return out.str();
        }

        friend std::istream &operator>>(std::ifstream &in, riff &riff_header)
        {
            in.read(reinterpret_cast<char *>(&riff_header.chunk_id), sizeof(riff_header.chunk_id));
            in.read(reinterpret_cast<char *>(&riff_header.chunk_size), sizeof(riff_header.chunk_size));
            in.read(reinterpret_cast<char *>(&riff_header.format), sizeof(riff_header.format));

            if (riff_header.chunk_id != 0x46464952)
            {
                throw std::runtime_error("RIFF header missing");
            }

            if (riff_header.format != 0x45564157)
            {
                throw std::runtime_error("WAVE header missing");
            }

            return in;
        }

        friend std::ostream &operator<<(std::ostream &out, const riff &riff_header)
        {
            out.write(reinterpret_cast<const char *>(&riff_header.chunk_id), sizeof(riff_header.chunk_id));
            out.write(reinterpret_cast<const char *>(&riff_header.chunk_size), sizeof(riff_header.chunk_size));
            out.write(reinterpret_cast<const char *>(&riff_header.format), sizeof(riff_header.format));

            return out;
        }
    };

    struct fmt
    {
        uint32_t chunk_id{};               // "fmt " in ASCII (0x20746d66 in 'little-endian')
        uint32_t chunk_size{};             // 16 for PCM (size of the remaining fields in the subchunk)
        uint16_t audio_format{};           // 1 for PCM, 3 for FLOAT. Other values are not supported
        uint16_t num_channels{};           // Number of channels (INTERLEAVED): 1 = Mono, 2 = Stereo, ...
        uint32_t sample_rate{};            // Sample rate
        uint32_t byte_rate{};              // SampleRate * NumChannels * BitsPerSample/8
        uint16_t block_align{};            // NumChannels * BitsPerSample/8. The number of bytes for one sample including all channels
        uint16_t bits_per_sample{};        // bit depth (8 bit, 16 bit, ...)
        std::optional<uint16_t> cb_size{}; // Size of the extension (for NON-PCM ONLY)

        fmt() {}
        fmt(audio_format_enum format, uint16_t num_channels, uint32_t sample_rate, uint16_t bit_depth, std::optional<uint16_t> cb_size = {})
            : chunk_id{0x20746d66},
              chunk_size{format == audio_format_enum::PCM_DATA ? 16u : 18u},
              audio_format{format},
              num_channels{num_channels},
              sample_rate{sample_rate},
              byte_rate{sample_rate * num_channels * bit_depth / 8},
              block_align{static_cast<uint16_t>(num_channels * bit_depth / 8u)},
              bits_per_sample{bit_depth},
              cb_size{cb_size}
        {
        }

        std::string str() const
        {
            std::stringstream out;

            out << "FMT Header:"
                << '\n'
                << "  - ChunkID:     " << utils::uint32_to_string(this->chunk_id) << '\n'
                << "  - ChunkSize:   " << this->chunk_size << '\n'
                << "  - AudioFormat: " << this->audio_format << '\n'
                << "  - NumChannels: " << this->num_channels << '\n'
                << "  - SampleRate:  " << this->sample_rate << " Hz" << '\n'
                << "  - BitRate:     " << this->byte_rate * 8 / 1000 << " kbps" << '\n'
                << "  - BlockAlign:  " << this->block_align << '\n'
                << "  - SampleFormat " << this->bits_per_sample << '\n'
                << std::endl;

            return out.str();
        }

        friend std::istream &operator>>(std::ifstream &in, fmt &fmt_header)
        {
            in.read(reinterpret_cast<char *>(&fmt_header.chunk_id), sizeof(fmt_header.chunk_id));
            in.read(reinterpret_cast<char *>(&fmt_header.chunk_size), sizeof(fmt_header.chunk_size));
            in.read(reinterpret_cast<char *>(&fmt_header.audio_format), sizeof(fmt_header.audio_format));
            in.read(reinterpret_cast<char *>(&fmt_header.num_channels), sizeof(fmt_header.num_channels));
            in.read(reinterpret_cast<char *>(&fmt_header.sample_rate), sizeof(fmt_header.sample_rate));
            in.read(reinterpret_cast<char *>(&fmt_header.byte_rate), sizeof(fmt_header.byte_rate));
            in.read(reinterpret_cast<char *>(&fmt_header.block_align), sizeof(fmt_header.block_align));
            in.read(reinterpret_cast<char *>(&fmt_header.bits_per_sample), sizeof(fmt_header.bits_per_sample));

            if (fmt_header.chunk_id != 0x20746d66)
            {
                throw std::runtime_error("fmt header missing");
            }

            if (fmt_header.audio_format == audio_format_enum::PCM_DATA)
            {
                // PCM
                return in;
            }
            else if (fmt_header.audio_format == audio_format_enum::FLOAT_DATA)
            {
                // FLOAT
                in.read(reinterpret_cast<char *>(&(*fmt_header.cb_size)), sizeof(*fmt_header.cb_size));
                return in;
            }

            throw std::runtime_error("Format " + std::to_string(fmt_header.audio_format) + " not supported");
        }

        friend std::ostream &operator<<(std::ostream &out, const fmt &fmt_header)
        {
            out.write(reinterpret_cast<const char *>(&fmt_header.chunk_id), sizeof(fmt_header.chunk_id));
            out.write(reinterpret_cast<const char *>(&fmt_header.chunk_size), sizeof(fmt_header.chunk_size));
            out.write(reinterpret_cast<const char *>(&fmt_header.audio_format), sizeof(fmt_header.audio_format));
            out.write(reinterpret_cast<const char *>(&fmt_header.num_channels), sizeof(fmt_header.num_channels));
            out.write(reinterpret_cast<const char *>(&fmt_header.sample_rate), sizeof(fmt_header.sample_rate));
            out.write(reinterpret_cast<const char *>(&fmt_header.byte_rate), sizeof(fmt_header.byte_rate));
            out.write(reinterpret_cast<const char *>(&fmt_header.block_align), sizeof(fmt_header.block_align));
            out.write(reinterpret_cast<const char *>(&fmt_header.bits_per_sample), sizeof(fmt_header.bits_per_sample));

            if (fmt_header.audio_format == audio_format_enum::FLOAT_DATA)
            {
                // FLOAT
                out.write(reinterpret_cast<const char *>(&(*fmt_header.cb_size)), sizeof(*fmt_header.cb_size));
            }

            return out;
        }
    };

    struct fact
    {
        uint32_t chunk_id{};      // "fact" in ASCII (0x74636166 in 'little-endian')
        uint32_t chunk_size{};    // 4
        uint32_t sample_length{}; // num_channels * num_samples (per channel)

        fact() {}
        fact(uint32_t sample_length) : chunk_id{0x74636166}, chunk_size{4}, sample_length{sample_length} {}

        std::string str() const
        {
            std::stringstream out;

            out << "FACT Header:"
                << '\n'
                << "  - ChunkID:     " << utils::uint32_to_string(this->chunk_id) << '\n'
                << "  - ChunkSize:   " << this->chunk_size << '\n'
                << "  - Length:      " << this->sample_length << '\n'
                << std::endl;

            return out.str();
        }

        friend std::istream &operator>>(std::ifstream &in, fact &fact_header)
        {
            in.read(reinterpret_cast<char *>(&fact_header.chunk_id), sizeof(fact_header.chunk_id));
            in.read(reinterpret_cast<char *>(&fact_header.chunk_size), sizeof(fact_header.chunk_size));
            in.read(reinterpret_cast<char *>(&fact_header.sample_length), sizeof(fact_header.sample_length));

            if (fact_header.chunk_id != 0x74636166)
            {
                // Fact header is missing, but data may be available -> continue reading
                fact_header.chunk_id = 0;
                fact_header.chunk_size = 0;
                fact_header.sample_length = 0;

                in.seekg(std::streamoff(-3 * sizeof(uint32_t)), in.cur);
                logger::error("fact header missing");
                // throw std::runtime_error("fact header missing")
            }

            return in;
        }

        friend std::ostream &operator<<(std::ostream &out, const fact &fact_header)
        {
            out.write(reinterpret_cast<const char *>(&fact_header.chunk_id), sizeof(fact_header.chunk_id));
            out.write(reinterpret_cast<const char *>(&fact_header.chunk_size), sizeof(fact_header.chunk_size));
            out.write(reinterpret_cast<const char *>(&fact_header.sample_length), sizeof(fact_header.sample_length));

            return out;
        }
    };

    struct data
    {
        uint32_t chunk_id{};            // "data" in ASCII (0x61746164 in 'little-endian')
        uint32_t chunk_size{};          // num_channels * num_samples_per_channel * byte_per_sample
        std::vector<uint8_t> samples{}; // samples (read as bytes)

        data() {}
        data(const std::vector<std::vector<T>> &data, sample_type_enum sample_type) : chunk_id{0x61746164}
        {
            if (data.size() == 0 || data[0].size() == 0)
            {
                throw std::runtime_error("data array must contain at least one sample");
            }

            switch (sample_type)
            {
            case sample_type_enum::UINT8:
            {
                std::size_t num_channels = data.size();
                chunk_size = data.size() * data[0].size();
                samples.resize(chunk_size);

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    for (std::size_t j = 0; j < data[i].size(); ++j)
                    {
                        samples[j * num_channels + i] = utils::audio::convert<uint8_t, T>(data[i][j]);
                    }
                }
                break;
            }

            case sample_type_enum::SINT16:
            {
                std::size_t num_channels = data.size();
                uint16_t byte_per_sample = 2;
                chunk_size = data.size() * data[0].size() * byte_per_sample;
                samples.resize(chunk_size);

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    for (std::size_t j = 0; j < data[i].size(); ++j)
                    {
                        *((int16_t *)(&samples[j * num_channels * byte_per_sample + i * byte_per_sample])) = utils::audio::convert<int16_t, T>(data[i][j]);
                    }
                }
                break;
            }

            case sample_type_enum::SINT24:
            {
                std::size_t num_channels = data.size();
                uint16_t byte_per_sample = 3;
                chunk_size = data.size() * data[0].size() * byte_per_sample;
                samples.resize(chunk_size);

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    for (std::size_t j = 0; j < data[i].size(); ++j)
                    {
                        int32_t sample = utils::audio::convert<int32_t, T>(data[i][j], byte_per_sample);
                        uint8_t _low = sample & 0x000000FF;
                        uint8_t _mid = (sample & 0x0000FF00) >> 8;
                        uint8_t _high = (sample & 0x00FF0000) >> 16;
                        samples[j * num_channels * byte_per_sample + i * byte_per_sample] = _low;
                        samples[j * num_channels * byte_per_sample + i * byte_per_sample + 1] = _mid;
                        samples[j * num_channels * byte_per_sample + i * byte_per_sample + 2] = _high;
                    }
                }
                break;
            }

            case sample_type_enum::SINT32:
            {
                std::size_t num_channels = data.size();
                uint16_t byte_per_sample = 4;
                chunk_size = data.size() * data[0].size() * byte_per_sample;
                samples.resize(chunk_size);

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    for (std::size_t j = 0; j < data[i].size(); ++j)
                    {
                        *((int32_t *)(&samples[j * num_channels * byte_per_sample + i * byte_per_sample])) = utils::audio::convert<int32_t, T>(data[i][j]);
                    }
                }
                break;
            }

            case sample_type_enum::SINT64:
            {
                std::size_t num_channels = data.size();
                uint16_t byte_per_sample = 8;
                chunk_size = data.size() * data[0].size() * byte_per_sample;
                samples.resize(chunk_size);

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    for (std::size_t j = 0; j < data[i].size(); ++j)
                    {
                        *((int64_t *)(&samples[j * num_channels * byte_per_sample + i * byte_per_sample])) = utils::audio::convert<int64_t, T>(data[i][j]);
                    }
                }
                break;
            }

            case sample_type_enum::FLOAT:
            {
                std::size_t num_channels = data.size();
                uint16_t byte_per_sample = 4;
                chunk_size = data.size() * data[0].size() * byte_per_sample;
                samples.resize(chunk_size);

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    for (std::size_t j = 0; j < data[i].size(); ++j)
                    {
                        *((float *)(&samples[j * num_channels * byte_per_sample + i * byte_per_sample])) = utils::audio::convert<float, T>(data[i][j]);
                    }
                }
                break;
            }

            case sample_type_enum::DOUBLE:
            {
                std::size_t num_channels = data.size();
                uint16_t byte_per_sample = 8;
                chunk_size = data.size() * data[0].size() * byte_per_sample;
                samples.resize(chunk_size);

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    for (std::size_t j = 0; j < data[i].size(); ++j)
                    {
                        *((double *)(&samples[j * num_channels * byte_per_sample + i * byte_per_sample])) = utils::audio::convert<double, T>(data[i][j]);
                    }
                }
                break;
            }

            default:
                break;
            }
        }

        std::string str() const
        {
            std::stringstream out;

            out << "DATA Header:"
                << '\n'
                << "  - ChunkID:     " << utils::uint32_to_string(this->chunk_id) << '\n'
                << "  - ChunkSize:   " << this->chunk_size << '\n'
                << "  - Length:      " << this->samples.size() << '\n'
                << std::endl;

            return out.str();
        }

        friend std::istream &operator>>(std::ifstream &in, data &data_header)
        {
            uint32_t tmp_chunk_id{};
            uint32_t tmp_chunk_size{};
            uint8_t tmp_junk{};
            while (tmp_chunk_id != 0x61746164)
            {
                // Read header
                in.read(reinterpret_cast<char *>(&tmp_chunk_id), sizeof(tmp_chunk_id));
                in.read(reinterpret_cast<char *>(&tmp_chunk_size), sizeof(tmp_chunk_size));

                if (in.eof())
                {
                    // We reached the EOF without finding the data header
                    break;
                }

                // If header != data => skip its content
                if (tmp_chunk_id != 0x61746164)
                {
#ifdef DEBUG
                    {
                        std::stringstream ss;
                        ss << utils::uint32_to_string(tmp_chunk_id) << " " << tmp_chunk_size + 8 << " (discarded)";

                        logger::warning(ss.str());
                    }
#endif
                    for (uint32_t i = 0; i < tmp_chunk_size; ++i)
                    {
                        in.read(reinterpret_cast<char *>(&tmp_junk), sizeof(tmp_junk));
                    }
                }
            }

            if (in.eof())
            {
                throw std::runtime_error("data header missing in WAV file");
            }

            data_header.chunk_id = tmp_chunk_id;
            data_header.chunk_size = tmp_chunk_size;

            data_header.samples.resize(data_header.chunk_size);

            for (uint32_t i = 0; i < data_header.chunk_size; ++i)
            {
                in.read(reinterpret_cast<char *>(&data_header.samples[i]), sizeof(*data_header.samples.begin()));
            }

            return in;
        }

        friend std::ostream &operator<<(std::ostream &out, const data &data_header)
        {
            out.write(reinterpret_cast<const char *>(&data_header.chunk_id), sizeof(data_header.chunk_id));
            out.write(reinterpret_cast<const char *>(&data_header.chunk_size), sizeof(data_header.chunk_size));

            for (auto sample : data_header.samples)
            {
                out.write(reinterpret_cast<const char *>(&sample), sizeof(sample));
            }

            return out;
        }
    };

    sample_type_enum get_sample_type(uint16_t bits_per_sample, bool is_float)
    {
        switch (bits_per_sample)
        {
        case 8:
            return sample_type_enum::UINT8;
        case 16:
            return sample_type_enum::SINT16;
        case 24:
            return sample_type_enum::SINT24;
        case 32:
            return is_float ? sample_type_enum::FLOAT : sample_type_enum::SINT32;
        case 64:
            return is_float ? sample_type_enum::DOUBLE : sample_type_enum::SINT64;
        default:
            throw std::runtime_error("sample format not supported");
        }
    }

    std::string filepath;

    std::vector<std::vector<T>> convert(const std::vector<uint8_t> &byte_array, uint16_t bits_per_sample, sample_type_enum s_type, uint16_t num_channels, [[maybe_unused]] uint32_t sample_rate)
    {
        std::vector<std::vector<T>> a;
        std::size_t byte_per_sample = bits_per_sample / 8;
        std::size_t len = byte_array.size() / (byte_per_sample * num_channels);
        for (int32_t i = 0; i < num_channels; ++i)
        {
            a.emplace_back(len);
        }

        for (std::size_t i = 0; i < len; ++i)
        {
            for (std::size_t j = 0; j < num_channels; ++j)
            {
                std::size_t k = i * num_channels * byte_per_sample + j * byte_per_sample;

                switch (s_type)
                {
                case sample_type_enum::UINT8:
                {
                    uint8_t sample = *((uint8_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, uint8_t>(sample);
                    break;
                }

                case sample_type_enum::SINT16:
                {
                    int16_t sample = *((int16_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, int16_t>(sample);
                    break;
                }

                case sample_type_enum::SINT24:
                {
                    uint8_t _low = *((uint8_t *)(&byte_array[k]));
                    uint8_t _mid = *((uint8_t *)(&byte_array[k + 1]));
                    uint8_t _high = *((uint8_t *)(&byte_array[k + 2]));
                    int32_t sample = (_high & 0x80) ? (0xff << 24) | (_high << 16) | (_mid << 8) | _low : (_high << 16) | (_mid << 8) | _low;

                    a[j][i] = utils::audio::convert<T, int32_t>(sample, 3);
                    break;
                }

                case sample_type_enum::SINT32:
                {
                    int32_t sample = *((int32_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, int32_t>(sample);
                    break;
                }

                case sample_type_enum::SINT64:
                {
                    int64_t sample = *((int64_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, int64_t>(sample);
                    break;
                }

                case sample_type_enum::FLOAT:
                {
                    float sample = *((float *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, float>(sample);
                    break;
                }

                case sample_type_enum::DOUBLE:
                {
                    double sample = *((double *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, double>(sample);
                    break;
                }

                default:
                {
                    throw std::runtime_error("format not supported");
                }
                }
            }
        }

        return a;
    }

    uint16_t get_byte_depth_from_sample_type(sample_type_enum sample_type)
    {
        switch (sample_type)
        {
        case sample_type_enum::UINT8:
            return 1;

        case sample_type_enum::SINT16:
            return 2;

        case sample_type_enum::SINT24:
            return 3;

        case sample_type_enum::SINT32:
        case sample_type_enum::FLOAT:
            return 4;

        case sample_type_enum::SINT64:
        case sample_type_enum::DOUBLE:
            return 8;

        default:
            return 1;
        }
    }

public:
    std::vector<std::vector<T>> data_samples; // A matrix n_channels * samples_per_channel in the desired type
    uint32_t sample_rate;
    sample_type_enum sample_type;

    wav_file(std::string filepath) : filepath{filepath}, data_samples{}, sample_rate{44100}, sample_type{sample_type_enum::SINT24}
    {
    }

    void read_file()
    {
        std::ifstream file(filepath, std::ios::binary);

        if (!file)
            throw std::runtime_error(filepath + " does not exist");

        wav_file::riff riff_header{};
        wav_file::fmt fmt_header{};
        std::optional<wav_file::fact> fact_header{};
        wav_file::data data_header{};

        file >> riff_header;
        file >> fmt_header;

        sample_rate = fmt_header.sample_rate;

        if (fmt_header.audio_format == audio_format_enum::FLOAT_DATA)
        {
            wav_file::fact _fact_header;
            file >> _fact_header;

            fact_header = _fact_header;
        }

        file >> data_header;

#ifdef DEBUG
        {
            std::stringstream ss;
            ss << riff_header.str();
            ss << fmt_header.str();
            if (fact_header.has_value())
            {
                ss << (*fact_header).str();
            }
            ss << data_header.str();

            logger::info(ss.str());
        }
#endif

        sample_type = get_sample_type(fmt_header.bits_per_sample, fmt_header.audio_format == audio_format_enum::FLOAT_DATA);

        // Convert bytearray to vector of the desired type
        data_samples = convert(data_header.samples, fmt_header.bits_per_sample, sample_type, fmt_header.num_channels, fmt_header.sample_rate);
    }

    void write_file(const std::vector<std::vector<T>> &data, uint32_t sample_rate, sample_type_enum sample_type = sample_type_enum::SINT24)
    {
        std::ofstream file(filepath, std::ios::binary);

        if (!file)
            throw std::runtime_error(filepath + " was not created due to some issues");

        wav_file::data data_header{data, sample_type};
        std::optional<wav_file::fact> fact_header{};
        if (sample_type == sample_type_enum::FLOAT || sample_type == sample_type_enum::DOUBLE)
        {
            fact_header = {static_cast<uint32_t>(data[0].size() * data.size())};
        }

        wav_file::fmt fmt_header{
            sample_type == sample_type_enum::FLOAT || sample_type == sample_type_enum::DOUBLE ? audio_format_enum::FLOAT_DATA : audio_format_enum::PCM_DATA,
            static_cast<uint16_t>(data.size()),
            sample_rate,
            static_cast<uint16_t>(get_byte_depth_from_sample_type(sample_type) * 8),
            sample_type == sample_type_enum::FLOAT || sample_type == sample_type_enum::DOUBLE ? std::optional<uint16_t>{0} : std::optional<uint16_t>{}};

        wav_file::riff riff_header{
            data_header.chunk_size + 8u +
            (fact_header
                 ? (*fact_header).chunk_size + 8u
                 : 0u) +
            fmt_header.chunk_size + 8u +
            4u};

#ifdef DEBUG
        std::cout
            << data_header.str();
        if (fact_header)
        {
            std::cout << (*fact_header).str();
        }
        std::cout
            << fmt_header.str();

        std::cout << riff_header.str();
#endif

        // Write file
        file << riff_header << fmt_header;
        if (fact_header.has_value())
        {
            file << *fact_header;
        }
        file << data_header;
    }
};

#endif