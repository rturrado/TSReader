#ifndef __FILE_WRITER_H__
#define __FILE_WRITER_H__

#include "StreamType.h"
#include "PES_Data.h"

#include <fstream>

namespace TS
{
    class FileWriter
    {
    public:
        explicit FileWriter(stream_type st) noexcept;
        ~FileWriter();

        stream_type get_stream_type() const;
        void write(const TPES_data& data);
    private:
        stream_type _stream_type{};
        std::ofstream _ofs{};
    };
}

#endif
