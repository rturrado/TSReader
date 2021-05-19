#include "Packet.h"

#include <iostream>

namespace TS
{
    /* friend */
    std::ostream& operator<<(std::ostream& os, const Header& header)
    {
        os << "{"
            << "TEI: " << header.transport_error_indicator << ", "
            << "PUI: " << header.payload_unit_start_indicator << ", "
            << "TP: " << header.transport_priority << ", "
            << "PID: 0x" << std::hex << header.PID << ", "
            << "TSC: 0x" << std::hex << static_cast<uint16_t>(header.transport_scrambling_control) << ", "
            << "AFC: 0x" << std::hex << static_cast<uint16_t>(header.adaptation_field_control) << ", "
            << "CC: " << std::dec << static_cast<uint16_t>(header.continuity_counter)
            << "}" << "\n";
        return os;
    }
}
