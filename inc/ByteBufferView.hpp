#ifndef __TS_BYTE_BUFFER_VIEW_HPP__
#define __TS_BYTE_BUFFER_VIEW_HPP__

#include <span>

namespace TS
{
    // Byte buffer views are spans of the byte array containing the packet data
    // They allow us to parse and process a packet without creating copies of sections of the byte array
    // 
    // At the moment, packets are processed and post-processed before reading the next one,
    // payloads being whether discarded or written out to files as part of that post-processing
    // That is, payloads or tables spanning across consecutive packets are unimplemented
    // In case we couldn't process a packet before reading the next one (e.g. for correctly dealing with the
    // current/next indicator set to zero), we would need to store the packet data temporarily
    //
    using byte_buffer_view = std::span<uint8_t>;
}

#endif
