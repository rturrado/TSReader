#ifndef __TS_PACKET_H__
#define __TS_PACKET_H__

#include <bitset>
#include <iostream>
#include <optional>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace TS
{
    // Constants
    constexpr uint8_t packet_size{ 188 };
    constexpr uint8_t header_size{ 4 };
    constexpr uint8_t sync_byte_valid_value{ 'G' };

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
    const boost::dynamic_bitset<uint8_t> discontinuity_indicator_mask{ 8, 0x80 };
    const boost::dynamic_bitset<uint8_t> random_access_indicator_mask{ 8, 0x40 };
    const boost::dynamic_bitset<uint8_t> elementary_stream_priority_indicator_mask{ 8, 0x20 };
    const boost::dynamic_bitset<uint8_t> PCR_flag_mask{ 8, 0x10 };
    const boost::dynamic_bitset<uint8_t> OPCR_flag_mask{ 8, 0x08 };
    const boost::dynamic_bitset<uint8_t> splicing_point_flag_mask{ 8, 0x04 };
    const boost::dynamic_bitset<uint8_t> transport_private_data_flag_mask{ 8, 0x02 };
    const boost::dynamic_bitset<uint8_t> extension_flag_mask{ 8, 0x01 };

    // Adaptation extension masks
    const boost::dynamic_bitset<uint8_t> legal_time_window_flag_mask{ 8, 0x80 };
    const boost::dynamic_bitset<uint8_t> piecewise_rate_flag_mask{ 8, 0x40 };
    const boost::dynamic_bitset<uint8_t> seamless_splice_flag_mask{ 8, 0x20 };
    const boost::dynamic_bitset<uint8_t> reserved_mask{ 8, 0x1f };
    const boost::dynamic_bitset<uint8_t> legal_time_window_valid_flag_mask{ 16, 0x80'00 };
    const boost::dynamic_bitset<uint8_t> legal_time_window_offset_mask{ 16, 0x7f'ff };
    const boost::dynamic_bitset<uint8_t> piecewise_rate_reserved_mask{ 24, 0xc0'00'00 };
    const boost::dynamic_bitset<uint8_t> piecewise_rate_mask{ 24, 0x3f'ff'ff };
    const std::array<uint8_t, 5> sstm_array{ 0xf0, 0x00, 0x00, 0x00, 0x00 };
    const std::array<uint8_t, 5> dnaum_array{ 0x0e, 0xff, 0xfe, 0xff, 0xfe };
    const boost::dynamic_bitset<uint8_t> seamless_splice_type_mask{ cbegin(sstm_array), cend(sstm_array) };
    const boost::dynamic_bitset<uint8_t> DTS_next_access_unit_mask{ cbegin(dnaum_array), cend(dnaum_array) };

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
    };

    struct AdaptationField
    {
        uint8_t length;
        bool discontinuity_indicator{ false };
        bool random_access_indicator{ false };
        bool elementary_stream_priority_indicator{ false };
        bool PCR_flag{ false };
        bool OPCR_flag{ false };
        bool splicing_point_flag{ false };
        bool transport_private_data_flag{ false };
        bool extension_flag{ false };
        std::optional<std::bitset<48>> program_clock_reference{};
        std::optional<std::bitset<48>> original_program_clock_reference{};
        std::optional<uint8_t> splice_countdown{};
        std::optional<uint8_t> transport_private_data_length{};
        std::optional<std::vector<char>> transport_private_data{};
        std::optional<AdaptationExtension> extension{};
        std::optional<std::vector<char>> stuffing_bytes{};
    };

    struct PayloadData
    {
        std::vector<char> data{};
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

        bool adaptation_field_present() { return adaptation_field_control == 2 || adaptation_field_control == 3; }
        bool payload_data_present() { return adaptation_field_control == 1 || adaptation_field_control == 3; }

        friend std::ostream& operator<<(std::ostream& os, const Header& header);
    };

    struct Packet
    {
        Header header{};
        std::optional<AdaptationField> adaptation_field{};
        std::optional<PayloadData> payload_data{};
    };
}

#endif
