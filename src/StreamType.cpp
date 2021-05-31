#include "StreamType.h"

namespace TS
{
    TStreamType_map stream_type_map{
        {0xf, "ISO/IEC 13818-7 ADTS AAC (MPEG-2 lower bit-rate audio)"},
        {0x1b, "ITU-T Rec. H.264 and ISO/IEC 14496-10 (lower bit-rate video)"}
    };

    TStreamType_map& get_stream_type_map()
    {
        return stream_type_map;
    }

    bool is_valid_stream_type(uint8_t stream_type)
    {
        return stream_type_map.contains(stream_type);
    }

    std::string get_stream_name(uint8_t stream_type)
    {
        return (stream_type_map.contains(stream_type) ? stream_type_map[stream_type] : "unknown");
    }
}
