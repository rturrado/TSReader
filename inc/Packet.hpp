#ifndef __TS_PACKET_HPP__
#define __TS_PACKET_HPP__

#include "ByteBufferView.hpp"

#include <cstdint>
#include <iostream>
#include <optional>
#include <variant>
#include <span>
#include <vector>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace TS
{
    // Sizes (in bytes)
    //
    // Packet
    constexpr uint8_t packet_size{ 188 };
    // Header
    constexpr uint8_t header_size{ 4 };
    // Adaptation field
    constexpr uint8_t af_length_size{ 1 };
    constexpr uint8_t af_flags_size{ 1 };
    // Adaptation field optional
    constexpr uint8_t afo_PCR_size{ 6 };
    constexpr uint8_t afo_OPCR_size{ 6 };
    constexpr uint8_t afo_splicing_countdown_size{ 1 };
    constexpr uint8_t afo_transport_private_data_length_size{ 1 };
    // Adaptation extension
    constexpr uint8_t adaptation_extension_header_size{ 2 };
    constexpr uint8_t ae_length_size{ 1 };
    constexpr uint8_t ae_flags_size{ 1 };
    // Adaptation extension optional
    constexpr uint8_t aeo_LTW_field_size{ 2 };
    constexpr uint8_t aeo_piecewise_field_size{ 3 };
    constexpr uint8_t aeo_seamless_field_size{ 5 };
    // Pointer
    constexpr uint8_t ptr_pointer_field_size{ 1 };
    // Table header
    constexpr uint8_t table_header_size{ 3 };
    constexpr uint8_t th_table_id_size{ 1 };
    constexpr uint16_t th_max_section_length{ 1021 };
    // Table syntax section
    constexpr uint16_t table_syntax_section_size{ 5 };
    constexpr uint16_t tss_table_id_extension_size{ 2 };
    constexpr uint8_t tss_reserved_version_current_next_indicator_size{ 1 };
    constexpr uint8_t tss_section_number_size{ 1 };
    constexpr uint8_t tss_last_section_number_size{ 1 };
    constexpr uint8_t tss_crc32_size{ 4 };
    // PAT table
    constexpr uint8_t PAT_table_data_program_size{ 4 };
    constexpr uint8_t PAT_table_data_program_num_size{ 2 };
    constexpr uint8_t PAT_table_data_program_map_PID_size{ 2 };
    // PMT table
    constexpr uint8_t PMT_table_data_header_size{ 4 };
    // ESSD
    constexpr uint8_t ESSD_header_size{ 5 };
    // Descriptors
    constexpr uint8_t descriptor_header_size{ 2 };



    // Constants
    //
    constexpr uint8_t sync_byte_valid_value{ 'G' };
    constexpr uint8_t stuffing_byte{ 0xff };
    // PIDs
    constexpr uint16_t PAT_PID{ 0 };
    constexpr uint16_t CAT_PID{ 1 };
    constexpr uint16_t default_NIT_PID{ 0x10 };
    // Table IDs
    constexpr uint8_t PAT_table_id{ 0 };
    constexpr uint8_t CAT_table_id{ 1 };
    constexpr uint8_t PMT_table_id{ 2 };
    // Program numbers
    constexpr uint16_t NIT_program_num{ 0 };
    // Tags
    constexpr uint8_t language_tag{ 0xa };



    // NIT PID accessors
    //
    class NIT_PID
    {
    public:
        NIT_PID(const NIT_PID&) = delete;
        NIT_PID(NIT_PID&) = delete;
        NIT_PID& operator=(const NIT_PID&) = delete;
        NIT_PID& operator=(NIT_PID&&) = delete;

        static NIT_PID& get_instance();

        uint16_t get_NIT_PID();
        void set_NIT_PID(uint16_t value);
    private:
        NIT_PID() {}

        uint16_t _value{ default_NIT_PID };
    };



    // Masks
    //
    // Header
    const std::array<uint8_t, 4> hdr_sync_byte_mask_array{ 0xff, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 4> hdr_transport_error_indicator_mask_array{ 0x00, 0x80, 0x00, 0x00 };
    const std::array<uint8_t, 4> hdr_payload_unit_start_indicator_mask_array{ 0x00, 0x40, 0x00, 0x00 };
    const std::array<uint8_t, 4> hdr_transport_priority_mask_array{ 0x00, 0x20, 0x00, 0x00 };
    const std::array<uint8_t, 4> hdr_PID_mask_array{ 0x00, 0x1f, 0xff, 0x00 };
    const std::array<uint8_t, 4> hdr_transport_scrambling_control_mask_array{ 0x00, 0x00, 0x00, 0xc0 };
    const std::array<uint8_t, 4> hdr_adaptation_field_control_mask_array{ 0x00, 0x00, 0x00, 0x30 };
    const std::array<uint8_t, 4> hdr_continuity_counter_mask_array{ 0x00, 0x00, 0x00, 0x0f };

    const boost::dynamic_bitset<uint8_t> hdr_sync_byte_mask_bs{
        cbegin(hdr_sync_byte_mask_array), cend(hdr_sync_byte_mask_array) };
    const boost::dynamic_bitset<uint8_t> hdr_transport_error_indicator_mask_bs{
        cbegin(hdr_transport_error_indicator_mask_array), cend(hdr_transport_error_indicator_mask_array) };
    const boost::dynamic_bitset<uint8_t> hdr_payload_unit_start_indicator_mask_bs{
        cbegin(hdr_payload_unit_start_indicator_mask_array), cend(hdr_payload_unit_start_indicator_mask_array) };
    const boost::dynamic_bitset<uint8_t> hdr_transport_priority_mask_bs{
        cbegin(hdr_transport_priority_mask_array), cend(hdr_transport_priority_mask_array) };
    const boost::dynamic_bitset<uint8_t> hdr_PID_mask_bs{
        cbegin(hdr_PID_mask_array), cend(hdr_PID_mask_array) };
    const boost::dynamic_bitset<uint8_t> hdr_transport_scrambling_control_mask_bs{
        cbegin(hdr_transport_scrambling_control_mask_array), cend(hdr_transport_scrambling_control_mask_array) };
    const boost::dynamic_bitset<uint8_t> hdr_adaptation_field_control_mask_bs{
        cbegin(hdr_adaptation_field_control_mask_array), cend(hdr_adaptation_field_control_mask_array) };
    const boost::dynamic_bitset<uint8_t> hdr_continuity_counter_mask_bs{
        cbegin(hdr_continuity_counter_mask_array), cend(hdr_continuity_counter_mask_array) };

    // Adaptation field
    const boost::dynamic_bitset<uint8_t> af_discontinuity_indicator_mask_bs{ 8, 0x80 };
    const boost::dynamic_bitset<uint8_t> af_random_access_indicator_mask_bs{ 8, 0x40 };
    const boost::dynamic_bitset<uint8_t> af_elementary_stream_priority_indicator_mask_bs{ 8, 0x20 };
    const boost::dynamic_bitset<uint8_t> af_PCR_flag_mask_bs{ 8, 0x10 };
    const boost::dynamic_bitset<uint8_t> af_OPCR_flag_mask_bs{ 8, 0x08 };
    const boost::dynamic_bitset<uint8_t> af_splicing_point_flag_mask_bs{ 8, 0x04 };
    const boost::dynamic_bitset<uint8_t> af_transport_private_data_flag_mask_bs{ 8, 0x02 };
    const boost::dynamic_bitset<uint8_t> af_extension_flag_mask_bs{ 8, 0x01 };

    // Adaptation extension
    const std::array<uint8_t, 2> ae_length_mask_array{ 0xff, 0x00};
    const std::array<uint8_t, 2> ae_legal_time_window_flag_mask_array{ 0x00, 0x80 };
    const std::array<uint8_t, 2> ae_piecewise_rate_flag_mask_array{ 0x00, 0x40 };
    const std::array<uint8_t, 2> ae_seamless_splice_flag_mask_array{ 0x00, 0x20 };
    const std::array<uint8_t, 2> ae_reserved_mask_array{ 0x00, 0x1f };

    const boost::dynamic_bitset<uint8_t> ae_length_mask_bs{
        cbegin(ae_length_mask_array), cend(ae_length_mask_array) };
    const boost::dynamic_bitset<uint8_t> ae_legal_time_window_flag_mask_bs{
        cbegin(ae_legal_time_window_flag_mask_array), cend(ae_legal_time_window_flag_mask_array) };
    const boost::dynamic_bitset<uint8_t> ae_piecewise_rate_flag_mask_bs{
        cbegin(ae_piecewise_rate_flag_mask_array), cend(ae_piecewise_rate_flag_mask_array) };
    const boost::dynamic_bitset<uint8_t> ae_seamless_splice_flag_mask_bs{
        cbegin(ae_seamless_splice_flag_mask_array), cend(ae_seamless_splice_flag_mask_array) };
    const boost::dynamic_bitset<uint8_t> ae_reserved_mask_bs{
        cbegin(ae_reserved_mask_array), cend(ae_reserved_mask_array) };

    // Adaptation extension optional
    const std::array<uint8_t, 2> aeo_legal_time_window_valid_flag_mask_array{ 0x80, 0x00 };
    const std::array<uint8_t, 2> aeo_legal_time_window_offset_mask_array{ 0x7f, 0xff };
    const std::array<uint8_t, 3> aeo_piecewise_rate_reserved_mask_array{ 0xc0, 0x00, 0x00 };
    const std::array<uint8_t, 3> aeo_piecewise_rate_mask_array{ 0x3f, 0xff, 0xff };
    const std::array<uint8_t, 5> aeo_seamless_splice_type_mask_array{ 0xf0, 0x00, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 5> aeo_DTS_next_access_unit_mask_array{ 0x0e, 0xff, 0xfe, 0xff, 0xfe };

    const boost::dynamic_bitset<uint8_t> aeo_legal_time_window_valid_flag_mask_bs{
        cbegin(aeo_legal_time_window_valid_flag_mask_array), cend(aeo_legal_time_window_valid_flag_mask_array) };
    const boost::dynamic_bitset<uint8_t> aeo_legal_time_window_offset_mask_bs{
        cbegin(aeo_legal_time_window_offset_mask_array), cend(aeo_legal_time_window_offset_mask_array) };
    const boost::dynamic_bitset<uint8_t> aeo_piecewise_rate_reserved_mask_bs{
        cbegin(aeo_piecewise_rate_reserved_mask_array), cend(aeo_piecewise_rate_reserved_mask_array) };
    const boost::dynamic_bitset<uint8_t> aeo_piecewise_rate_mask_bs{
        cbegin(aeo_piecewise_rate_mask_array), cend(aeo_piecewise_rate_mask_array) };
    const boost::dynamic_bitset<uint8_t> aeo_seamless_splice_type_mask_bs{
        cbegin(aeo_seamless_splice_type_mask_array), cend(aeo_seamless_splice_type_mask_array) };
    const boost::dynamic_bitset<uint8_t> aeo_DTS_next_access_unit_mask_bs{
        cbegin(aeo_DTS_next_access_unit_mask_array), cend(aeo_DTS_next_access_unit_mask_array) };

    // Table header
    const std::array<uint8_t, 3> th_table_id_mask_array{ 0xff, 0x00, 0x00 };
    const std::array<uint8_t, 3> th_section_syntax_indicator_mask_array{ 0x00, 0x80, 0x00 };
    const std::array<uint8_t, 3> th_private_bit_mask_array{ 0x00, 0x40, 0x00 };
    const std::array<uint8_t, 3> th_reserved_bits_mask_array{ 0x00, 0x30, 0x00 };
    const std::array<uint8_t, 3> th_section_length_unused_bits_mask_array{ 0x00, 0x0c, 0x00 };
    const std::array<uint8_t, 3> th_section_length_mask_array{ 0x00, 0x03, 0xff };

    const boost::dynamic_bitset<uint8_t> th_table_id_mask_bs{
        cbegin(th_table_id_mask_array), cend(th_table_id_mask_array) };
    const boost::dynamic_bitset<uint8_t> th_section_syntax_indicator_mask_bs{
        cbegin(th_section_syntax_indicator_mask_array), cend(th_section_syntax_indicator_mask_array) };
    const boost::dynamic_bitset<uint8_t> th_private_bit_mask_bs{
        cbegin(th_private_bit_mask_array), cend(th_private_bit_mask_array) };
    const boost::dynamic_bitset<uint8_t> th_reserved_bits_mask_bs{
        cbegin(th_reserved_bits_mask_array), cend(th_reserved_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> th_section_length_unused_bits_mask_bs{
        cbegin(th_section_length_unused_bits_mask_array), cend(th_section_length_unused_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> th_section_length_mask_bs{
        cbegin(th_section_length_mask_array), cend(th_section_length_mask_array) };

    // Table syntax section
    const std::array<uint8_t, 5> tss_table_id_extension_mask_array{ 0xff, 0xff, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 5> tss_reserved_bits_mask_array{ 0x00, 0x00, 0xc0, 0x00, 0x00 };
    const std::array<uint8_t, 5> tss_version_number_mask_array{ 0x00, 0x00, 0x3e, 0x00, 0x00 };
    const std::array<uint8_t, 5> tss_current_next_indicator_mask_array{ 0x00, 0x00, 0x01, 0x00, 0x00 };
    const std::array<uint8_t, 5> tss_section_number_mask_array{ 0x00, 0x00, 0x00, 0xff, 0x00 };
    const std::array<uint8_t, 5> tss_last_section_number_mask_array{ 0x00, 0x00, 0x00, 0x00, 0xff };
    const std::array<uint8_t, 4> tss_crc32_mask_array{ 0xff, 0xff, 0xff, 0xff };

    const boost::dynamic_bitset<uint8_t> tss_table_id_extension_mask_bs{
        cbegin(tss_table_id_extension_mask_array), cend(tss_table_id_extension_mask_array) };
    const boost::dynamic_bitset<uint8_t> tss_reserved_bits_mask_bs{
        cbegin(tss_reserved_bits_mask_array), cend(tss_reserved_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> tss_version_number_mask_bs{
        cbegin(tss_version_number_mask_array), cend(tss_version_number_mask_array) };
    const boost::dynamic_bitset<uint8_t> tss_current_next_indicator_mask_bs{
        cbegin(tss_current_next_indicator_mask_array), cend(tss_current_next_indicator_mask_array) };
    const boost::dynamic_bitset<uint8_t> tss_section_number_mask_bs{
        cbegin(tss_section_number_mask_array), cend(tss_section_number_mask_array) };
    const boost::dynamic_bitset<uint8_t> tss_last_section_number_mask_bs{
        cbegin(tss_last_section_number_mask_array), cend(tss_last_section_number_mask_array) };
    const boost::dynamic_bitset<uint8_t> tss_crc32_mask_bs{
        cbegin(tss_crc32_mask_array), cend(tss_crc32_mask_array) };

    // PAT table
    const std::array<uint8_t, 4> PAT_table_data_table_id_extension_mask_array{ 0xff, 0xff, 0x00, 0x00 };
    const std::array<uint8_t, 4> PAT_table_data_reserved_bits_mask_array{ 0x00, 0x00, 0xe0, 0x00 };
    const std::array<uint8_t, 4> PAT_table_data_program_map_PID_mask_array{ 0x00, 0x00, 0x1f, 0xff };

    const boost::dynamic_bitset<uint8_t> PAT_table_data_table_id_extension_mask_bs{
        cbegin(PAT_table_data_table_id_extension_mask_array), cend(PAT_table_data_table_id_extension_mask_array) };
    const boost::dynamic_bitset<uint8_t> PAT_table_data_reserved_bits_mask_bs{
        cbegin(PAT_table_data_reserved_bits_mask_array), cend(PAT_table_data_reserved_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> PAT_table_data_program_map_PID_mask_bs{
        cbegin(PAT_table_data_program_map_PID_mask_array), cend(PAT_table_data_program_map_PID_mask_array) };

    // PMT table
    const std::array<uint8_t, 4> PMT_reserved_bits_mask_array{ 0xe0, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 4> PMT_PCR_PID_mask_array{ 0x1f, 0xff, 0x00, 0x00 };
    const std::array<uint8_t, 4> PMT_reserved_bits_2_mask_array{ 0x00, 0x00, 0xf0, 0x00 };
    const std::array<uint8_t, 4> PMT_program_info_length_unused_bits_mask_array{ 0x00, 0x00, 0x0c, 0x00 };
    const std::array<uint8_t, 4> PMT_program_info_length_array{ 0x00, 0x00, 0x03, 0xff };

    const boost::dynamic_bitset<uint8_t> PMT_reserved_bits_mask_bs{
        cbegin(PMT_reserved_bits_mask_array), cend(PMT_reserved_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> PMT_PCR_PID_mask_bs{
        cbegin(PMT_PCR_PID_mask_array), cend(PMT_PCR_PID_mask_array) };
    const boost::dynamic_bitset<uint8_t> PMT_reserved_bits_2_mask_bs{
        cbegin(PMT_reserved_bits_2_mask_array), cend(PMT_reserved_bits_2_mask_array) };
    const boost::dynamic_bitset<uint8_t> PMT_program_info_length_unused_bits_mask_bs{
        cbegin(PMT_program_info_length_unused_bits_mask_array), cend(PMT_program_info_length_unused_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> PMT_program_info_length_bs{
        cbegin(PMT_program_info_length_array), cend(PMT_program_info_length_array) };

    // Elementary stream specific data
    const std::array<uint8_t, 5> ESSD_stream_type_mask_array{ 0xff, 0x00, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 5> ESSD_reserved_bits_mask_array{ 0x00, 0xe0, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 5> ESSD_elementary_PID_mask_array{ 0x00, 0x1f, 0xff, 0x00, 0x00 };
    const std::array<uint8_t, 5> ESSD_reserved_bits_2_mask_array{ 0x00, 0x00, 0x00, 0xf0, 0x00 };
    const std::array<uint8_t, 5> ESSD_info_length_unused_bits_mask_array{ 0x00, 0x00, 0x00, 0x0c, 0x00 };
    const std::array<uint8_t, 5> ESSD_info_length_mask_array{ 0x00, 0x00, 0x00, 0x03, 0xff };

    const boost::dynamic_bitset<uint8_t> ESSD_stream_type_mask_bs{
        cbegin(ESSD_stream_type_mask_array), cend(ESSD_stream_type_mask_array) };
    const boost::dynamic_bitset<uint8_t> ESSD_reserved_bits_mask_bs{
        cbegin(ESSD_reserved_bits_mask_array), cend(ESSD_reserved_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> ESSD_elementary_PID_mask_bs{
        cbegin(ESSD_elementary_PID_mask_array), cend(ESSD_elementary_PID_mask_array) };
    const boost::dynamic_bitset<uint8_t> ESSD_reserved_bits_2_mask_bs{
        cbegin(ESSD_reserved_bits_2_mask_array), cend(ESSD_reserved_bits_2_mask_array) };
    const boost::dynamic_bitset<uint8_t> ESSD_info_length_unused_bits_mask_bs{
        cbegin(ESSD_info_length_unused_bits_mask_array), cend(ESSD_info_length_unused_bits_mask_array) };
    const boost::dynamic_bitset<uint8_t> ESSD_info_length_mask_bs{
        cbegin(ESSD_info_length_mask_array), cend(ESSD_info_length_mask_array) };

    // Descriptors
    const std::array<uint8_t, 2> dsc_tag_mask_array{ 0xff, 0x00 };
    const std::array<uint8_t, 2> dsc_length_mask_array{ 0x00, 0xff };

    const boost::dynamic_bitset<uint8_t> dsc_tag_mask_bs{ cbegin(dsc_tag_mask_array), cend(dsc_tag_mask_array) };
    const boost::dynamic_bitset<uint8_t> dsc_length_mask_bs{ cbegin(dsc_length_mask_array), cend(dsc_length_mask_array) };



    // Structs
    //
    struct Descriptor
    {
        uint8_t tag{ 0 };
        uint8_t length{ 0 };
        std::optional<byte_buffer_view> data{};
    };

    struct ESSD
    {
        uint8_t stream_type{ 0 };
        uint16_t elementary_PID{ 0 };
        uint16_t info_length{ 0 };
        std::optional<std::vector<Descriptor>> descriptors{};
    };

    struct PMT_Table
    {
        uint16_t PCR_PID{ 0 };
        uint16_t program_info_length{ 0 };

        std::optional<std::vector<Descriptor>> program_descriptors{};
        std::optional<std::vector<ESSD>> ESSD_info_data{};
    };

    struct PAT_Table
    {
        using program_num = uint16_t;
        using program_map_PID = uint16_t;

        std::vector<std::pair<program_num, program_map_PID>> data{};
    };

    struct TableSyntax
    {
        uint16_t table_id_extension{ 0 };
        uint8_t version_number{ 0 };
        bool current_next_indicator{ false };
        uint8_t section_number{ 0 };
        uint8_t last_section_number{ 0 };
        std::variant<PAT_Table, PMT_Table> table_data{};
        uint32_t crc32{ 0 };
    };

    struct TableHeader
    {
        uint8_t table_id{ 0 };
        bool section_syntax_indicator{ false };
        bool private_bit{ false };
        uint16_t section_length{ 0 };
        std::optional<TableSyntax> table_syntax{};

        bool has_syntax_section() const;

        friend std::ostream& operator<<(std::ostream& os, const TableHeader& th);
    };

    struct Pointer
    {
        uint8_t pointer_field{ 0 };
        std::optional<byte_buffer_view> pointer_filler_bytes{};

        friend std::ostream& operator<<(std::ostream& os, const Pointer& ptr);
    };

    struct PayloadData
    {
        std::optional<Pointer> pointer{};
        std::optional<TableHeader> table_header{};
        std::optional<byte_buffer_view> PES_data{};

        bool has_PES_data() const;
        const byte_buffer_view get_PES_data() const;

        friend std::ostream& operator<<(std::ostream& os, const PayloadData& pd);
    };

    struct AdaptationExtension
    {
        uint8_t length;
        bool legal_time_window_flag{ false };
        bool piecewise_rate_flag{ false };
        bool seamless_splice_flag{ false };
        uint8_t reserved;
        std::optional<bool> legal_time_window_valid_flag;
        std::optional<uint16_t> legal_time_window_offset;
        std::optional<uint8_t> piecewise_rate_reserved;
        std::optional<uint32_t> piecewise_rate;
        std::optional<uint8_t> seamless_splice_type;
        std::optional<uint64_t> DTS_next_access_unit;

        friend std::ostream& operator<<(std::ostream& os, const AdaptationExtension& ae);
    };

    struct AdaptationFieldFlags
    {
        bool discontinuity_indicator{ false };
        bool random_access_indicator{ false };
        bool elementary_stream_priority_indicator{ false };
        bool PCR_flag{ false };
        bool OPCR_flag{ false };
        bool splicing_point_flag{ false };
        bool transport_private_data_flag{ false };
        bool extension_flag{ false };

        friend std::ostream& operator<<(std::ostream& os, const AdaptationFieldFlags& aff);
    };

    struct AdaptationFieldOptional
    {
        std::optional<boost::dynamic_bitset<uint8_t>> PCR{};
        std::optional<boost::dynamic_bitset<uint8_t>> OPCR{};
        std::optional<int8_t> splice_countdown{};  // two's complement signed
        std::optional<uint8_t> transport_private_data_length{};
        std::optional<byte_buffer_view> transport_private_data{};
        std::optional<AdaptationExtension> extension{};
        std::optional<byte_buffer_view> stuffing_bytes{};

        friend std::ostream& operator<<(std::ostream& os, const AdaptationFieldOptional& afo);
    };

    struct AdaptationField
    {
        uint8_t length{ 0 };
        std::optional<AdaptationFieldFlags> flags{};
        std::optional<AdaptationFieldOptional> optional{};

        friend std::ostream& operator<<(std::ostream& os, const AdaptationField& af);
    };

    struct Header
    {
        uint8_t sync_byte{ 0 };
        bool transport_error_indicator{ false };
        bool payload_unit_start_indicator{ false };
        bool transport_priority{ false };
        uint16_t PID{ 0 };
        uint8_t transport_scrambling_control{ 0 };
        uint8_t adaptation_field_control{ 0 };
        uint8_t continuity_counter{ 0 };

        friend std::ostream& operator<<(std::ostream& os, const Header& header);
    };

    struct Packet
    {
        Header header{};
        std::optional<AdaptationField> adaptation_field{};
        std::optional<PayloadData> payload_data{};

        bool get_payload_unit_start_indicator() const;
        uint16_t get_PID() const;
        uint8_t get_continuity_counter() const;

        bool has_adaptation_field() const;
        bool has_payload_data() const;

        bool payload_contains_PAT_table() const;
        bool payload_contains_CAT_table() const;
        bool payload_contains_NIT_table() const;
        bool payload_contains_PMT_table() const;
        bool payload_contains_PSI() const;

        friend std::ostream& operator<<(std::ostream& os, const Packet& packet);
    };
}

#endif
