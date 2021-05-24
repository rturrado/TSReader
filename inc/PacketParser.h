#ifndef __TS_PACKET_PARSER_H__
#define __TS_PACKET_PARSER_H__

#include "Packet.h"
#include "PacketBuffer.h"

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
        void parse_adaptation_extension(std::vector<uint8_t>& af_buffer);
        void parse_payload_data(PacketBuffer& buffer);
        void parse_pointer(PacketBuffer& buffer);
        void parse_table_header(PacketBuffer& buffer);
        void parse_PAT_table(PacketBuffer& p_buffer);
        void parse_PMT_table(PacketBuffer& p_buffer);
        void parse_table_syntax(PacketBuffer& p_buffer);
        void parse_elementary_stream_specific_data(PacketBuffer& p_buffer);

        Packet _packet{};
    };
}

#endif
