#ifndef __TS_PACKET_BUFFER_H__
#define __TS_PACKET_BUFFER_H__

#include "Packet.hpp"

#include <array>
#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace TS
{
    class PacketBuffer
    {
    public:
        char* data() { return reinterpret_cast<char*>(_buffer.data()); }

        std::vector<uint8_t> read(uint8_t n);

        [[nodiscard]] constexpr uint8_t size() const { return packet_size; }
        [[nodiscard]] uint8_t size_not_read() const { return packet_size - _pos; }

        [[nodiscard]] uint8_t get_read_position() const { return _pos; }
        void reset_read_position() { _pos = 0; }

        friend std::ifstream& operator>>(std::ifstream& ifs, PacketBuffer& pb);

    private:
        std::array<uint8_t, packet_size> _buffer{ 0 };
        uint8_t _pos{ 0 };
    };
}

#endif
