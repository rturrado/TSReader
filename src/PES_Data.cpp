#include "PES_Data.h"

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

    TPES_data PES_Data::get_PES_data(PID p) const
    {
        return PES_map.at(p);
    }

    void PES_Data::set_PES_data(PID p, TPES_data& data)
    {
        PES_map[p] = data;
    }
}
