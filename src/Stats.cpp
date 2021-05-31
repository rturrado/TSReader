#include "Packet.h"
#include "Stats.h"
#include "StreamType.h"
#include "Tables.h"

#include <iostream>
#include <sstream>

namespace TS
{
    void Stats::collect(const Packet& packet)
    {
        _pids.insert(packet.header.PID);
    }



    /* friend */
    std::ostream& operator<<(std::ostream& os, const Stats& stats)
    {
        TPMT_map& PMT_map = get_PMT_map();
        for (auto it = cbegin(stats._pids); it != cend(stats._pids); it = stats._pids.upper_bound(*it))
        {
            PID pid{ *it };
            std::string stream_type_str{ "unknown" };
            if (PMT_map.contains(pid))
            {
                std::ostringstream oss{};
                uint8_t stream_type{ PMT_map[pid] };
                oss << "{" << "0x" << std::hex << static_cast<int16_t>(stream_type);
                oss << ", " << get_stream_name(stream_type);
                oss << "}";
                stream_type_str = oss.str();
            }
            os << "PID: 0x" << std::hex << pid << std::dec
                << "\tcount = " << stats._pids.count(*it)
                << "\tstream type = " << stream_type_str
                << "\n";
        }
        return os;
    }
}
