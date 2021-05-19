#include "Exception.h"
#include "Packet.h"
#include "PacketParser.h"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <iostream>

namespace TS
{
    template <typename ReturnType>
    ReturnType read_field(const boost::dynamic_bitset<uint8_t>& bs, const boost::dynamic_bitset<uint8_t>& mask_bs)
    {
        auto tmp{ bs & mask_bs };
        tmp >>= mask_bs.find_first();
        return static_cast<ReturnType>(tmp.to_ulong());
    }



    void PacketParser::parse_header(PacketBuffer& buffer)
    {
        Header& hdr = _packet.header;

        // Read from buffer into bitset
        auto header_buffer = buffer.read<true>(header_size);
        boost::dynamic_bitset<uint8_t> header_bs{ header_size * 8 };
        from_block_range(cbegin(header_buffer), cbegin(header_buffer) + header_size, header_bs);

        // Read header fields
        hdr.sync_byte = read_field<uint8_t>(header_bs, sync_byte_mask_bs);
        if (hdr.sync_byte != sync_byte_valid_value)
        {
            throw InvalidSyncByte(hdr.sync_byte);
        }
        hdr.transport_error_indicator = header_bs.test(transport_error_indicator_mask_bs.find_first());
        hdr.payload_unit_start_indicator = header_bs.test(payload_unit_start_indicator_mask_bs.find_first());
        hdr.transport_priority = header_bs.test(transport_priority_mask_bs.find_first());
        hdr.PID = read_field<uint16_t>(header_bs, PID_mask_bs);
        hdr.transport_scrambling_control = read_field<uint8_t>(header_bs, transport_scrambling_control_mask_bs);
        hdr.adaptation_field_control = read_field<uint8_t>(header_bs, adaptation_field_control_mask_bs);
        hdr.continuity_counter = read_field<uint8_t>(header_bs, continuity_counter_mask_bs);
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
