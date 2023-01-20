#ifndef __WAV_HPP__
#define __WAV_HPP__

// Reference: http://soundfile.sapp.org/doc/WaveFormat/
//            https://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

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

template <typename T, std::enable_if_t<true == std::is_floating_point_v<T> && !std::is_same<T, bool>::value, bool> = true>
class wav_file
{
private:
    struct riff
    {
        uint32_t chunk_id{};   // "RIFF" in ASCI (0x46464952 in 'little-endian')
        uint32_t chunk_size{}; // Size in bytes (does not include chunk_id and chunk_size in the count) 4 + (8 + subchunk1_size) + (8 + subchunk2_size)
        uint32_t format{};     // "WAVE" in ASCI (0x45564157 in 'little-ednian')

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
            out << riff_header.chunk_id
                << riff_header.chunk_size
                << riff_header.format;

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

            if (fmt_header.audio_format == 1)
            {
                // PCM
                return in;
            }
            else if (fmt_header.audio_format == 3)
            {
                // FLOAT
                in.read(reinterpret_cast<char *>(&(*fmt_header.cb_size)), sizeof(*fmt_header.cb_size));
                return in;
            }

            throw std::runtime_error("Format " + std::to_string(fmt_header.audio_format) + " not supported");
        }

        friend std::ostream &operator<<(std::ostream &out, const fmt &fmt_header)
        {
            if (fmt_header.audio_format != 1 && fmt_header.audio_format != 3)
            {
                throw std::runtime_error("Format " + std::to_string(fmt_header.audio_format) + " not supported");
            }

            out << fmt_header.chunk_id
                << fmt_header.chunk_size
                << fmt_header.audio_format
                << fmt_header.num_channels
                << fmt_header.sample_rate
                << fmt_header.byte_rate
                << fmt_header.block_align
                << fmt_header.bits_per_sample;

            if (fmt_header.audio_format == 3)
            {
                out << *fmt_header.cb_size;
            }

            return out;
        }
    };

    struct fact
    {
        uint32_t chunk_id{};      // "fact" in ASCII (0x74636166 in 'little-endian')
        uint32_t chunk_size{};    // 4
        uint32_t sample_length{}; // num_channels * num_samples (per channel)

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
                throw std::runtime_error("fact header missing");
            }

            return in;
        }

        friend std::ostream &operator<<(std::ostream &out, const fact &fact_header)
        {
            out << fact_header.chunk_id
                << fact_header.chunk_size
                << fact_header.sample_length;

            return out;
        }
    };

    struct data
    {
        uint32_t chunk_id{};            // "data" in ASCII (0x61746164 in 'little-endian')
        uint32_t chunk_size{};          // num_channels * num_samples_per_channel * byte_per_sample
        std::vector<uint8_t> samples{}; // samples (read as bytes)

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
            out << data_header.chunk_id << data_header.chunk_size;

            for(auto sample : data_header.samples)
            {
                out << sample;
            }

            return out;
        }
    };

    enum sample_type
    {
        UINT8,
        SINT16,
        SINT24,
        SINT32,
        SINT64,
        FLOAT,
        DOUBLE
    };

    sample_type get_sample_type(uint16_t bits_per_sample, bool is_float)
    {
        switch (bits_per_sample)
        {
        case 8:
            return sample_type::UINT8;
        case 16:
            return sample_type::SINT16;
        case 24:
            return sample_type::SINT24;
        case 32:
            return is_float ? sample_type::FLOAT : sample_type::SINT32;
        case 64:
            return is_float ? sample_type::DOUBLE : sample_type::SINT64;
        default:
            throw std::runtime_error("sample format not supported");
        }
    }

    std::string filepath;
    std::vector<std::vector<T>> data_samples; // A matrix n_channels * samples_per_channel in the desired type

    std::vector<std::vector<T>> convert(const std::vector<uint8_t> &byte_array, uint16_t bits_per_sample, sample_type s_type, uint16_t num_channels, [[maybe_unused]] uint32_t sample_rate)
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
                case sample_type::UINT8:
                {
                    uint8_t sample = *((uint8_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, uint8_t>(sample);
                    break;
                }

                case sample_type::SINT16:
                {
                    int16_t sample = *((int16_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, int16_t>(sample);
                    break;
                }

                case sample_type::SINT24:
                {
                    uint8_t _low = *((uint8_t *)(&byte_array[k]));
                    uint8_t _mid = *((uint8_t *)(&byte_array[k + 1]));
                    uint8_t _high = *((uint8_t *)(&byte_array[k + 2]));
                    int32_t sample = (_high & 0x80) ? (0xff << 24) | (_high << 16) | (_mid << 8) | _low : (_high << 16) | (_mid << 8) | _low;

                    a[j][i] = utils::audio::convert<T, int32_t>(sample, 3);
                    break;
                }

                case sample_type::SINT32:
                {
                    int32_t sample = *((int32_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, int32_t>(sample);
                    break;
                }

                case sample_type::SINT64:
                {
                    int64_t sample = *((int64_t *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, int64_t>(sample);
                    break;
                }

                case sample_type::FLOAT:
                {
                    float sample = *((float *)(&byte_array[k]));

                    a[j][i] = utils::audio::convert<T, float>(sample);
                    break;
                }

                case sample_type::DOUBLE:
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

public:
    wav_file(std::string filepath) : filepath{filepath} {}

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

        if (fmt_header.audio_format == 3)
        {
            file >> *fact_header;
        }

        file >> data_header;

        std::cout << riff_header.str();
        std::cout << fmt_header.str();
        if (fact_header)
        {
            std::cout << (*fact_header).str();
        }
        std::cout << data_header.str();

        // Convert bytearray to vector of the desired type
        convert(data_header.samples, fmt_header.bits_per_sample, get_sample_type(fmt_header.bits_per_sample, fmt_header.audio_format == 3), fmt_header.num_channels, fmt_header.sample_rate);
    }

    void write_file()
    {
    }
};

#endif