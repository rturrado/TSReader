#ifndef __TS_READER_H__
#define __TS_READER_H__

#include "Stats.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace TS
{
    class FileReader
    {
    public:
        explicit FileReader(const std::filesystem::path& path, std::vector<uint8_t> stream_type_list, bool collect_stats) noexcept
            : _file_path{ path }
        {
            if (collect_stats)
            {
                _stats = std::make_unique<Stats>();
            }
        }
        void start();
    private:
        std::filesystem::path _file_path{};
        std::vector<uint8_t> _stream_type_list{};
        std::unique_ptr<Stats> _stats{};
    };
}

#endif
