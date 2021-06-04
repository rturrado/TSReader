#include "Exception.h"
#include "Packet.h"
#include "PacketProcessor.h"
#include "PES_Data.h"
#include "PSI_Tables.h"

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
                    // TODO: should we consider version number to update or not PAT_map?
                    //       Should we store table data in PAT_map if current/next indicator is true,
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
        const PMT_Table& pmtt = std::get<PMT_Table>(ts.table_data);

        // Update PMT table
        std::for_each(cbegin(*pmtt.ESSD_info_data), cend(*pmtt.ESSD_info_data), [](const auto& essd) {
            PSI_Tables::get_instance().set_PMT_stream_type(essd.elementary_PID, essd.stream_type);
            });
    }

    void PacketProcessor::process_PES_payload(const Packet& packet)
    {
        auto PES_PID = packet.get_PID();
        auto PES_data = packet.payload_data->get_PES_data();

        // ****
        // TODO: if we always process the packet data before reading the next packet buffer,
        //       we may not need to do a single copy
        //       (packet buffer to packet's payload data's PES data and then from there to PES data map)
        //       We could just pass references (or iterators)
        // ****
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
