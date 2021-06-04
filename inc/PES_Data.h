#ifndef __PES_DATA_H__
#define __PES_DATA_H__

#include <functional>
#include <map>
#include <vector>

namespace TS
{
    // Data structures to hold processed PES data

    using PID = uint16_t;
    using TPES_data = std::vector<uint8_t>;

    using TPES_map = std::map<PID, TPES_data>;  // PES PID -> PES data

    class PES_Data
    {
    public:
        PES_Data() = default;
        PES_Data(const PES_Data&) = delete;
        PES_Data(PES_Data&) = delete;
        PES_Data& operator=(const PES_Data&) = delete;
        PES_Data& operator=(PES_Data&&) = delete;

        static PES_Data& get_instance();

        bool has_PES_data(PID p) const;
        TPES_data get_PES_data(PID p) const;
        void set_PES_data(PID p, TPES_data& data);
    private:
        TPES_map PES_map;
    };
}

#endif
