#ifndef __TS_READER_H__
#define __TS_READER_H__

#include "Stats.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <string>

namespace TS
{
    class FileReader
    {
    public:
        explicit FileReader(const std::filesystem::path& path, bool collect_stats) noexcept
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
        std::unique_ptr<Stats> _stats{};
    };
}

#endif
