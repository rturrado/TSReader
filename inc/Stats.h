#ifndef __STATS_H__
#define __STATS_H__

#include "Packet.h"

#include <set>

namespace TS
{
    class Stats
    {
    public:
        void collect(const Packet& packet);
        friend std::ostream& operator<<(std::ostream& os, const Stats& stats);
    private:
        std::multiset<uint16_t> _pids;
    };
}

#endif
