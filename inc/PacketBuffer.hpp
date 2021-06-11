#ifndef __TS_PACKET_BUFFER_HPP__
#define __TS_PACKET_BUFFER_HPP__

#include "Packet.hpp"

#include <array>
#include <fstream>
#include <span>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace TS
{
    class PacketBuffer
    {
    public:
        char* data_as_char_pointer() { return reinterpret_cast<char*>(_buffer.data()); }

        const byte_buffer_view read(uint8_t n);

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
