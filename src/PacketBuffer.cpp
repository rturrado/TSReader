#include "Exception.h"
#include "PacketBuffer.h"

#include <algorithm>

namespace TS
{
    boost::dynamic_bitset<uint8_t> PacketBuffer::read(uint8_t n, bool big_endian)
    {
        if (_pos + n > size())
        {
            throw PacketBufferOverrun(n, size() - _pos);
        }

        boost::dynamic_bitset<uint8_t> ret{};
        if (!big_endian)
        {
            ret.append(begin(_buffer) + _pos, begin(_buffer) + _pos + n);
        }
        else
        {
            for (auto i = n - 1; i >= 0; --i)
            {
                ret.append(_buffer[_pos + i]);
            }
        }

        _pos += n;

        return ret;
    }



    void PacketBuffer::reset_read_position()
    {
        _pos = 0;
    }



    /* friend */
    std::ifstream& operator>>(std::ifstream& ifs, PacketBuffer& pb)
    {
        pb.reset_read_position();
        ifs.read(pb.data(), pb.size());
        std::streamsize read_data_size = ifs.gcount();
        if (read_data_size != 0 && read_data_size < pb.size())
        {
            std::cout << "Error: read packet of size: " << read_data_size << "\n";
        }
        return ifs;
    }
}
