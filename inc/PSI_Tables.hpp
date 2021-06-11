#ifndef __TS_PSI_TABLES_HPP__
#define __TS_PSI_TABLES_HPP__

#include <map>

namespace TS
{
    // Data structures to hold processed table information

    using PID = uint16_t;
    using program_number = uint16_t;
    using stream_type = uint8_t;

    using TPAT_map = std::map<PID, program_number>;  // PMT PID -> program number
    using TPMT_map = std::map<PID, stream_type>;  // PES PID -> stream type (PMT level)
    using TPES_stream_type_cache_map = std::map<PID, stream_type>;  // PES PID -> stream type (TS file level)


    class PSI_Tables
    {
    private:
        class PSI_Table
        {
        public:
            bool needs_update(uint8_t version);
        private:
            bool _initialized{ false };
            uint8_t _version{ 0 };
        };

        class PAT_Table : public PSI_Table
        {
        public:
            bool is_PMT_PID(PID p) const { return PAT_map.contains(p); }
            [[nodiscard]] bool contains(PID p) const { return PAT_map.contains(p); }
            program_number& operator[](PID p) { return PAT_map[p]; }
            program_number at(PID p) const { return PAT_map.at(p); }
        private:
            TPAT_map PAT_map{};
        };

        class PMT_Table : public PSI_Table
        {
        public:
            [[nodiscard]] bool contains(PID p) const { return PMT_map.contains(p); }
            stream_type& operator[](PID p) { return PMT_map[p]; }
            stream_type at(PID p) const { return PMT_map.at(p); }
        private:
            TPMT_map PMT_map{};
        };

        // PES stream types cache
        // Considering PIDs are unique across a TS file,
        // this PES PID -> stream type map spares us from searching through all the PMT tables
        class PES_Stream_Type_Cache
        {
        public:
            bool is_PES_PID(PID p) const { return PES_stream_type_cache_map.contains(p); }
            [[nodiscard]] bool contains(PID p) const { return PES_stream_type_cache_map.contains(p); }
            stream_type& operator[](PID p) { return PES_stream_type_cache_map[p]; }
            stream_type at(PID p) const { return PES_stream_type_cache_map.at(p); }
        private:
            TPES_stream_type_cache_map PES_stream_type_cache_map{};
        };
    
    public:
        PSI_Tables(const PSI_Tables&) = delete;
        PSI_Tables(PSI_Tables&) = delete;
        PSI_Tables& operator=(const PSI_Tables&) = delete;
        PSI_Tables& operator=(PSI_Tables&&) = delete;

        static PSI_Tables& get_instance();

        program_number get_PAT_program_number(PID p) const;
        stream_type get_PES_stream_type(PID p) const;

        void set_PAT_program_number(PID p, program_number n);
        void set_PMT_stream_type(program_number n, PID p, stream_type st);

        [[nodiscard]] bool PAT_needs_update(uint8_t version) { return PAT_table.needs_update(version); }
        [[nodiscard]] bool PMT_needs_update(program_number n, uint8_t version) { return PMT_tables[n].needs_update(version); }

        bool is_PMT_PID(PID p) const;
        bool is_PES_PID(PID p) const;
    
    private:
        PSI_Tables() {}

        PAT_Table PAT_table{};
        std::map<program_number, PMT_Table> PMT_tables{};
        PES_Stream_Type_Cache PES_stream_type_cache_table{};
    };
}

#endif
