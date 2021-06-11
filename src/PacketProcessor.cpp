#include "Exception.hpp"
#include "Packet.hpp"
#include "PacketProcessor.hpp"
#include "PES_Data.hpp"
#include "PSI_Tables.hpp"

#include <algorithm>

namespace TS
{
    void PacketProcessor::process(const Packet& packet)
    {
        if (packet.has_payload_data())
        {
            if (packet.payload_contains_PSI())
            {
                if (packet.payload_contains_PAT_table())
                {
                    // ****
                    // TODO: should we store table data in PAT_map if current/next indicator is true,
                    //       and store table data in a buffer if current/next indicator is false?
                    // ****
                    process_PAT_payload(packet);
                }
                else if (packet.payload_contains_PMT_table())
                {
                    process_PMT_payload(packet);
                }
            }
            else
            {
                process_PES_payload(packet);
            }
        }
    }

    void PacketProcessor::process_PAT_payload(const Packet& packet)
    {
        const TableSyntax& ts = *packet.payload_data->table_header->table_syntax;

        if (not PSI_Tables::get_instance().PAT_needs_update(ts.version_number))
        {
            return;
        }

        const PAT_Table& patt = std::get<PAT_Table>(ts.table_data);

        std::for_each(cbegin(patt.data), cend(patt.data), [](const auto& program) {
            auto program_num = program.first;
            auto program_map_PID = program.second;

            // Update PAT table
            PSI_Tables::get_instance().set_PAT_program_number(program_map_PID, program_num);

            // Update NIT PID if needed
            if (program_num == NIT_program_num)
            {
                NIT_PID::get_instance().set_NIT_PID(program_map_PID);
            }
        });
    }

    void PacketProcessor::process_PMT_payload(const Packet& packet)
    {
        const TableSyntax& ts = *packet.payload_data->table_header->table_syntax;
        auto program_num{ PSI_Tables::get_instance().get_PAT_program_number(packet.get_PID()) };

        if (not PSI_Tables::get_instance().PMT_needs_update(program_num, ts.version_number))
        {
            return;
        }

        // Update PMT table
        const PMT_Table& pmtt = std::get<PMT_Table>(ts.table_data);

        std::for_each(cbegin(*pmtt.ESSD_info_data), cend(*pmtt.ESSD_info_data), [&program_num](const auto& essd) {
            PSI_Tables::get_instance().set_PMT_stream_type(program_num, essd.elementary_PID, essd.stream_type);
            });
    }

    void PacketProcessor::process_PES_payload(const Packet& packet)
    {
        auto PES_PID = packet.get_PID();
        auto PES_data = packet.payload_data->get_PES_data();

        // Save PES data
        if (PSI_Tables::get_instance().is_PES_PID(PES_PID))
        {
            PES_Data::get_instance().set_PES_data(PES_PID, PES_data);
        }
        else
        {
            throw UnknownPES{};
        }
    }
}
