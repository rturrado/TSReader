#ifndef __TS_PACKET_BUFFER_H__
#define __TS_PACKET_BUFFER_H__

#include "Packet.h"

#include <array>
#include <fstream>
#include <iostream>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace TS
{
    class PacketBuffer
    {
    public:
        char* data() { return reinterpret_cast<char*>(_buffer.data()); }

        constexpr uint8_t size() { return packet_size; }

        boost::dynamic_bitset<uint8_t> read(uint8_t n, bool big_endian = false);

        void reset_read_position();

        friend std::ifstream& operator>>(std::ifstream& ifs, PacketBuffer& pb);

    private:
        std::array<uint8_t, packet_size> _buffer;
        uint8_t _pos{ 0 };
    };
}

#endif
