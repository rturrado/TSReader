#include "Exception.h"
#include "PacketBuffer.h"

#include <algorithm>
#include <bitset>
#include <vector>

namespace TS
{
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
