#ifndef __TS_STATS_HPP__
#define __TS_STATS_HPP__

#include "Packet.hpp"

#include <set>

namespace TS
{
    class Stats
    {
    public:
        Stats(const Stats&) = delete;
        Stats(Stats&) = delete;
        Stats& operator=(const Stats&) = delete;
        Stats& operator=(Stats&&) = delete;

        static Stats& get_instance();
        void collect(const Packet& packet);
        friend std::ostream& operator<<(std::ostream& os, const Stats& stats);
    private:
        Stats() {}

        std::multiset<uint16_t> _pids;
    };
}

#endif
