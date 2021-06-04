#include "PSI_Tables.h"

namespace TS
{
    /* static */
    PSI_Tables& PSI_Tables::get_instance()
    {
        static PSI_Tables instance;
        return instance;
    }

    void PSI_Tables::set_PAT_program_number(PID p, program_number n)
    {
        PAT_map[p] = n;
    }

    void PSI_Tables::set_PMT_stream_type(PID p, stream_type st)
    {
        PMT_map[p] = st;
    }

    bool PSI_Tables::is_PMT_PID(PID p) const
    {
        return PAT_map.contains(p);
    }
    
    bool PSI_Tables::is_PES_PID(PID p) const
    {
        return PMT_map.contains(p);
    }

    stream_type PSI_Tables::get_stream_type(PID p) const
    {
        return PMT_map.at(p);
    }
}
