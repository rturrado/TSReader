#include "Packet.h"

#include <iostream>

namespace TS
{
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
        if (packet.adaptation_field_present()) { os << ", " << *packet.adaptation_field; }
        if (packet.payload_data_present()) { os << ", " << *packet.payload_data; }
        os << "]";
        return os;
    }
}
