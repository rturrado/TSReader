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
        size_t get_packet_index() { return _index; }
    private:
        void parse_header(PacketBuffer& buffer);
        void parse_adaptation_field(PacketBuffer& buffer);
        void parse_adaptation_field_flags(PacketBuffer& buffer);
        void parse_adaptation_field_optional(PacketBuffer& buffer);
        void parse_adaptation_extension(std::vector<uint8_t> af_buffer, auto cbegin_it, auto cend_it);
        void parse_payload_data(PacketBuffer& buffer);

        Packet _packet{};
        size_t _index{ 0 };
    };
}

#endif
