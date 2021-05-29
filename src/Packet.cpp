#include "Packet.h"
#include "Tables.h"

#include <iostream>

namespace TS
{
    // ****
    // TODO: DO NOT use global variables?
    // ****

    // NIT PID is set to 0x10 by default
    // However, if PAT table associates program 0 with a different PID, we can update the NIT PID to that new value

    uint16_t NIT_PID{ default_NIT_PID };

    uint16_t get_NIT_PID()
    {
        return NIT_PID;
    }

    void set_NIT_PID(uint16_t value)
    {
        NIT_PID = value;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const AdaptationExtension& ae)
    {
        os << "extension=<L=" << static_cast<uint16_t>(ae.length) << ">";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const AdaptationFieldFlags& aff)
    {
        os << "flags=("
            << "DI=" << aff.discontinuity_indicator << ", "
            << "RAI=" << aff.random_access_indicator << ", "
            << "ESPI=" << aff.elementary_stream_priority_indicator
            << ")";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const AdaptationFieldOptional& afo)
    {
        bool first{ true };
        os << "optional=(";
        if (afo.PCR) { os << "PCR=<data>"; first = false; }
        if (afo.OPCR) { os << (first ? "" : ", ") << "OPCR=<data>"; first = false; }
        if (afo.splice_countdown)
        {
            os << (first ? "" : ", ") << "SC=" << static_cast<int16_t>(*afo.splice_countdown);
            first = false;
        }
        if (afo.transport_private_data_length)
        {
            os << "TPDL=" << (first ? "" : ", ") << static_cast<uint16_t>(*afo.transport_private_data_length);
            os << "TPD=<data>";
            first = false;
        }
        if (afo.extension)
        {
            os << (first ? "" : ", ") << *afo.extension;
            first = false;
        }
        if (afo.stuffing_bytes)
        {
            os << (first ? "" : ", ") << "SB=<data>";
        }
        os << ")";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const AdaptationField& af)
    {
        os << "af={L=" << static_cast<uint16_t>(af.length);
        if (af.flags) { os << ", " << *af.flags; }
        if (af.optional) { os << (af.flags ? ", " : "") << *af.optional; }
        os << "}";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const Pointer& ptr)
    {
        os << "ptr=(PF=" << static_cast<uint16_t>(ptr.pointer_field) << ")";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const TableHeader& th)
    {
        os << "th=("
            << "ID=" << static_cast<uint16_t>(th.table_id) << ", "
            << "SSI=" << th.section_syntax_indicator << ", "
            << "SL=" << th.section_length
            << ")";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const PayloadData& pd)
    {
        os << "payload={";
        if (!pd.pointer and !pd.table_header) { os << "<data>"; }
        else
        {
            if (pd.pointer) { os << *pd.pointer; }
            if (pd.table_header) { os << (pd.pointer ? ", " : "") << *pd.table_header; }
        }
        os << "}";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const Header& header)
    {
        os << "header={"
            << "TEI=" << header.transport_error_indicator << ", "
            << "PUSI=" << header.payload_unit_start_indicator << ", "
            << "TP=" << header.transport_priority << ", "
            << "PID=0x" << std::hex << header.PID << ", "
            << "TSC=0x" << std::hex << static_cast<uint16_t>(header.transport_scrambling_control) << ", "
            << "AFC=0x" << std::hex << static_cast<uint16_t>(header.adaptation_field_control) << ", "
            << "CC=" << std::dec << static_cast<uint16_t>(header.continuity_counter)
            << "}";
        return os;
    }

    /* friend */
    std::ostream& operator<<(std::ostream& os, const Packet& packet)
    {
        os << "packet=[" << packet.header;
        if (packet.has_adaptation_field()) { os << ", " << *packet.adaptation_field; }
        if (packet.has_payload_data()) { os << ", " << *packet.payload_data; }
        os << "]";
        return os;
    }



    // TableHeader

    bool TableHeader::has_syntax_section() const
    {
        return section_length != 0;
    }



    // Packet

    bool Packet::get_payload_unit_start_indicator() const
    {
        return header.payload_unit_start_indicator;
    }
    uint16_t Packet::get_PID() const
    {
        return header.PID;
    }
    uint8_t Packet::get_continuity_counter() const
    {
        return header.continuity_counter;
    }

    bool Packet::has_adaptation_field() const
    {
        return header.adaptation_field_control == 2 || header.adaptation_field_control == 3;
    }
    bool Packet::has_payload_data() const
    {
        return header.adaptation_field_control == 1 || header.adaptation_field_control == 3;
    }

    bool Packet::payload_contains_PAT_table() const { return header.PID == PAT_PID; }
    bool Packet::payload_contains_CAT_table() const { return header.PID == CAT_PID; }
    bool Packet::payload_contains_NIT_table() const { return header.PID == get_NIT_PID(); }
    bool Packet::payload_contains_PMT_table() const { return get_PAT_map().contains(header.PID); }
    bool Packet::payload_contains_PSI() const
    {
        return payload_contains_PAT_table()
            || payload_contains_CAT_table()
            || payload_contains_NIT_table()
            || payload_contains_PMT_table();
    }
}
