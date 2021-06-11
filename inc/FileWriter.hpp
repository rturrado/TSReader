#ifndef __TS_FILE_WRITER_HPP__
#define __TS_FILE_WRITER_HPP__

#include "ByteBufferView.hpp"
#include "StreamType.hpp"
#include "PES_Data.hpp"

#include <fstream>

namespace TS
{
    class FileWriter
    {
    public:
        explicit FileWriter(stream_type st) noexcept;
        ~FileWriter();

        stream_type get_stream_type() const;
        void write(const byte_buffer_view& data);
    private:
        stream_type _stream_type{};
        std::ofstream _ofs{};
    };
}

#endif
