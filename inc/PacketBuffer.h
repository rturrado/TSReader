#ifndef __TS_PACKET_BUFFER_H__
#define __TS_PACKET_BUFFER_H__

#include "Packet.h"

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

        constexpr uint8_t size() { return packet_size; }
        uint8_t size_not_read() { return packet_size - _pos; }
        uint8_t get_read_position() { return _pos; }
        void reset_read_position() { _pos = 0; }

        template <bool BigEndian = false>
        std::vector<uint8_t> read(uint8_t n);

        friend std::ifstream& operator>>(std::ifstream& ifs, PacketBuffer& pb);

    private:
        std::array<uint8_t, packet_size> _buffer{ 0 };
        uint8_t _pos{ 0 };
    };



    template <bool BigEndian>
    std::vector<uint8_t> PacketBuffer::read(uint8_t n)
    {
        if (_pos + n > size())
        {
            throw PacketBufferOverrun(n, size() - _pos);
        }

        std::vector<uint8_t> ret{ n * 8u };

        if constexpr (BigEndian)
        {
            auto cbegin_it{ crend(_buffer) - _pos - n };
            auto cend_it{ crend(_buffer) - _pos };
            ret = std::vector<uint8_t>{ cbegin_it, cend_it };
            // ****
            // TODO: fix? this is a reverse vector; but we should apply endianness to word level?
            // ****
        }

        else
        {
            auto cbegin_it{ cbegin(_buffer) + _pos };
            auto cend_it{ cbegin(_buffer) + _pos + n };
            ret = std::vector<uint8_t>{ cbegin_it, cend_it };
        }
        
        _pos += n;

        return ret;
    }
    // ****
    // TODO: should we treat endiannes at this point or at the caller point?
    // ****
}

#endif
