#ifndef __TS_READER_H__
#define __TS_READER_H__

#include <exception>
#include <filesystem>
#include <string>

namespace TS
{
    class FileReader
    {
    public:
        explicit FileReader(const std::filesystem::path path) noexcept : _file_path{ path } {}
        void start();
    private:
        std::filesystem::path _file_path{};
    };
}

#endif
