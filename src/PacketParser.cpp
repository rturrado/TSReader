#include "Exception.h"
#include "Packet.h"
#include "PacketParser.h"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <iostream>

namespace TS
{
    /* static */
    std::multiset<uint16_t> PacketParser::pids{};

    void PacketParser::parse_header(PacketBuffer& buffer)
    {
        bool big_endian{ true };
        auto header = buffer.read(header_size, big_endian);

        auto tmp{ header };
        tmp &= sync_byte_mask;
        tmp >>= sync_byte_mask.find_first();
        _packet.header.sync_byte = static_cast<uint8_t>(tmp.to_ulong());

        if (_packet.header.sync_byte != sync_byte_valid_value)
        {
            throw InvalidSyncByte(_packet.header.sync_byte);
        }

        tmp = header;
        tmp &= transport_error_indicator_mask;
        tmp >>= transport_error_indicator_mask.find_first();
        _packet.header.transport_error_indicator = tmp.any();

        tmp = header;
        tmp &= payload_unit_start_indicator_mask;
        tmp >>= payload_unit_start_indicator_mask.find_first();
        _packet.header.payload_unit_start_indicator = tmp.any();

        tmp = header;
        tmp &= transport_priority_mask;
        tmp >>= transport_priority_mask.find_first();
        _packet.header.transport_priority = tmp.any();

        tmp = header;
        tmp &= PID_mask;
        tmp >>= PID_mask.find_first();
        _packet.header.PID = static_cast<uint16_t>(tmp.to_ulong());

        tmp = header;
        tmp &= transport_scrambling_control_mask;
        tmp >>= transport_scrambling_control_mask.find_first();
        _packet.header.transport_scrambling_control = static_cast<uint8_t>(tmp.to_ulong());

        tmp = header;
        tmp &= adaptation_field_control_mask;
        tmp >>= adaptation_field_control_mask.find_first();
        _packet.header.adaptation_field_control = static_cast<uint8_t>(tmp.to_ulong());

        tmp = header;
        tmp &= continuity_counter_mask;
        tmp >>= continuity_counter_mask.find_first();
        _packet.header.continuity_counter = static_cast<uint8_t>(tmp.to_ulong());
    }

    void PacketParser::parse_adaptation_field(PacketBuffer& buffer)
    {
    }

    void PacketParser::parse_payload_data(PacketBuffer& buffer)
    {
    }

    void PacketParser::parse(PacketBuffer& buffer)
    {
        parse_header(buffer);

        pids.insert(_packet.header.PID);

        if (_packet.header.adaptation_field_present())
        {
            parse_adaptation_field(buffer);
        }
        if (_packet.header.payload_data_present())
        {
            parse_payload_data(buffer);
        }
    }
}
