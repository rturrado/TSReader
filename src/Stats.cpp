#include "Packet.h"
#include "Stats.h"

#include <iostream>

namespace TS
{
    void Stats::collect(const Packet& packet)
    {
        _pids.insert(packet.header.PID);
    }



    /* friend */
    std::ostream& operator<<(std::ostream& os, const Stats& stats)
    {
        for (auto it = cbegin(stats._pids); it != cend(stats._pids); it = stats._pids.upper_bound(*it))
        {
            os << "PID: 0x" << std::hex << *it << std::dec << "\tcount = " << stats._pids.count(*it) << "\n";
        }
        return os;
    }
}
