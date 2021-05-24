#ifndef __TS_PACKET_H__
#define __TS_PACKET_H__

#include <bitset>
#include <iostream>
#include <optional>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace TS
{
    // Sizes (in bytes)
    constexpr uint8_t packet_size{ 188 };
    constexpr uint8_t header_size{ 4 };
    constexpr uint8_t adaptation_field_length_size{ 1 };
    constexpr uint8_t adaptation_field_flags_size{ 1 };
    constexpr uint8_t PCR_size{ 6 };
    constexpr uint8_t OPCR_size{ 6 };
    constexpr uint8_t splicing_countdown_size{ 1 };
    constexpr uint8_t transport_private_data_length_size{ 1 };
    constexpr uint8_t adaptation_extension_length_size{ 1 };
    constexpr uint8_t adaptation_extension_flags_size{ 1 };
    constexpr uint8_t adaptation_extension_LTW_field_size{ 2 };
    constexpr uint8_t adaptation_extension_piecewise_field_size{ 3 };
    constexpr uint8_t adaptation_extension_seamless_field_size{ 5 };
    constexpr uint8_t pointer_field_size{ 1 };
    constexpr uint8_t table_header_size{ 3 };
    constexpr uint8_t table_id_size{ 1 };

    // Constants
    constexpr uint8_t sync_byte_valid_value{ 'G' };
    constexpr uint8_t stuffing_byte{ 0xff };
    constexpr uint16_t PAT_PID{ 0 };
    constexpr uint16_t CAT_PID{ 1 };
    constexpr uint16_t default_NIT_PID{ 0x10 };
    constexpr uint8_t PAT_table_id{ 0 };
    constexpr uint8_t CAT_table_id{ 1 };
    constexpr uint8_t PMT_table_id{ 2 };

    // NIT PID accessors
    uint16_t get_NIT_PID();
    void set_NIT_PID(uint16_t value);

    // Header masks
    const boost::dynamic_bitset<uint8_t> sync_byte_mask_bs{ 32, 0xff'00'00'00 };
    const boost::dynamic_bitset<uint8_t> transport_error_indicator_mask_bs{ 32, 0x00'80'00'00 };
    const boost::dynamic_bitset<uint8_t> payload_unit_start_indicator_mask_bs{ 32, 0x00'40'00'00 };
    const boost::dynamic_bitset<uint8_t> transport_priority_mask_bs{ 32, 0x00'20'00'00 };
    const boost::dynamic_bitset<uint8_t> PID_mask_bs{ 32, 0x00'00'1f'ff'00 };
    const boost::dynamic_bitset<uint8_t> transport_scrambling_control_mask_bs{ 32, 0x00'00'00'c0 };
    const boost::dynamic_bitset<uint8_t> adaptation_field_control_mask_bs{ 32, 0x00'00'00'30 };
    const boost::dynamic_bitset<uint8_t> continuity_counter_mask_bs{ 32, 0x00'00'00'0f };

    // Adaptation field masks
    const boost::dynamic_bitset<uint8_t> discontinuity_indicator_mask_bs{ 8, 0x80 };
    const boost::dynamic_bitset<uint8_t> random_access_indicator_mask_bs{ 8, 0x40 };
    const boost::dynamic_bitset<uint8_t> elementary_stream_priority_indicator_mask_bs{ 8, 0x20 };
    const boost::dynamic_bitset<uint8_t> PCR_flag_mask_bs{ 8, 0x10 };
    const boost::dynamic_bitset<uint8_t> OPCR_flag_mask_bs{ 8, 0x08 };
    const boost::dynamic_bitset<uint8_t> splicing_point_flag_mask_bs{ 8, 0x04 };
    const boost::dynamic_bitset<uint8_t> transport_private_data_flag_mask_bs{ 8, 0x02 };
    const boost::dynamic_bitset<uint8_t> extension_flag_mask_bs{ 8, 0x01 };

    // Adaptation extension masks
    const boost::dynamic_bitset<uint8_t> legal_time_window_flag_mask_bs{ 8, 0x80 };
    const boost::dynamic_bitset<uint8_t> piecewise_rate_flag_mask_bs{ 8, 0x40 };
    const boost::dynamic_bitset<uint8_t> seamless_splice_flag_mask_bs{ 8, 0x20 };
    const boost::dynamic_bitset<uint8_t> reserved_mask_bs{ 8, 0x1f };
    const boost::dynamic_bitset<uint8_t> legal_time_window_valid_flag_mask_bs{ 16, 0x80'00 };
    const boost::dynamic_bitset<uint8_t> legal_time_window_offset_mask_bs{ 16, 0x7f'ff };
    const boost::dynamic_bitset<uint8_t> piecewise_rate_reserved_mask_bs{ 24, 0xc0'00'00 };
    const boost::dynamic_bitset<uint8_t> piecewise_rate_mask_bs{ 24, 0x3f'ff'ff };
    const std::array<uint8_t, 5> sstm_array{ 0xf0, 0x00, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 5> dnaum_array{ 0x0e, 0xff, 0xfe, 0xff, 0xfe };
    const boost::dynamic_bitset<uint8_t> seamless_splice_type_mask_bs{ cbegin(sstm_array), cend(sstm_array) };
    const boost::dynamic_bitset<uint8_t> DTS_next_access_unit_mask_bs{ cbegin(dnaum_array), cend(dnaum_array) };

    // Table header masks
    const boost::dynamic_bitset<uint8_t> section_syntax_indicator_mask_bs{ 16, 0x80'00 };
    const boost::dynamic_bitset<uint8_t> private_bit_mask_bs{ 16, 0x40'00 };
    const boost::dynamic_bitset<uint8_t> reserved_bits_mask_bs{ 16, 0x30'00 };
    const boost::dynamic_bitset<uint8_t> section_length_unused_bits_mask_bs{ 16, 0x0c'00 };
    const boost::dynamic_bitset<uint8_t> section_length_mask_bs{ 16, 0x03'ff };

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
        std::optional<std::vector<uint8_t>> transport_private_data{};
        std::optional<AdaptationExtension> extension{};
        std::optional<std::vector<uint8_t>> stuffing_bytes{};

        friend std::ostream& operator<<(std::ostream& os, const AdaptationFieldOptional& afo);
    };

    struct AdaptationField
    {
        uint8_t length{ 0 };
        std::optional<AdaptationFieldFlags> flags{};
        std::optional<AdaptationFieldOptional> optional{};

        friend std::ostream& operator<<(std::ostream& os, const AdaptationField& af);
    };

    struct Pointer
    {
        uint8_t pointer_field{ 0 };
        std::optional<std::vector<uint8_t>> pointer_filler_bytes{};

        friend std::ostream& operator<<(std::ostream& os, const Pointer& ptr);
    };

    struct TableHeader
    {
        uint8_t table_id{ 0 };
        bool section_syntax_indicator{ false };
        bool private_bit{ false };
        uint8_t reserved_bits{ 0 };
        uint8_t section_length_unused_bits{ 0 };
        uint16_t section_length{ 0 };

        friend std::ostream& operator<<(std::ostream& os, const TableHeader& th);
    };

    struct PayloadData
    {
        std::optional<Pointer> pointer{};
        std::optional<TableHeader> table_header{};

        friend std::ostream& operator<<(std::ostream& os, const PayloadData& pd);
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

        uint16_t get_PID() const { return header.PID; }
        bool adaptation_field_present() const { return header.adaptation_field_control == 2 || header.adaptation_field_control == 3; }
        bool payload_data_present() const { return header.adaptation_field_control == 1 || header.adaptation_field_control == 3; }
        bool pointer_present() const { return header.payload_unit_start_indicator; }
        bool payload_contains_PAT_table() const { return header.PID == PAT_PID; }
        bool payload_contains_CAT_table() const { return header.PID == CAT_PID; }
        bool payload_contains_NIT_table() const { return header.PID == get_NIT_PID(); }

        friend std::ostream& operator<<(std::ostream& os, const Packet& packet);
    };
}

#endif
