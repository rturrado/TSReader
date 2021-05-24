#ifndef __TABLES_H__
#define __TABLES_H__

#include "Packet.h"

#include <map>

using PID = uint16_t;
using program_number = uint16_t;
using stream_type = uint8_t;

using TPAT_map = std::map<PID, program_number>;
using TPMT_map = std::map<PID, stream_type>;

TPAT_map PAT_map{};
TPMT_map PMT_map{};

#endif
