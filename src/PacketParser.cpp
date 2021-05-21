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



    void PacketParser::parse(PacketBuffer& p_buffer)
    {
        _index = 0;

        parse_header(p_buffer);

        if (_packet.header.adaptation_field_present())
        {
            parse_adaptation_field(p_buffer);
        }
        if (_packet.header.payload_data_present())
        {
            parse_payload_data(p_buffer);
        }

        _index++;
    }



    void PacketParser::parse_header(PacketBuffer& p_buffer)
    {
        Header& hdr = _packet.header;

        // Read from buffer
        auto header_buffer = p_buffer.read<true>(header_size);
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



    void PacketParser::parse_adaptation_field(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field = AdaptationField{};

        // Read from buffer
        auto af_buffer = p_buffer.read(adaptation_field_length_size);

        // Set fields
        _packet.adaptation_field->length = af_buffer[0];

        if (_packet.adaptation_field->length > 0)
        {
            parse_adaptation_field_flags(p_buffer);
        }
        if (_packet.adaptation_field->length > adaptation_field_flags_size)
        {
            parse_adaptation_field_optional(p_buffer);
        }
    }



    void PacketParser::parse_adaptation_field_flags(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field->flags = AdaptationFieldFlags{};

        AdaptationField& af = *_packet.adaptation_field;
        AdaptationFieldFlags& aff = *af.flags;

        // Read from buffer
        auto af_buffer = p_buffer.read(adaptation_field_flags_size);
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



    void PacketParser::parse_adaptation_field_optional(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field->optional = AdaptationFieldOptional{};

        AdaptationField& af = *_packet.adaptation_field;
        AdaptationFieldFlags& aff = *af.flags;
        AdaptationFieldOptional& afo = *af.optional;

        // Read from buffer
        auto af_buffer = p_buffer.read(af.length - adaptation_field_flags_size);

        // Set fields
        if (aff.PCR_flag)
        {
            afo.PCR = boost::dynamic_bitset<uint8_t>{ PCR_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + PCR_size, *afo.PCR);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + PCR_size);
        }

        if (aff.OPCR_flag)
        {
            afo.OPCR = boost::dynamic_bitset<uint8_t>{ OPCR_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + OPCR_size, *afo.OPCR);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + OPCR_size);
        }

        if (aff.splicing_point_flag)
        {
            afo.splice_countdown = static_cast<int8_t>(*cbegin(af_buffer));
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + splicing_countdown_size);
        }

        if (aff.transport_private_data_flag)
        {
            // Length
            afo.transport_private_data_length = *cbegin(af_buffer);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + transport_private_data_length_size);

            // Data
            afo.transport_private_data = std::vector<uint8_t>(cbegin(af_buffer), cbegin(af_buffer) + *afo.transport_private_data_length);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + *afo.transport_private_data_length);
        }

        if (aff.extension_flag)
        {
            parse_adaptation_extension(af_buffer);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + afo.extension->length);
        }

        if (af_buffer.size() > 0)
        {
            afo.stuffing_bytes = std::move(af_buffer);

            if (std::any_of(cbegin(af_buffer), cend(af_buffer),
                [](uint8_t b) { return b != stuffing_byte; }))
            {
                throw InvalidStuffingBytes(_packet, _index);
            }
        }
    }



    void PacketParser::parse_adaptation_extension(std::vector<uint8_t>& af_buffer)
    {
        _packet.adaptation_field->optional->extension = AdaptationExtension{};

        AdaptationFieldOptional& afo = *_packet.adaptation_field->optional;
        AdaptationExtension& ae = *afo.extension;

        // Set length
        ae.length = af_buffer[0];
        af_buffer.erase(begin(af_buffer));

        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> ae_bs{ adaptation_extension_flags_size * 8, af_buffer[0] };
        af_buffer.erase(begin(af_buffer));

        // Set flags
        ae.legal_time_window_flag = ae_bs.test(legal_time_window_flag_mask.find_first());
        ae.piecewise_rate_flag = ae_bs.test(piecewise_rate_flag_mask.find_first());
        ae.seamless_splice_flag = ae_bs.test(seamless_splice_flag_mask.find_first());
        ae.reserved = read_field<uint8_t>(ae_bs, reserved_mask);

        if (ae.legal_time_window_flag)
        {
            boost::dynamic_bitset<uint8_t> ltw_bs{ adaptation_extension_LTW_field_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + adaptation_extension_LTW_field_size, ltw_bs);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + adaptation_extension_LTW_field_size);

            ae.legal_time_window_valid_flag = ltw_bs.test(legal_time_window_valid_flag_mask.find_first());
            ae.legal_time_window_offset = read_field<uint16_t>(ltw_bs, legal_time_window_offset_mask);
        }
        if (ae.piecewise_rate_flag)
        {
            boost::dynamic_bitset<uint8_t> piecewise_bs{ adaptation_extension_piecewise_field_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + adaptation_extension_piecewise_field_size, piecewise_bs);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + adaptation_extension_piecewise_field_size);

            ae.piecewise_rate_reserved = read_field<uint8_t>(piecewise_bs, piecewise_rate_reserved_mask);
            ae.piecewise_rate = read_field<uint32_t>(piecewise_bs, piecewise_rate_mask);
        }
        if (ae.seamless_splice_flag)
        {
            boost::dynamic_bitset<uint8_t> seamless_bs{ adaptation_extension_seamless_field_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + adaptation_extension_seamless_field_size, seamless_bs);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + adaptation_extension_seamless_field_size);

            ae.seamless_splice_type = read_field<uint8_t>(seamless_bs, seamless_splice_type_mask);
            ae.DTS_next_access_unit = read_field<uint64_t>(seamless_bs, DTS_next_access_unit_mask);
        }
    }



    void PacketParser::parse_payload_data(PacketBuffer& p_buffer)
    {
    }
}
