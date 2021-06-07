#include "FileWriter.hpp"
#include "StreamType.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace TS
{
    std::filesystem::path get_new_output_stream_fp(stream_type t)
    {
        const char* file_stem{ "ts_stream_" };

        std::ostringstream oss{ file_stem };
        oss << file_stem << "0x" << std::hex << static_cast<uint16_t>(t) << '.' << StreamTypeMap::get_instance().get_file_extension(t);

        return oss.str();
    }

    FileWriter::FileWriter(stream_type st) noexcept
        : _stream_type{ st }
    {
        _ofs.open(get_new_output_stream_fp(_stream_type), std::ios_base::binary);
    }

    FileWriter::~FileWriter()
    {
        _ofs.close();
    }

    stream_type FileWriter::get_stream_type() const
    {
        return _stream_type;
    }

    void FileWriter::write(const TPES_data& data)
    {
        if (_ofs)
        {
            _ofs.write(reinterpret_cast<const char*>(&data[0]), data.size());
        }
    }
}
