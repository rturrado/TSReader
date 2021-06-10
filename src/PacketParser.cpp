#include "Exception.hpp"
#include "Packet.hpp"
#include "PacketParser.hpp"
#include "PSI_Tables.hpp"

#include <algorithm>
#include <boost/crc.hpp>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <boost/endian/conversion.hpp>
#include <iostream>
#include <iterator>

using namespace boost::endian;

/*
TS headers are encoded as Big Endian (most significative byte in lowest memory address).
TS payloads are written out in the same byte order as they are read.

To parse a field from the TS file                     [table header file block]          2    1    0        (index)
(e.g. table header's section length):                                                  <---------------
                                                                                       | 0d | b0 | 00 |     (value)
                                                                                       <---------------

1) Read the TS package into a byte buffer.            [table header buffer]              2    1    0
2) Create the bitset from the buffer.                 [table header bitset]            <---------------
                                                                                       | 0d | b0 | 00 |
                                                                                       <---------------

3) Bitwise-and the bitset with the mask bitset        [section length mask bitset]       2    1    0
   into a temporary.                                                                   <---------------
                                                                                       | ff | 03 | 00 |
                                                                                       <---------------

                                                      [tmp bitset]                       2    1    0
                                                                                       <---------------
                                                                                       | 0d | 00 | 00 |
                                                                                       <---------------

4) Right shift the temporary bitset so that           [tmp bitset]                       2    1    0
   the masked value is at the lowest position.                                         <---------------
                                                                                       | 00 | 0d | 00 |
                                                                                       <---------------

5) Convert the value to ulong.                        [ulong]                          0x00'00'0d'00
6) Cast it to the return type.                        [uint32_t]                       0x00'00'0d'00  // Big Endian
7) Convert the value from Big Endian to native.       [uint32_t]                       0x00'00'00'0d  // Native
*/

namespace TS
{
    /* static */
    size_t PacketParser::_packet_index{ 0 };

    template <uint8_t buffer_size>
    auto from_packet_buffer(PacketBuffer& p_buffer)
    {
        // Read from packet buffer into byte buffer
        auto buffer = p_buffer.read(buffer_size);

        // Create bitset from buffer
        boost::dynamic_bitset<uint8_t> ret_bs{ buffer_size * 8 };
        from_block_range(cbegin(buffer), cend(buffer), ret_bs);

        return ret_bs;
    }

    template <typename ReturnType>
    ReturnType read_field(const boost::dynamic_bitset<uint8_t>& bs, const boost::dynamic_bitset<uint8_t>& mask_bs)
    {
        auto tmp_bs{ bs & mask_bs };
        tmp_bs >>= mask_bs.find_first();
        return big_to_native(static_cast<ReturnType>(tmp_bs.to_ulong()));
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
        // Create bitset from packet buffer
        auto header_bs = from_packet_buffer<header_size>(p_buffer);

        // Set fields
        Header& hdr = _packet.header;

        hdr.sync_byte = read_field<uint8_t>(header_bs, hdr_sync_byte_mask_bs);
        hdr.transport_error_indicator = header_bs.test(hdr_transport_error_indicator_mask_bs.find_first());

        if (hdr.sync_byte != sync_byte_valid_value) { throw InvalidSyncByte{}; }
        if (hdr.transport_error_indicator) { throw TransportError{}; }

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

        af.length = *cbegin(af_buffer);

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

        // Create bitset from packet buffer
        auto af_bs = from_packet_buffer<af_flags_size>(p_buffer);

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
            afo.PCR = from_packet_buffer<afo_PCR_size>(p_buffer);
            af_optional_size += afo_PCR_size;
        }

        if (aff.OPCR_flag)
        {
            afo.OPCR = from_packet_buffer<afo_OPCR_size>(p_buffer);
            af_optional_size += afo_OPCR_size;
        }

        if (aff.splicing_point_flag)
        {
            auto splice_countdown_buffer = p_buffer.read(afo_splicing_countdown_size);
            afo.splice_countdown = static_cast<int8_t>(*cbegin(splice_countdown_buffer));
            af_optional_size += afo_splicing_countdown_size;
        }

        if (aff.transport_private_data_flag)
        {
            // Length
            auto transport_private_data_length_buffer = p_buffer.read(afo_transport_private_data_length_size);
            afo.transport_private_data_length = *cbegin(transport_private_data_length_buffer);
            af_optional_size += afo_transport_private_data_length_size;

            // Data
            if (afo.transport_private_data_length != 0)
            {
                afo.transport_private_data = p_buffer.read(*afo.transport_private_data_length);
                af_optional_size += *afo.transport_private_data_length;
            }
        }

        if (aff.extension_flag)
        {
            parse_adaptation_extension(p_buffer);
            af_optional_size += afo.extension->length;
        }

        if (auto af_stuffing_bytes_size = af.length - af_flags_size - af_optional_size;
            af_stuffing_bytes_size > 0)
        {
            afo.stuffing_bytes = p_buffer.read(af_stuffing_bytes_size);

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

        // Create bitset from packet buffer
        auto ae_bs = from_packet_buffer<adaptation_extension_header_size>(p_buffer);

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
            auto ltw_bs = from_packet_buffer<aeo_LTW_field_size>(p_buffer);

            ae.legal_time_window_valid_flag = ltw_bs.test(aeo_legal_time_window_valid_flag_mask_bs.find_first());
            ae.legal_time_window_offset = read_field<uint16_t>(ltw_bs, aeo_legal_time_window_offset_mask_bs);
        }
        if (ae.piecewise_rate_flag)
        {
            auto piecewise_bs = from_packet_buffer<aeo_piecewise_field_size>(p_buffer);

            ae.piecewise_rate_reserved = read_field<uint8_t>(piecewise_bs, aeo_piecewise_rate_reserved_mask_bs);
            ae.piecewise_rate = read_field<uint32_t>(piecewise_bs, aeo_piecewise_rate_mask_bs);
        }
        if (ae.seamless_splice_flag)
        {
            auto seamless_bs = from_packet_buffer<aeo_seamless_field_size>(p_buffer);

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
        _packet.payload_data->PES_data = p_buffer.read(p_buffer.size_not_read());
    }

    void PacketParser::parse_payload_data_as_PSI(PacketBuffer& p_buffer)
    {
        if (_packet.get_payload_unit_start_indicator())
        {
            parse_pointer(p_buffer);
        }

        parse_table_header(p_buffer);
    }

    void PacketParser::parse_pointer(PacketBuffer& p_buffer)
    {
        _packet.payload_data->pointer = Pointer{};

        // Read from buffer
        auto ptr_buffer = p_buffer.read(ptr_pointer_field_size);

        // Set fields
        Pointer& ptr = *_packet.payload_data->pointer;

        ptr.pointer_field = *cbegin(ptr_buffer);  // pointer field

        if (ptr.pointer_field != 0)
        {
            ptr.pointer_filler_bytes = p_buffer.read(ptr.pointer_field);  // pointer filler bytes

            // If first of the filler bytes is 0xFF, then the rest of the bytes have to be 0xFF as well
            if (not check_and_parse_stuffing_bytes_section(*ptr.pointer_filler_bytes))
            {
                throw Unimplemented("parsing of pointer filler bytes containing end of section");
            }
        }
    }

    void PacketParser::parse_table_header(PacketBuffer& p_buffer)
    {
        _packet.payload_data->table_header = TableHeader{};

        // Save packet buffer start read position
        auto packet_buffer_start_pos = p_buffer.get_read_position();

        // Create bitset from packet buffer
        auto th_bs = from_packet_buffer<table_header_size>(p_buffer);

        // Set fields
        TableHeader& th = *_packet.payload_data->table_header;
        th.table_id = read_field<uint8_t>(th_bs, th_table_id_mask_bs);

        // Check end of table section repeat case
        if (check_and_parse_stuffing_bytes_section(th.table_id, p_buffer.size_not_read(), p_buffer))
        {
            return;
        }

        bool payload_contains_PAT_CAT_or_PMT_table = _packet.payload_contains_PAT_table()
            || _packet.payload_contains_CAT_table()
            || _packet.payload_contains_PMT_table();

        th.section_syntax_indicator = th_bs.test(th_section_syntax_indicator_mask_bs.find_first());
        th.private_bit = th_bs.test(th_private_bit_mask_bs.find_first());
        th.section_length = read_field<uint16_t>(th_bs, th_section_length_mask_bs);

        if (th.section_syntax_indicator != payload_contains_PAT_CAT_or_PMT_table) { throw InvalidSectionSyntaxIndicator{}; }
        if (th.private_bit == payload_contains_PAT_CAT_or_PMT_table) { throw InvalidPrivateBit{}; }
        if (not all_field_bits_set(th_bs, th_reserved_bits_mask_bs)) { throw InvalidReservedBits{}; }
        if (not all_field_bits_unset(th_bs, th_section_length_unused_bits_mask_bs)) { throw InvalidUnusedBits{}; }

        if (th.section_length > th_max_section_length) { throw InvalidSectionLength{}; }
        if (th.section_length > p_buffer.size_not_read()) { throw Unimplemented{ "PSI table spanning across different packets" }; }
            
        if (th.has_syntax_section())
        {
            parse_table_syntax_section(p_buffer);

            // Save packet buffer end read position
            auto packet_buffer_end_pos = p_buffer.get_read_position();

            // Check CRC32
            auto byte_count{ packet_buffer_end_pos - packet_buffer_start_pos - tss_crc32_size };
            using crc_32_mpeg2 = boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0x00000000, false, false>;
            crc_32_mpeg2 result{};
            result.process_bytes(p_buffer.data() + packet_buffer_start_pos, byte_count);
            if (th.table_syntax->crc32 != result.checksum())
            {
                throw InvalidCRC32{};
            }
        }
    }

    bool PacketParser::check_and_parse_stuffing_bytes_section(const std::vector<uint8_t>& buffer) const
    {
        if (*cbegin(buffer) == stuffing_byte)
        {
            if (std::any_of(cbegin(buffer) + 1, cend(buffer),
                [](uint8_t b) { return b != stuffing_byte; }))
            {
                throw InvalidStuffingBytes{};
            }

            return true;
        }

        return false;
    }

    bool PacketParser::check_and_parse_stuffing_bytes_section(
        uint8_t first_byte, uint8_t bytes_to_read, PacketBuffer& p_buffer) const
    {
        if (first_byte == stuffing_byte)
        {
            auto buffer = p_buffer.read(bytes_to_read);

            if (std::any_of(cbegin(buffer), cend(buffer),
                [](uint8_t b) { return b != stuffing_byte; }))
            {
                throw InvalidStuffingBytes{};
            }

            return true;
        }
        return false;
    }

    void PacketParser::parse_table_syntax_section(PacketBuffer& p_buffer)
    {
        _packet.payload_data->table_header->table_syntax = TableSyntax{};

        // Create bitset from packet buffer
        auto ts_bs = from_packet_buffer<table_syntax_section_size>(p_buffer);

        // Set fields
        TableSyntax& ts = *_packet.payload_data->table_header->table_syntax;

        ts.table_id_extension = read_field<uint16_t>(ts_bs, tss_table_id_extension_mask_bs);

        if (not all_field_bits_set(ts_bs, tss_reserved_bits_mask_bs)) { throw InvalidReservedBits{}; }

        ts.version_number = read_field<uint8_t>(ts_bs, tss_version_number_mask_bs);
        ts.current_next_indicator = ts_bs.test(tss_current_next_indicator_mask_bs.find_first());
        ts.section_number = read_field<uint8_t>(ts_bs, tss_section_number_mask_bs);
        ts.last_section_number = read_field<uint8_t>(ts_bs, tss_last_section_number_mask_bs);

        if (_packet.payload_contains_PAT_table())
        {
            parse_PAT_table(p_buffer);
        }
        else if (_packet.payload_contains_CAT_table()) { throw Unimplemented{ "parsing of CAT table" }; }
        else if (_packet.payload_contains_NIT_table()) { throw Unimplemented{ "parsing of NIT table" }; }
        else if (_packet.payload_contains_PMT_table())
        {
            parse_PMT_table(p_buffer);
        }

        parse_CRC32(p_buffer);
    }

    void PacketParser::parse_PAT_table(PacketBuffer& p_buffer)
    {
        // Check PAT table size is not null
        TableHeader& th = *_packet.payload_data->table_header;

        const uint16_t table_data_size = th.section_length
            - table_syntax_section_size
            - tss_crc32_size;
        if (table_data_size == 0)
        {
            return;
        }

        // Create the PAT table
        _packet.payload_data->table_header->table_syntax->table_data = PAT_Table{};

        TableSyntax& ts = *th.table_syntax;
        PAT_Table& patt = std::get<PAT_Table>(ts.table_data);

        for (auto i = 0; i < table_data_size; i += PAT_table_data_program_size)
        {
            // Create bitset from packet buffer
            auto pat_entry_bs = from_packet_buffer<PAT_table_data_program_size>(p_buffer);

            // Set fields
            if (not all_field_bits_set(pat_entry_bs, PAT_table_data_reserved_bits_mask_bs)) { throw InvalidReservedBits{}; }

            uint16_t program_num = read_field<uint16_t>(pat_entry_bs, PAT_table_data_table_id_extension_mask_bs);
            uint16_t program_map_PID = read_field<uint16_t>(pat_entry_bs, PAT_table_data_program_map_PID_mask_bs);

            patt.data.push_back({ program_num, program_map_PID });
        }
    }

    void PacketParser::parse_PMT_table(PacketBuffer& p_buffer)
    {
        // Create the PMT table
        _packet.payload_data->table_header->table_syntax->table_data = PMT_Table{};

        // Create bitset from packet buffer
        auto td_bs = from_packet_buffer<PMT_table_data_header_size>(p_buffer);

        // Set fields
        TableHeader& th = *_packet.payload_data->table_header;
        TableSyntax& ts = *th.table_syntax;
        PMT_Table& pmtt = std::get<PMT_Table>(ts.table_data);

        if (not all_field_bits_set(td_bs, PMT_reserved_bits_mask_bs)) { throw InvalidReservedBits{}; }
        if (not all_field_bits_set(td_bs, PMT_reserved_bits_2_mask_bs)) { throw InvalidReservedBits{}; }
        if (not all_field_bits_unset(td_bs, PMT_program_info_length_unused_bits_mask_bs)) { throw InvalidUnusedBits{}; }

        pmtt.PCR_PID = read_field<uint16_t>(td_bs, PMT_reserved_bits_mask_bs);
        pmtt.program_info_length = read_field<uint16_t>(td_bs, PMT_program_info_length_bs);

        if (pmtt.program_info_length != 0) { throw Unimplemented{ "parsing of PTM program descriptors" }; }

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

    void PacketParser::parse_CRC32(PacketBuffer& p_buffer)
    {
        // Create bitset from packet buffer
        auto crc32_bs = from_packet_buffer<tss_crc32_size>(p_buffer);

        // Set fields
        TableSyntax& ts = *_packet.payload_data->table_header->table_syntax;

        ts.crc32 = read_field<uint32_t>(crc32_bs, tss_crc32_mask_bs);
    }

    void PacketParser::parse_elementary_stream_specific_data(PacketBuffer& p_buffer, uint16_t elementary_stream_specific_data_size)
    {
        // Create the ESSD info data
        TableHeader& th = *_packet.payload_data->table_header;
        TableSyntax& ts = *th.table_syntax;
        PMT_Table& pmtt = std::get<PMT_Table>(ts.table_data);
        pmtt.ESSD_info_data = std::vector<ESSD>{};

        while (elementary_stream_specific_data_size)
        {
            // Create bitset from packet buffer
            auto essd_bs = from_packet_buffer<ESSD_header_size>(p_buffer);

            // Set fields
            ESSD essd{};

            if (not all_field_bits_set(essd_bs, ESSD_reserved_bits_mask_bs)) { throw InvalidReservedBits{}; }
            if (not all_field_bits_set(essd_bs, ESSD_reserved_bits_2_mask_bs)) { throw InvalidReservedBits{}; }
            if (not all_field_bits_unset(essd_bs, ESSD_info_length_unused_bits_mask_bs)) { throw InvalidUnusedBits{}; }

            essd.stream_type = read_field<uint8_t>(essd_bs, ESSD_stream_type_mask_bs);
            essd.elementary_PID = read_field<uint16_t>(essd_bs, ESSD_elementary_PID_mask_bs);
            essd.info_length = read_field<uint16_t>(essd_bs, ESSD_info_length_mask_bs);

            if (essd.info_length != 0)
            {
                essd.descriptors = parse_descriptors(p_buffer, essd.info_length);
            }

            pmtt.ESSD_info_data->push_back(essd);

            elementary_stream_specific_data_size -= (ESSD_header_size + essd.info_length);
        }
    }

    std::vector<Descriptor> PacketParser::parse_descriptors(PacketBuffer& p_buffer, uint16_t descriptors_size)
    {
        std::vector<Descriptor> ret{};

        while (descriptors_size)
        {
            // Create bitset from packet buffer
            auto dsc_bs = from_packet_buffer<descriptor_header_size>(p_buffer);

            // Set fields
            Descriptor descriptor{};

            descriptor.tag = read_field<uint8_t>(dsc_bs, dsc_tag_mask_bs);
            descriptor.length = read_field<uint8_t>(dsc_bs, dsc_length_mask_bs);

            if (descriptor.length != 0)
            {
                descriptor.data = p_buffer.read(descriptor.length);
            }

            ret.push_back(descriptor);

            descriptors_size -= (descriptor_header_size + descriptor.length);
        }

        return ret;
    }
}
