#include "Exception.h"
#include "Packet.h"
#include "PacketParser.h"

#include <algorithm>
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

        // Read from buffer
        auto header_buffer = buffer.read<true>(header_size);
        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> header_bs{ header_size * 8 };
        from_block_range(cbegin(header_buffer), cend(header_buffer), header_bs);

        // Set fields
        hdr.sync_byte = read_field<uint8_t>(header_bs, sync_byte_mask_bs);
        if (hdr.sync_byte != sync_byte_valid_value)
        {
            throw InvalidSyncByte(_packet, _index);
        }
        hdr.transport_error_indicator = header_bs.test(transport_error_indicator_mask_bs.find_first());
        hdr.payload_unit_start_indicator = header_bs.test(payload_unit_start_indicator_mask_bs.find_first());
        hdr.transport_priority = header_bs.test(transport_priority_mask_bs.find_first());
        hdr.PID = read_field<uint16_t>(header_bs, PID_mask_bs);
        hdr.transport_scrambling_control = read_field<uint8_t>(header_bs, transport_scrambling_control_mask_bs);
        hdr.adaptation_field_control = read_field<uint8_t>(header_bs, adaptation_field_control_mask_bs);
        hdr.continuity_counter = read_field<uint8_t>(header_bs, continuity_counter_mask_bs);
    }



    void PacketParser::parse_adaptation_field_flags(PacketBuffer& buffer)
    {
        _packet.adaptation_field->flags = AdaptationFieldFlags{};

        AdaptationField& af = *_packet.adaptation_field;
        AdaptationFieldFlags& aff = *af.flags;

        // Read from buffer
        auto af_buffer = buffer.read(adaptation_field_flags_size);
        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> af_bs{ 8, af_buffer[0] };

        // Set fields
        aff.discontinuity_indicator = af_bs.test(discontinuity_indicator_mask.find_first());
        aff.random_access_indicator = af_bs.test(random_access_indicator_mask.find_first());
        aff.elementary_stream_priority_indicator = af_bs.test(elementary_stream_priority_indicator_mask.find_first());
        aff.PCR_flag = af_bs.test(PCR_flag_mask.find_first());
        aff.OPCR_flag = af_bs.test(OPCR_flag_mask.find_first());
        aff.splicing_point_flag = af_bs.test(splicing_point_flag_mask.find_first());
        aff.transport_private_data_flag = af_bs.test(transport_private_data_flag_mask.find_first());
        aff.extension_flag = af_bs.test(extension_flag_mask.find_first());
    }



    void PacketParser::parse_adaptation_field_optional(PacketBuffer& buffer)
    {
        _packet.adaptation_field->optional = AdaptationFieldOptional{};

        AdaptationField& af = *_packet.adaptation_field;
        AdaptationFieldFlags& aff = *af.flags;
        AdaptationFieldOptional& afo = *af.optional;

        // Read from buffer
        auto af_buffer = buffer.read(af.length - adaptation_field_flags_size);

        // Keep track of stuffing bytes length
        uint8_t stuffing_bytes_length{ static_cast<uint8_t>(af_buffer.size()) };

        // Set fields
        auto cbegin_it{ cbegin(af_buffer) };
        auto cend_it{ cbegin(af_buffer) };
        if (aff.PCR_flag)
        {
            cend_it += PCR_size;

            afo.PCR = boost::dynamic_bitset<uint8_t>{ PCR_size * 8, 0x0 };
            from_block_range(cbegin_it, cend_it, *afo.PCR);
            
            stuffing_bytes_length -= PCR_size;
        }

        if (aff.OPCR_flag)
        {
            cbegin_it = cend_it;
            cend_it += OPCR_size;

            afo.OPCR = boost::dynamic_bitset<uint8_t>{ OPCR_size * 8, 0x0 };
            from_block_range(cbegin_it, cend_it, *afo.OPCR);
            
            stuffing_bytes_length -= OPCR_size;
        }

        if (aff.splicing_point_flag)
        {
            cbegin_it = cend_it;
            cend_it += splicing_countdown_size;

            afo.splice_countdown = static_cast<int8_t>(*cbegin_it);
            
            stuffing_bytes_length--;
        }

        if (aff.transport_private_data_flag)
        {
            // Length
            cbegin_it = cend_it;
            cend_it += transport_private_data_length_size;

            afo.transport_private_data_length = *cbegin_it;
            
            stuffing_bytes_length--;

            // Data
            cbegin_it = cend_it;
            cend_it += *afo.transport_private_data_length;

            afo.transport_private_data = std::vector<uint8_t>(cbegin_it, cend_it);
            
            stuffing_bytes_length -= *afo.transport_private_data_length;
        }

        if (aff.extension_flag)
        {
            cbegin_it = cend_it;
            cend_it = cbegin_it + afo.extension->length;

            parse_adaptation_extension(af_buffer, cbegin_it, cend_it);

            stuffing_bytes_length -= afo.extension->length;
        }

        if (stuffing_bytes_length > 0)
        {
            cbegin_it = cend_it;
            cend_it = cbegin_it + stuffing_bytes_length;

            afo.stuffing_bytes = std::vector<uint8_t>(cbegin_it, cend_it);

            if (std::any_of(cbegin(*afo.stuffing_bytes), cend(*afo.stuffing_bytes),
                [](uint8_t b) { return b != stuffing_byte; }))
            {
                throw InvalidStuffingBytes(_packet, _index);
            }
        }
    }



    void PacketParser::parse_adaptation_extension(std::vector<uint8_t> af_buffer, auto cbegin_it, auto cend_it)
    {
        _packet.adaptation_field->optional->extension = AdaptationExtension{};

        AdaptationFieldOptional& afo = *_packet.adaptation_field->optional;
        AdaptationExtension& ae = *afo.extension;
    }



    void PacketParser::parse_adaptation_field(PacketBuffer& buffer)
    {
        _packet.adaptation_field = AdaptationField{};

        // Read from buffer
        auto af_buffer = buffer.read(adaptation_field_length_size);

        // Set fields
        _packet.adaptation_field->length = af_buffer[0];
        if (_packet.adaptation_field->length > 0)
        {
            parse_adaptation_field_flags(buffer);
        }
        if (_packet.adaptation_field->length > adaptation_field_flags_size)
        {
            parse_adaptation_field_optional(buffer);
        }
    }



    void PacketParser::parse_payload_data(PacketBuffer& buffer)
    {
    }



    void PacketParser::parse(PacketBuffer& buffer)
    {
        _index = 0;

        parse_header(buffer);

        if (_packet.header.adaptation_field_present())
        {
            parse_adaptation_field(buffer);
        }
        if (_packet.header.payload_data_present())
        {
            parse_payload_data(buffer);
        }

        _index++;
    }
}
