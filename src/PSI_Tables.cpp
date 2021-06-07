#include "Exception.hpp"
#include "PSI_Tables.hpp"

#include <algorithm>

namespace TS
{
    bool PSI_Tables::PSI_Table::needs_update(uint8_t version)
    {
        if (not _initialized)
        {
            _initialized = true;
            _version = version;

            return true;
        }

        if (version > _version)
        {
            _version = version;

            return true;
        }

        return false;
    }

    /* static */
    PSI_Tables& PSI_Tables::get_instance()
    {
        static PSI_Tables instance;
        return instance;
    }

    program_number PSI_Tables::get_PAT_program_number(PID p) const
    {
        return PAT_table.at(p);
    }

    stream_type PSI_Tables::get_PES_stream_type(PID p) const
    {
        return PES_stream_type_cache_table.at(p);
    }

    void PSI_Tables::set_PAT_program_number(PID p, program_number n)
    {
        if (PAT_table.contains(p))
        {
            throw Duplicated_PMT_PID{};
        }
        PAT_table[p] = n;
    }

    void PSI_Tables::set_PMT_stream_type(program_number n, PID p, stream_type st)
    {
        if (PES_stream_type_cache_table.contains(p))
        {
            throw Duplicated_PES_PID{};
        }

        // Update PMT table
        if (not PMT_tables.contains(n))
        {
            throw Unknown_PMT_Program_Number{};
        }
        PMT_Table& pmtt = PMT_tables[n];
        pmtt[p] = st;

        // Update PES stream type cache table
        PES_stream_type_cache_table[p] = st;
    }

    bool PSI_Tables::is_PMT_PID(PID p) const
    {
        return PAT_table.is_PMT_PID(p);
    }

    bool PSI_Tables::is_PES_PID(PID p) const
    {
        return PES_stream_type_cache_table.is_PES_PID(p);
    }
}
