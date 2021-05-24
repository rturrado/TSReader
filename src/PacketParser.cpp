#include "Exception.h"
#include "Packet.h"
#include "PacketParser.h"
#include "Tables.h"

#include <algorithm>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <iostream>

namespace TS
{
    /* static */ size_t PacketParser::_packet_index{ 0 };


    template <typename ReturnType>
    ReturnType read_field(const boost::dynamic_bitset<uint8_t>& bs, const boost::dynamic_bitset<uint8_t>& mask_bs)
    {
        auto tmp{ bs & mask_bs };
        tmp >>= mask_bs.find_first();
        return static_cast<ReturnType>(tmp.to_ulong());
    }



    void PacketParser::parse(PacketBuffer& p_buffer)
    {
        try
        {
            parse_header(p_buffer);

            if (_packet.adaptation_field_present())
            {
                parse_adaptation_field(p_buffer);
            }
            if (_packet.payload_data_present())
            {
                parse_payload_data(p_buffer);
            }
        }
        catch (const InvalidSyncByte& err)
        {
            std::ostringstream oss{};
            oss << err.what() << " " << static_cast<uint16_t>(_packet.header.sync_byte) << ", in packet " << _packet_index;
            throw std::exception(oss.str().c_str());
        }
        catch (const std::exception& err)
        {
            std::ostringstream oss{};
            oss << err.what() << ", in packet " << _packet_index;
            throw std::exception(oss.str().c_str());
        }

        _packet_index++;
    }



    void PacketParser::parse_header(PacketBuffer& p_buffer)
    {
        // Read from buffer
        auto header_buffer = p_buffer.read<true>(header_size);
        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> header_bs{ header_size * 8 };
        from_block_range(cbegin(header_buffer), cend(header_buffer), header_bs);

        // Set fields
        Header& hdr = _packet.header;

        hdr.sync_byte = read_field<uint8_t>(header_bs, sync_byte_mask_bs);
        if (hdr.sync_byte != sync_byte_valid_value)
        {
            throw InvalidSyncByte();
        }
        hdr.transport_error_indicator = header_bs.test(transport_error_indicator_mask_bs.find_first());
        if (hdr.transport_error_indicator)
        {
            throw TransportError();
        }
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
        AdaptationField& af = *_packet.adaptation_field;

        af.length = af_buffer[0];

        if (af.length > 0)
        {
            parse_adaptation_field_flags(p_buffer);
        }
        if (af.length > adaptation_field_flags_size)
        {
            parse_adaptation_field_optional(p_buffer);
        }
    }



    void PacketParser::parse_adaptation_field_flags(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field->flags = AdaptationFieldFlags{};

        // Read from buffer
        auto af_buffer = p_buffer.read(adaptation_field_flags_size);
        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> af_bs{ 8, af_buffer[0] };

        // Set fields
        AdaptationField& af = *_packet.adaptation_field;
        AdaptationFieldFlags& aff = *af.flags;

        aff.discontinuity_indicator = af_bs.test(discontinuity_indicator_mask_bs.find_first());
        aff.random_access_indicator = af_bs.test(random_access_indicator_mask_bs.find_first());
        aff.elementary_stream_priority_indicator = af_bs.test(elementary_stream_priority_indicator_mask_bs.find_first());
        aff.PCR_flag = af_bs.test(PCR_flag_mask_bs.find_first());
        aff.OPCR_flag = af_bs.test(OPCR_flag_mask_bs.find_first());
        aff.splicing_point_flag = af_bs.test(splicing_point_flag_mask_bs.find_first());
        aff.transport_private_data_flag = af_bs.test(transport_private_data_flag_mask_bs.find_first());
        aff.extension_flag = af_bs.test(extension_flag_mask_bs.find_first());
    }



    void PacketParser::parse_adaptation_field_optional(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field->optional = AdaptationFieldOptional{};

        // Read from buffer
        AdaptationField& af = *_packet.adaptation_field;

        auto af_buffer = p_buffer.read(af.length - adaptation_field_flags_size);

        // Set fields
        AdaptationFieldFlags& aff = *af.flags;
        AdaptationFieldOptional& afo = *af.optional;

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
                throw InvalidStuffingBytes();
            }
        }
    }



    void PacketParser::parse_adaptation_extension(std::vector<uint8_t>& af_buffer)
    {
        _packet.adaptation_field->optional->extension = AdaptationExtension{};

        // Set length
        AdaptationExtension& ae = *_packet.adaptation_field->optional->extension;

        ae.length = af_buffer[0];
        af_buffer.erase(begin(af_buffer));

        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> ae_bs{ adaptation_extension_flags_size * 8, af_buffer[0] };
        af_buffer.erase(begin(af_buffer));

        // Set flags
        ae.legal_time_window_flag = ae_bs.test(legal_time_window_flag_mask_bs.find_first());
        ae.piecewise_rate_flag = ae_bs.test(piecewise_rate_flag_mask_bs.find_first());
        ae.seamless_splice_flag = ae_bs.test(seamless_splice_flag_mask_bs.find_first());
        ae.reserved = read_field<uint8_t>(ae_bs, reserved_mask_bs);

        if (ae.legal_time_window_flag)
        {
            boost::dynamic_bitset<uint8_t> ltw_bs{ adaptation_extension_LTW_field_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + adaptation_extension_LTW_field_size, ltw_bs);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + adaptation_extension_LTW_field_size);

            ae.legal_time_window_valid_flag = ltw_bs.test(legal_time_window_valid_flag_mask_bs.find_first());
            ae.legal_time_window_offset = read_field<uint16_t>(ltw_bs, legal_time_window_offset_mask_bs);
        }
        if (ae.piecewise_rate_flag)
        {
            boost::dynamic_bitset<uint8_t> piecewise_bs{ adaptation_extension_piecewise_field_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + adaptation_extension_piecewise_field_size, piecewise_bs);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + adaptation_extension_piecewise_field_size);

            ae.piecewise_rate_reserved = read_field<uint8_t>(piecewise_bs, piecewise_rate_reserved_mask_bs);
            ae.piecewise_rate = read_field<uint32_t>(piecewise_bs, piecewise_rate_mask_bs);
        }
        if (ae.seamless_splice_flag)
        {
            boost::dynamic_bitset<uint8_t> seamless_bs{ adaptation_extension_seamless_field_size * 8, 0x0 };
            from_block_range(cbegin(af_buffer), cbegin(af_buffer) + adaptation_extension_seamless_field_size, seamless_bs);
            af_buffer.erase(begin(af_buffer), begin(af_buffer) + adaptation_extension_seamless_field_size);

            ae.seamless_splice_type = read_field<uint8_t>(seamless_bs, seamless_splice_type_mask_bs);
            ae.DTS_next_access_unit = read_field<uint64_t>(seamless_bs, DTS_next_access_unit_mask_bs);
        }
    }



    void PacketParser::parse_pointer(PacketBuffer& p_buffer)
    {
        _packet.payload_data->pointer = Pointer{};

        // Read from buffer
        auto ptr_buffer = p_buffer.read(pointer_field_size);

        // Set fields
        Pointer& ptr = *_packet.payload_data->pointer;

        ptr.pointer_field = ptr_buffer[0];  // pointer field

        if (ptr.pointer_field != 0)
        {
            ptr.pointer_filler_bytes = p_buffer.read(ptr.pointer_field);  // pointer filler bytes

            // If first of the filler bytes is 0xFF, then the rest of the bytes have to be 0xFF as well
            if ((*ptr.pointer_filler_bytes)[0] == stuffing_byte)
            {
                if (std::any_of(cbegin(*ptr.pointer_filler_bytes) + 1, cend(*ptr.pointer_filler_bytes),
                    [](uint8_t b) { return b != stuffing_byte; }))
                {
                    throw InvalidStuffingBytes();
                }
            }
            else
            {
                throw Unimplemented("parsing of pointer filler bytes containing end of section");
            }
        }
    }



    void PacketParser::parse_table_header(PacketBuffer& p_buffer)
    {
        // Read from buffer
        auto th_buffer = p_buffer.read(table_header_size);

        // Set fields
        TableHeader& th = *_packet.payload_data->table_header;

        th.table_id = th_buffer[0];

        // Check end of table section repeat case
        if (th.table_id == stuffing_byte)
        {
            th_buffer = p_buffer.read(p_buffer.size() - p_buffer.get_read_position());

            if (std::any_of(cbegin(th_buffer), cend(th_buffer),
                [](uint8_t b) { return b != stuffing_byte; }))
            {
                throw InvalidStuffingBytes();
            }
        }
        else
        {
            // Create bitset from buffer
            boost::dynamic_bitset<uint8_t> th_bs{ (table_header_size - table_id_size) * 8, 0x0 };
            from_block_range(cbegin(th_buffer) + table_id_size, cend(th_buffer), th_bs);

            th.section_syntax_indicator = th_bs.test(section_syntax_indicator_mask_bs.find_first());
            th.private_bit = th_bs.test(private_bit_mask_bs.find_first());
            th.reserved_bits = read_field<uint8_t>(th_bs, reserved_bits_mask_bs);
            th.section_length_unused_bits = read_field<uint8_t>(th_bs, section_length_unused_bits_mask_bs);
            th.section_length = read_field<uint16_t>(th_bs, section_length_mask_bs);
        }
    }



    void PacketParser::parse_payload_data(PacketBuffer& p_buffer)
    {
        _packet.payload_data = PayloadData{};

        if (_packet.pointer_present())
        {
            parse_pointer(p_buffer);
        }

        if (_packet.payload_contains_PAT_table())
        {
            parse_PAT_table(p_buffer);
        }
        else if (_packet.payload_contains_CAT_table())
        {
            throw Unimplemented{ "parsing of CAT table" };
        }
        else if (_packet.payload_contains_NIT_table())
        {
            throw Unimplemented{ "parsing of NIT table" };
        }
        else if (PAT_map.contains(_packet.get_PID()))
        {
            parse_PMT_table(p_buffer);
        }
        else if (PMT_map.contains(_packet.get_PID()))
        {
            parse_elementary_stream_specific_data(p_buffer);
        }
    }



    void PacketParser::parse_PAT_table(PacketBuffer& p_buffer)
    {
    }



    void PacketParser::parse_PMT_table(PacketBuffer& p_buffer)
    {
    }



    void PacketParser::parse_table_syntax(PacketBuffer& p_buffer)
    {
    }



    void PacketParser::parse_elementary_stream_specific_data(PacketBuffer& p_buffer)
    {
    }
}
