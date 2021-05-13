#ifndef __TS_PACKET_PARSER_H__
#define __TS_PACKET_PARSER_H__

#include "Packet.h"
#include "PacketBuffer.h"

#include <set>

namespace TS
{
    class PacketParser
    {
    public:
        static std::multiset<uint16_t> pids;

        void parse(PacketBuffer& buffer);
    private:
        void parse_header(PacketBuffer& buffer);
        void parse_adaptation_field(PacketBuffer& buffer);
        void parse_payload_data(PacketBuffer& buffer);

        Packet _packet{};
    };
}

#endif
