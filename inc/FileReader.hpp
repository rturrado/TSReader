#ifndef __TS_FILE_READER_HPP__
#define __TS_FILE_READER_HPP__

#include "Stats.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace TS
{
    class FileReader
    {
    public:
        explicit FileReader(
            const std::filesystem::path& path,
            std::vector<uint8_t>&& stream_type_list,
            bool collect_stats);
        ~FileReader();
        void start();
    private:
        std::ifstream _ifs{};
        std::vector<uint8_t> _stream_type_list{};
        bool _collect_stats{false};
    };
}

#endif
