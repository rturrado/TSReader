#ifndef __PSI_TABLES_H__
#define __PSI_TABLES_H__

#include <map>

namespace TS
{
    // Data structures to hold processed table information

    using PID = uint16_t;
    using program_number = uint16_t;
    using stream_type = uint8_t;

    using TPAT_map = std::map<PID, program_number>;  // PMT PID -> program number
    using TPMT_map = std::map<PID, stream_type>;  // PES PID -> stream type

    class PSI_Tables
    {
    public:
        PSI_Tables(const PSI_Tables&) = delete;
        PSI_Tables(PSI_Tables&) = delete;
        PSI_Tables& operator=(const PSI_Tables&) = delete;
        PSI_Tables& operator=(PSI_Tables&&) = delete;

        static PSI_Tables& get_instance();

        void set_PAT_program_number(PID p, program_number n);
        void set_PMT_stream_type(PID p, stream_type st);

        bool is_PMT_PID(PID p) const;
        bool is_PES_PID(PID p) const;
        stream_type get_stream_type(PID p) const;
    private:
        PSI_Tables() {}

        TPAT_map PAT_map{};
        TPMT_map PMT_map{};
    };
}

#endif
