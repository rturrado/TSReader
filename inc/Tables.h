#ifndef __TABLES_H__
#define __TABLES_H__

#include "Packet.h"

#include <array>
#include <map>
#include <vector>

namespace TS
{
    // Data structures to hold processed table information

    using PID = uint16_t;
    using program_number = uint16_t;
    using stream_type = uint8_t;
    using TPES_data = std::vector<uint8_t>;

    using TPAT_map = std::map<PID, program_number>;  // PMT PID -> program number
    using TPMT_map = std::map<PID, stream_type>;  // PES PID -> stream type
    using TPES_map = std::map<PID, TPES_data>;  // PES PID -> PES data

    TPAT_map& get_PAT_map();
    TPMT_map& get_PMT_map();
    TPES_map& get_PES_map();
}

#endif
