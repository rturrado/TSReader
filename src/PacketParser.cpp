#include "Exception.h"
#include "Packet.h"
#include "PacketParser.h"
#include "Tables.h"

#include <algorithm>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <iostream>
#include <iterator>

namespace TS
{
    /* static */ size_t PacketParser::_packet_index{ 0 };

    template <typename ReturnType>
    ReturnType read_field(const boost::dynamic_bitset<uint8_t>& bs, const boost::dynamic_bitset<uint8_t>& mask_bs)
    {
        auto tmp_bs{ bs & mask_bs };
        tmp_bs >>= mask_bs.find_first();
        return static_cast<ReturnType>(tmp_bs.to_ulong());
    }

    bool all_field_bits_set(const boost::dynamic_bitset<uint8_t>& bs, const boost::dynamic_bitset<uint8_t>& mask_bs)
    {
        auto tmp_bs{ bs & mask_bs };
        return tmp_bs.to_ulong() == mask_bs.to_ulong();
    }

    bool all_field_bits_unset(const boost::dynamic_bitset<uint8_t>& bs, const boost::dynamic_bitset<uint8_t>& mask_bs)
    {
        auto tmp_bs{ bs & mask_bs };
        return tmp_bs.to_ulong() == 0;
    }

    void PacketParser::parse(PacketBuffer& p_buffer)
    {
        parse_header(p_buffer);

        if (_packet.has_adaptation_field())
        {
            parse_adaptation_field(p_buffer);
        }
        if (_packet.has_payload_data())
        {
            parse_payload_data(p_buffer);
        }

        PacketParser::_packet_index++;
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

        hdr.sync_byte = read_field<uint8_t>(header_bs, hdr_sync_byte_mask_bs);
        if (hdr.sync_byte != sync_byte_valid_value)
        {
            throw InvalidSyncByte{};
        }
        hdr.transport_error_indicator = header_bs.test(hdr_transport_error_indicator_mask_bs.find_first());
        if (hdr.transport_error_indicator)
        {
            throw TransportError{};
        }
        hdr.payload_unit_start_indicator = header_bs.test(hdr_payload_unit_start_indicator_mask_bs.find_first());
        hdr.transport_priority = header_bs.test(hdr_transport_priority_mask_bs.find_first());
        hdr.PID = read_field<uint16_t>(header_bs, hdr_PID_mask_bs);
        hdr.transport_scrambling_control = read_field<uint8_t>(header_bs, hdr_transport_scrambling_control_mask_bs);
        hdr.adaptation_field_control = read_field<uint8_t>(header_bs, hdr_adaptation_field_control_mask_bs);
        hdr.continuity_counter = read_field<uint8_t>(header_bs, hdr_continuity_counter_mask_bs);
    }

    void PacketParser::parse_adaptation_field(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field = AdaptationField{};

        // Read from buffer
        auto af_buffer = p_buffer.read(af_length_size);

        // Set fields
        AdaptationField& af = *_packet.adaptation_field;

        af.length = af_buffer[0];

        if (af.length > 0)
        {
            parse_adaptation_field_flags(p_buffer);
        }
        if (af.length > af_flags_size)
        {
            parse_adaptation_field_optional(p_buffer);
        }
    }

    void PacketParser::parse_adaptation_field_flags(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field->flags = AdaptationFieldFlags{};

        // Read from buffer
        auto af_buffer = p_buffer.read(af_flags_size);
        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> af_bs{ 8, af_buffer[0] };

        // Set fields
        AdaptationField& af = *_packet.adaptation_field;
        AdaptationFieldFlags& aff = *af.flags;

        aff.discontinuity_indicator = af_bs.test(af_discontinuity_indicator_mask_bs.find_first());
        aff.random_access_indicator = af_bs.test(af_random_access_indicator_mask_bs.find_first());
        aff.elementary_stream_priority_indicator = af_bs.test(af_elementary_stream_priority_indicator_mask_bs.find_first());
        aff.PCR_flag = af_bs.test(af_PCR_flag_mask_bs.find_first());
        aff.OPCR_flag = af_bs.test(af_OPCR_flag_mask_bs.find_first());
        aff.splicing_point_flag = af_bs.test(af_splicing_point_flag_mask_bs.find_first());
        aff.transport_private_data_flag = af_bs.test(af_transport_private_data_flag_mask_bs.find_first());
        aff.extension_flag = af_bs.test(af_extension_flag_mask_bs.find_first());
    }

    void PacketParser::parse_adaptation_field_optional(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field->optional = AdaptationFieldOptional{};

        // Set fields
        AdaptationField& af = *_packet.adaptation_field;
        AdaptationFieldFlags& aff = *af.flags;
        AdaptationFieldOptional& afo = *af.optional;

        auto af_optional_size{ 0 };

        if (aff.PCR_flag)
        {
            auto pcr_buffer = p_buffer.read<true>(afo_PCR_size);
            afo.PCR = boost::dynamic_bitset<uint8_t>{ afo_PCR_size * 8, 0x0 };
            from_block_range(cbegin(pcr_buffer), cbegin(pcr_buffer) + afo_PCR_size, *afo.PCR);
            af_optional_size += afo_PCR_size;
        }

        if (aff.OPCR_flag)
        {
            auto opcr_buffer = p_buffer.read<true>(afo_OPCR_size);
            afo.OPCR = boost::dynamic_bitset<uint8_t>{ afo_OPCR_size * 8, 0x0 };
            from_block_range(cbegin(opcr_buffer), cbegin(opcr_buffer) + afo_OPCR_size, *afo.OPCR);
            af_optional_size += afo_OPCR_size;
        }

        if (aff.splicing_point_flag)
        {
            auto splice_countdown_buffer = p_buffer.read<true>(afo_splicing_countdown_size);
            afo.splice_countdown = static_cast<int8_t>(*cbegin(splice_countdown_buffer));
            af_optional_size += afo_splicing_countdown_size;
        }

        if (aff.transport_private_data_flag)
        {
            // Length
            auto transport_private_data_length_buffer = p_buffer.read<true>(afo_transport_private_data_length_size);
            afo.transport_private_data_length = *cbegin(transport_private_data_length_buffer);
            af_optional_size += afo_transport_private_data_length_size;

            // Data
            if (afo.transport_private_data_length != 0)
            {
                afo.transport_private_data = p_buffer.read<true>(*afo.transport_private_data_length);
                af_optional_size += *afo.transport_private_data_length;
            }
        }

        if (aff.extension_flag)
        {
            parse_adaptation_extension(p_buffer);
            af_optional_size += afo.extension->length;
        }

        if (auto af_stuffing_bytes_size = af.length - af_flags_size - af_optional_size; af_stuffing_bytes_size > 0)
        {
            afo.stuffing_bytes = p_buffer.read<true>(af_stuffing_bytes_size);

            if (std::any_of(cbegin(*afo.stuffing_bytes), cend(*afo.stuffing_bytes),
                [](uint8_t b) { return b != stuffing_byte; }))
            {
                throw InvalidStuffingBytes{};
            }
        }
    }

    void PacketParser::parse_adaptation_extension(PacketBuffer& p_buffer)
    {
        _packet.adaptation_field->optional->extension = AdaptationExtension{};

        // Read from buffer
        auto ae_buffer = p_buffer.read<true>(adaptation_extension_header_size);
        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> ae_bs{ adaptation_extension_header_size * 8 };
        from_block_range(cbegin(ae_buffer), cend(ae_buffer), ae_bs);

        AdaptationExtension& ae = *_packet.adaptation_field->optional->extension;

        // Set length
        ae.length = read_field<uint8_t>(ae_bs, ae_length_mask_bs);

        // Set flags
        ae.legal_time_window_flag = ae_bs.test(ae_legal_time_window_flag_mask_bs.find_first());
        ae.piecewise_rate_flag = ae_bs.test(ae_piecewise_rate_flag_mask_bs.find_first());
        ae.seamless_splice_flag = ae_bs.test(ae_seamless_splice_flag_mask_bs.find_first());
        ae.reserved = read_field<uint8_t>(ae_bs, ae_reserved_mask_bs);

        // Set optional fields
        if (ae.legal_time_window_flag)
        {
            auto ltw_buffer = p_buffer.read<true>(aeo_LTW_field_size);
            boost::dynamic_bitset<uint8_t> ltw_bs{ aeo_LTW_field_size * 8, 0x0 };
            from_block_range(cbegin(ltw_buffer), cend(ltw_buffer), ltw_bs);

            ae.legal_time_window_valid_flag = ltw_bs.test(aeo_legal_time_window_valid_flag_mask_bs.find_first());
            ae.legal_time_window_offset = read_field<uint16_t>(ltw_bs, aeo_legal_time_window_offset_mask_bs);
        }
        if (ae.piecewise_rate_flag)
        {
            auto piecewise_buffer = p_buffer.read<true>(aeo_piecewise_field_size);
            boost::dynamic_bitset<uint8_t> piecewise_bs{ aeo_piecewise_field_size * 8, 0x0 };
            from_block_range(cbegin(piecewise_buffer), cend(piecewise_buffer), piecewise_bs);

            ae.piecewise_rate_reserved = read_field<uint8_t>(piecewise_bs, aeo_piecewise_rate_reserved_mask_bs);
            ae.piecewise_rate = read_field<uint32_t>(piecewise_bs, aeo_piecewise_rate_mask_bs);
        }
        if (ae.seamless_splice_flag)
        {
            auto seamless_buffer = p_buffer.read<true>(aeo_seamless_field_size);
            boost::dynamic_bitset<uint8_t> seamless_bs{ aeo_seamless_field_size * 8, 0x0 };
            from_block_range(cbegin(seamless_buffer), cend(seamless_buffer), seamless_bs);

            ae.seamless_splice_type = read_field<uint8_t>(seamless_bs, aeo_seamless_splice_type_mask_bs);
            ae.DTS_next_access_unit = read_field<uint64_t>(seamless_bs, aeo_DTS_next_access_unit_mask_bs);
        }
    }

    void PacketParser::parse_payload_data(PacketBuffer& p_buffer)
    {
        _packet.payload_data = PayloadData{};

        // Check if we are parsing a PSI or a PES payload
        if (_packet.payload_contains_PSI())
        {
            parse_payload_data_as_PSI(p_buffer);
        }
        else
        {
            parse_payload_data_as_PES(p_buffer);
        }
    }

    void PacketParser::parse_payload_data_as_PES(PacketBuffer& p_buffer)
    {
        // ****
        // TODO: read payload data in Big Endian?
        // TODO: write to file and check if files can be played
        // ****
        auto pd_buffer = p_buffer.read(p_buffer.size_not_read());

        auto PES_PID = _packet.get_PID();

        auto& PMT_map = get_PMT_map();
        if (PMT_map.contains(PES_PID))
        {
            auto& PES_map = get_PES_map();
            auto& essd_data = PES_map[PES_PID];
            // ****
            // TODO: it may be a lot clearer to do these actions in a different stage, after the parsing stage
            // TODO: actually, this action may be optional, e.g. we may not store some stream data
            // ****
            std::copy(cbegin(pd_buffer), cend(pd_buffer), std::back_inserter(essd_data));
        }
        else
        {
            throw UnknownPES{};
        }
    }

    void PacketParser::parse_payload_data_as_PSI(PacketBuffer& p_buffer)
    {
        if (_packet.get_payload_unit_start_indicator())
        {
            parse_pointer(p_buffer);
        }

        parse_table_header(p_buffer);

        TableHeader& th = *_packet.payload_data->table_header;
        if (th.has_syntax_section())
        {
            parse_table_syntax_section(p_buffer);
        }
    }

    void PacketParser::parse_pointer(PacketBuffer& p_buffer)
    {
        _packet.payload_data->pointer = Pointer{};

        // Read from buffer
        auto ptr_buffer = p_buffer.read(ptr_pointer_field_size);

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
                    throw InvalidStuffingBytes{};
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
        _packet.payload_data->table_header = TableHeader{};

        // Read from buffer
        auto th_buffer = p_buffer.read<true>(table_header_size);

        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> th_bs{ table_header_size * 8, 0x0 };
        from_block_range(cbegin(th_buffer), cend(th_buffer), th_bs);

        // Set fields
        TableHeader& th = *_packet.payload_data->table_header;

        th.table_id = read_field<uint8_t>(th_bs, th_table_id_mask_bs);

        // Check end of table section repeat case
        if (th.table_id == stuffing_byte)
        {
            th_buffer = p_buffer.read(p_buffer.size_not_read());

            if (std::any_of(cbegin(th_buffer), cend(th_buffer),
                [](uint8_t b) { return b != stuffing_byte; }))
            {
                throw InvalidStuffingBytes{};
            }
        }
        else
        {
            bool payload_contains_PAT_CAT_or_PMT_table = _packet.payload_contains_PAT_table()
                || _packet.payload_contains_CAT_table()
                || _packet.payload_contains_PMT_table();
            th.section_syntax_indicator = th_bs.test(th_section_syntax_indicator_mask_bs.find_first());
            if (th.section_syntax_indicator != payload_contains_PAT_CAT_or_PMT_table)
            {
                throw InvalidSectionSyntaxIndicator{};
            }
            th.private_bit = th_bs.test(th_private_bit_mask_bs.find_first());
            if (th.private_bit == payload_contains_PAT_CAT_or_PMT_table)
            {
                throw InvalidPrivateBit{};
            }
            if (not all_field_bits_set(th_bs, th_reserved_bits_mask_bs))
            {
                throw InvalidReservedBits{};
            }
            if (not all_field_bits_unset(th_bs, th_section_length_unused_bits_mask_bs))
            {
                throw InvalidUnusedBits{};
            }
            th.section_length = read_field<uint16_t>(th_bs, th_section_length_mask_bs);
            if (th.section_length > th_max_section_length)
            {
                throw InvalidSectionLength{};
            }
            if (th.section_length > p_buffer.size_not_read())
            {
                throw Unimplemented{ "PSI table spanning across different packets" };
            }
        }
    }

    void PacketParser::parse_table_syntax_section(PacketBuffer& p_buffer)
    {
        _packet.payload_data->table_header->table_syntax = TableSyntax{};

        // Read from buffer
        auto ts_buffer = p_buffer.read(table_syntax_section_size);

        // Set fields
        TableSyntax& ts = *_packet.payload_data->table_header->table_syntax;

        ts.table_id_extension = *(reinterpret_cast<uint16_t*>(&ts_buffer[0]));
        ts_buffer.erase(begin(ts_buffer), begin(ts_buffer) + tss_table_id_extension_size);

        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> ts_bs{ tss_reserved_version_current_next_indicator_size * 8, ts_buffer[0] };

        if (not all_field_bits_set(ts_bs, tss_reserved_bits_mask_bs))
        {
            throw InvalidReservedBits{};
        }
        ts.version_number = read_field<uint8_t>(ts_bs, tss_version_number_mask_bs);
        // ****
        // TODO: should we consider version number to update or not PAT_map?
        // ****
        ts.current_next_indicator = ts_bs.test(tss_current_next_indicator_mask_bs.find_first());
        // ****
        // TODO: should we store table data in PAT_map if current/next indicator is true,
        //       and store table data in a buffer if current/next indicator is false?
        // ****
        ts_buffer.erase(begin(ts_buffer), begin(ts_buffer) + tss_reserved_version_current_next_indicator_size);

        ts.section_number = ts_buffer[0];
        ts.last_section_number = ts_buffer[1];
        ts_buffer.erase(begin(ts_buffer), begin(ts_buffer) + tss_section_number_size + tss_last_section_number_size);

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
        else if (_packet.payload_contains_PMT_table())
        {
            parse_PMT_table(p_buffer);
        }
        // TODO: read and check CRC32
    }

    void PacketParser::parse_PAT_table(PacketBuffer& p_buffer)
    {
        TableHeader& th = *_packet.payload_data->table_header;

        const uint16_t table_data_size = th.section_length
            - table_syntax_section_size
            - tss_crc32_size;

        // Read from buffer
        auto td_buffer = p_buffer.read<true>(static_cast<uint8_t>(table_data_size));

        if (td_buffer.size() % PAT_table_data_program_size != 0)
        {
            throw InvalidPATTableDataSize{};
        }

        for (auto it = cbegin(td_buffer); it != cend(td_buffer); it += PAT_table_data_program_size)
        {
            // Create bitset from buffer
            boost::dynamic_bitset<uint8_t> td_bs{ PAT_table_data_program_size * 8, 0x0 };
            from_block_range(it, it + PAT_table_data_program_size, td_bs);

            uint16_t program_num = read_field<uint16_t>(td_bs, PAT_table_data_table_id_extension_mask_bs);
            if (not all_field_bits_set(td_bs, PAT_table_data_reserved_bits_mask_bs))
            {
                throw InvalidReservedBits{};
            }
            uint16_t program_map_PID = read_field<uint16_t>(td_bs, PAT_table_data_program_map_PID_mask_bs);

            // ****
            // TODO: it may be a lot clearer to do these actions in a different stage, after the parsing stage
            // ****

            // Update PAT table
            TPAT_map& PAT_map = get_PAT_map();
            PAT_map[program_map_PID] = program_num;

            // Update NIT PID if needed
            if (program_num == NIT_program_num && program_map_PID != default_NIT_PID)
            {
                set_NIT_PID(program_map_PID);
            }
        }
    }

    void PacketParser::parse_PMT_table(PacketBuffer& p_buffer)
    {
        _packet.payload_data->table_header->table_syntax->PMT_table = PMTTable{};

        // Read from buffer
        auto td_buffer = p_buffer.read<true>(static_cast<uint8_t>(PMT_table_data_header_size));

        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> td_bs{ PMT_table_data_header_size * 8, 0x0 };
        from_block_range(cbegin(td_buffer), cend(td_buffer), td_bs);

        // Set fields
        TableHeader& th = *_packet.payload_data->table_header;
        PMTTable& pmtt = *th.table_syntax->PMT_table;

        if (not all_field_bits_set(td_bs, PMT_reserved_bits_mask_bs))
        {
            throw InvalidReservedBits{};
        }
        pmtt.PCR_PID = read_field<uint16_t>(td_bs, PMT_reserved_bits_mask_bs);
        if (not all_field_bits_set(td_bs, PMT_reserved_bits_2_mask_bs))
        {
            throw InvalidReservedBits{};
        }
        if (not all_field_bits_unset(td_bs, PMT_program_info_length_unused_bits_mask_bs))
        {
            throw InvalidUnusedBits{};
        }
        pmtt.program_info_length = read_field<uint16_t>(td_bs, PMT_program_info_length_bs);
        if (pmtt.program_info_length != 0)
        {
            throw Unimplemented{ "parsing of PTM program descriptors" };
        }

        uint16_t elementary_stream_specific_data_size = th.section_length
            - table_syntax_section_size
            - PMT_table_data_header_size
            - pmtt.program_info_length
            - tss_crc32_size;
        if (elementary_stream_specific_data_size != 0)
        {
            parse_elementary_stream_specific_data(p_buffer, elementary_stream_specific_data_size);
        }
    }

    void PacketParser::parse_elementary_stream_specific_data(PacketBuffer& p_buffer, uint16_t elementary_stream_specific_data_size)
    {
        TableHeader& th = *_packet.payload_data->table_header;
        PMTTable& pmtt = *th.table_syntax->PMT_table;
        pmtt.ESSD_info_data = std::vector<ESSD>{};

        while (elementary_stream_specific_data_size)
        {
            // Read from buffer
            auto essd_buffer = p_buffer.read<true>(ESSD_header_size);

            // Create bitset from buffer
            boost::dynamic_bitset<uint8_t> essd_bs{ ESSD_header_size * 8, 0x0 };
            from_block_range(cbegin(essd_buffer), cend(essd_buffer), essd_bs);

            // Set fields
            ESSD essd{};

            essd.stream_type = read_field<uint8_t>(essd_bs, ESSD_stream_type_mask_bs);
            if (not all_field_bits_set(essd_bs, ESSD_reserved_bits_mask_bs))
            {
                throw InvalidReservedBits{};
            }
            essd.elementary_PID = read_field<uint16_t>(essd_bs, ESSD_elementary_PID_mask_bs);
            if (not all_field_bits_set(essd_bs, ESSD_reserved_bits_2_mask_bs))
            {
                throw InvalidReservedBits{};
            }
            if (not all_field_bits_unset(essd_bs, ESSD_info_length_unused_bits_mask_bs))
            {
                throw InvalidUnusedBits{};
            }
            essd.info_length = read_field<uint16_t>(essd_bs, ESSD_info_length_mask_bs);
            if (essd.info_length != 0)
            {
                essd.descriptors = parse_descriptors(p_buffer, essd.info_length);
            }

            pmtt.ESSD_info_data->push_back(essd);

            // ****
            // TODO: it may be a lot clearer to do these actions in a different stage, after the parsing stage
            // ****
            
            // Update PMT table
            TPMT_map& PMT_map = get_PMT_map();
            PMT_map[essd.elementary_PID] = essd.stream_type;

            elementary_stream_specific_data_size -= (ESSD_header_size + essd.info_length);
        }
    }

    std::vector<Descriptor> PacketParser::parse_descriptors(PacketBuffer& p_buffer, uint16_t descriptors_size)
    {
        std::vector<Descriptor> ret{};

        while (descriptors_size)
        {
            // Read from buffer
            auto dsc_buffer = p_buffer.read<true>(descriptor_header_size);

            // Create bitset from buffer
            boost::dynamic_bitset<uint8_t> dsc_bs{ descriptor_header_size * 8, 0x0 };
            from_block_range(cbegin(dsc_buffer), cend(dsc_buffer), dsc_bs);

            // Set fields
            Descriptor descriptor{};

            descriptor.tag = read_field<uint8_t>(dsc_bs, dsc_tag_mask_bs);
            descriptor.length = read_field<uint8_t>(dsc_bs, dsc_length_mask_bs);
            if (descriptor.length != 0)
            {
                descriptor.data = p_buffer.read<true>(descriptor.length);
            }

            ret.push_back(descriptor);

            descriptors_size -= (descriptor_header_size + descriptor.length);
        }

        return ret;
    }
}
