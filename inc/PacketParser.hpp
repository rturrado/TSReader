#ifndef __TS_PACKET_PARSER_H__
#define __TS_PACKET_PARSER_H__

#include "Packet.hpp"
#include "PacketBuffer.hpp"

#include <vector>

namespace TS
{
    class PacketParser
    {
    public:
        void parse(PacketBuffer& buffer);
        Packet& get_packet() { return _packet; }
        size_t get_packet_index() { return _packet_index; }

    private:
        static size_t _packet_index;

        void parse_header(PacketBuffer& buffer);
        void parse_adaptation_field(PacketBuffer& buffer);
        void parse_adaptation_field_flags(PacketBuffer& buffer);
        void parse_adaptation_field_optional(PacketBuffer& buffer);
        void parse_adaptation_extension(PacketBuffer& p_buffer);
        void parse_payload_data(PacketBuffer& buffer);
        void parse_payload_data_as_PES(PacketBuffer& p_buffer);
        void parse_payload_data_as_PSI(PacketBuffer& p_buffer);
        void parse_pointer(PacketBuffer& buffer);
        void parse_table_header(PacketBuffer& buffer);
        bool check_and_parse_stuffing_bytes_section(const std::vector<uint8_t>& buffer) const;
        bool check_and_parse_stuffing_bytes_section(uint8_t first_byte, uint8_t bytes_to_read, PacketBuffer& p_buffer) const;
        void parse_table_syntax_section(PacketBuffer& p_buffer);
        void parse_PAT_table(PacketBuffer& p_buffer);
        void parse_PMT_table(PacketBuffer& p_buffer);
        void parse_CRC32(PacketBuffer& p_buffer);
        void parse_elementary_stream_specific_data(PacketBuffer& p_buffer, uint16_t elementary_stream_specific_data_size);
        std::vector<Descriptor> parse_descriptors(PacketBuffer& p_buffer, uint16_t descriptors_size);

        Packet _packet{};
    };
}

#endif
