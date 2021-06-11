#ifndef __TS_STREAM_TYPES_HPP__
#define __TS_STREAM_TYPES_HPP__

#include <map>
#include <string>

namespace TS
{
    using stream_type = uint8_t;
    using stream_description = std::string;
    using file_extension = std::string;

    class StreamTypeMap
    {
        using type_to_description_map_type = std::map<stream_type, stream_description>;  // stream type -> stream description
        using type_to_file_extension_map_type = std::map<stream_type, file_extension>;  // stream type -> file extension

    public:
        StreamTypeMap(const StreamTypeMap&) = delete;
        StreamTypeMap(StreamTypeMap&&) = delete;
        StreamTypeMap& operator=(const StreamTypeMap&) = delete;
        StreamTypeMap& operator=(StreamTypeMap&&) = delete;

        static StreamTypeMap& get_instance();
        bool is_valid_stream_type(stream_type t) const;
        stream_description get_stream_description(stream_type t) const;
        file_extension get_file_extension(stream_type t) const;

    private:
        StreamTypeMap() {}

        type_to_description_map_type _type_to_description_map{
            {0xf, "ISO/IEC 13818-7 ADTS AAC (MPEG-2 lower bit-rate audio)"},
            {0x1b, "ITU-T Rec. H.264 and ISO/IEC 14496-10 (lower bit-rate video)"}
        };

        type_to_file_extension_map_type _type_to_file_extension_map{
            {0xf, "aac"},
            {0x1b, "mp4"}
        };
    };
}

#endif
