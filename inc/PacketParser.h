#ifndef __TS_PACKET_PARSER_H__
#define __TS_PACKET_PARSER_H__

#include "Packet.h"
#include "PacketBuffer.h"

namespace TS
{
    class PacketParser
    {
    public:
        void parse(PacketBuffer& buffer);
        Packet& get_packet() { return _packet; }
    private:
        void parse_header(PacketBuffer& buffer);
        void parse_adaptation_field(PacketBuffer& buffer);
        void parse_payload_data(PacketBuffer& buffer);

        Packet _packet{};
    };
}

#endif
