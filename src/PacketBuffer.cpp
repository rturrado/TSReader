#include "Exception.hpp"
#include "PacketBuffer.hpp"

#include <algorithm>
#include <span>

namespace TS
{
    const byte_buffer_view PacketBuffer::read(uint8_t n)
    {
        if (_pos + n > size())
        {
            throw PacketBufferOverrun(n, size() - _pos);
        }

        const byte_buffer_view ret{ begin(_buffer) + _pos, n };

        _pos += n;

        return ret;
    }

    /* friend */
    std::ifstream& operator>>(std::ifstream& ifs, PacketBuffer& pb)
    {
        pb.reset_read_position();
        ifs.read(pb.data_as_char_pointer(), pb.size());
        std::streamsize read_data_size = ifs.gcount();
        if (read_data_size != 0 && read_data_size < pb.size())
        {
            std::cout << "Error: read packet of size: " << read_data_size << "\n";
        }
        return ifs;
    }
}
