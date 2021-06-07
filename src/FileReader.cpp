#include "Exception.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "PacketBuffer.hpp"
#include "PacketParser.hpp"
#include "PacketProcessor.hpp"
#include "PES_Data.hpp"
#include "PSI_Tables.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace TS
{
    FileReader::FileReader(
        const std::filesystem::path& file_path,
        std::vector<uint8_t>&& stream_type_list,
        bool collect_stats)
        : _stream_type_list{ std::move(stream_type_list) }
        , _collect_stats{ collect_stats }
    {
        _ifs.open(file_path, std::fstream::binary);
        if (!_ifs)
        {
            throw CouldNotOpenTSFile(file_path);
        }
    }

    FileReader::~FileReader()
    {
        _ifs.close();
    }

    void FileReader::start()
    {
        // Initialize writers
        std::vector<std::shared_ptr<FileWriter>> writers{};
        std::for_each(cbegin(_stream_type_list), cend(_stream_type_list),
            [&writers] (uint8_t st) {
                writers.push_back(std::make_unique<FileWriter>(st));
            });

        // Read packets from TS stream loop
        PacketParser parser{};
        PacketProcessor processor{};
        for (PacketBuffer buffer{}; _ifs >> buffer; )
        {
            try
            {
                // Parse packet
                parser.parse(buffer);

                // Process parsed packet
                processor.process(parser.get_packet());

                // Write streams to output files
                PID pid = parser.get_packet().get_PID();
                if (PES_Data::get_instance().has_PES_data(pid))
                {
                    std::for_each(cbegin(writers), cend(writers),
                        [&pid](std::shared_ptr<FileWriter> fw_sptr) {
                            if (fw_sptr->get_stream_type() == PSI_Tables::get_instance().get_PES_stream_type(pid))
                            {
                                fw_sptr->write(PES_Data::get_instance().get_PES_data(pid));
                            }
                        });
                }

                // Collect stats
                if (_collect_stats)
                {
                    Stats& stats = Stats::get_instance();
                    stats.collect(parser.get_packet());
                }
            }
            catch (const std::exception& err)
            {
                std::ostringstream oss{};
                oss << err.what() << "\n\tindex=" << parser.get_packet_index() << ", " << parser.get_packet() << "\n";
                throw std::exception(oss.str().c_str());
            }
        }

        // Print stats summary
        if (_collect_stats)
        {
            const Stats& stats = Stats::get_instance();
            std::cout << "\n" << stats << "\n";
        }
    }

    // Tests
    // File path: exists, does not exist, exists but cannot open
    // File size: empty, less than 188, 188, more than 188 but not multiple of 188, more than 188 and multiple of 188
}
