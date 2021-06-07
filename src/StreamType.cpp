#include "StreamType.hpp"

#include <memory>

namespace TS
{
    /* static */
    StreamTypeMap& StreamTypeMap::get_instance()
    {
        static StreamTypeMap instance;
        return instance;
    }

    bool StreamTypeMap::is_valid_stream_type(stream_type t) const
    {
        return _type_to_description_map.contains(t);
    }

    std::string StreamTypeMap::get_stream_description(stream_type t) const
    {
        return (_type_to_description_map.contains(t) ? _type_to_description_map.at(t) : "unknown");
    }
    
    file_extension StreamTypeMap::get_file_extension(stream_type t) const
    {
        return (_type_to_file_extension_map.contains(t) ? _type_to_file_extension_map.at(t) : "unknown");
    }
}
