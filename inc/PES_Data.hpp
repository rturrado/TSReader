#ifndef __TS_PES_DATA_HPP__
#define __TS_PES_DATA_HPP__

#include "ByteBufferView.hpp"

#include <functional>
#include <map>
#include <span>

namespace TS
{
    // Data structures to hold processed PES data

    using PID = uint16_t;

    using TPES_map = std::map<PID, byte_buffer_view>;  // PES PID -> PES data

    class PES_Data
    {
    public:
        PES_Data(const PES_Data&) = delete;
        PES_Data(PES_Data&) = delete;
        PES_Data& operator=(const PES_Data&) = delete;
        PES_Data& operator=(PES_Data&&) = delete;

        static PES_Data& get_instance();

        bool has_PES_data(PID p) const;
        const byte_buffer_view get_PES_data(PID p) const;
        void set_PES_data(PID p, const byte_buffer_view& data);
    private:
        PES_Data() {}

        TPES_map PES_map;
    };
}

#endif
