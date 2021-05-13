#include "Exception.h"
#include "FileReader.h"
#include "PacketBuffer.h"
#include "PacketParser.h"

#include <array>
#include <fstream>
#include <iostream>

namespace TS
{
    void FileReader::start()
    {
        std::ifstream ifs(_file_path, std::fstream::binary);

        if (!ifs)
        {
            throw CouldNotOpenTSFile(_file_path);
        }

        PacketParser parser{};
        for (PacketBuffer buffer{}; ifs >> buffer; )
        {
            parser.parse(buffer);
        }

        for (auto it = begin(PacketParser::pids);
            it != end(PacketParser::pids);
            it = PacketParser::pids.upper_bound(*it))
        {
            std::cout << "PID: 0x" << std::hex << *it
                << std::dec << "\tcount = " << PacketParser::pids.count(*it)
                << "\n";
        }
    }

    // Tests
    // File path: exists, does not exist, exists but cannot open
    // File size: empty, less than 188, 188, more than 188 but not multiple of 188, more than 188 and multiple of 188
}
