#ifndef __PACKET_PROCESSOR_H__
#define __PACKET_PROCESSOR_H__

#include "Packet.hpp"

namespace TS
{
    class PacketProcessor
    {
    public:
        void process(const Packet& packet);
        void process_PAT_payload(const Packet& packet);
        void process_PMT_payload(const Packet& packet);
        void process_PES_payload(const Packet& packet);
    private:
    };
}

#endif
