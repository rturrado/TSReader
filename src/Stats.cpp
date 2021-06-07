#include "Packet.hpp"
#include "Stats.hpp"
#include "StreamType.hpp"
#include "PSI_Tables.hpp"

#include <iostream>
#include <sstream>

namespace TS
{
    /* static */
    Stats& Stats::get_instance()
    {
        static Stats instance;
        return instance;
    }

    void Stats::collect(const Packet& packet)
    {
        _pids.insert(packet.header.PID);
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const Stats& stats)
    {
        for (auto it = cbegin(stats._pids); it != cend(stats._pids); it = stats._pids.upper_bound(*it))
        {
            std::string stream_info{ "unknown" };

            const PSI_Tables& tables{ PSI_Tables::get_instance() };
            PID pid{ *it };
            if (tables.is_PES_PID(pid))
            {
                uint8_t stream_type{ tables.get_stream_type(pid) };

                std::ostringstream oss{};
                oss << "{" << "0x" << std::hex << static_cast<int16_t>(stream_type);
                oss << ", " << StreamTypeMap::get_instance().get_stream_description(stream_type);
                oss << "}";
                stream_info = oss.str();
            }

            os << "PID: 0x" << std::hex << pid << std::dec
                << "\tcount = " << stats._pids.count(*it)
                << "\tstream info = " << stream_info
                << "\n";
        }
        return os;
    }
}
