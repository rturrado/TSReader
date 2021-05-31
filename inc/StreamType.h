#ifndef __STREAM_TYPES_H__
#define __STREAM_TYPES_H__

#include <map>
#include <string>

namespace TS
{
    using stream_name = std::string;
    using stream_type = uint8_t;

    using TStreamType_map = std::map<stream_type, stream_name>;  // stream type -> stream name

    TStreamType_map& get_stream_type_map();
    bool is_valid_stream_type(uint8_t stream_type);
    std::string get_stream_name(uint8_t stream_type);
}

#endif
