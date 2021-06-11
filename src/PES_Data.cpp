#include "PES_Data.hpp"

namespace TS
{
    /* static */
    PES_Data& PES_Data::get_instance()
    {
        static PES_Data instance;
        return instance;
    }

    bool PES_Data::has_PES_data(PID p) const
    {
        return PES_map.contains(p);
    }

    const byte_buffer_view PES_Data::get_PES_data(PID p) const
    {
        return PES_map.at(p);
    }

    void PES_Data::set_PES_data(PID p, const byte_buffer_view& data)
    {
        PES_map[p] = data;
    }
}
