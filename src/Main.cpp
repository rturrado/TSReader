#include "Exception.h"
#include "FileReader.h"
#include "StreamType.h"

#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

using namespace TS;



void print_usage()
{
    std::cout << "Usage: ts_reader <TS FILE PATH> [-e|--extract <STREAM TYPE LIST>] [-s|--stats]\n";
    std::cout << "\n";
    std::cout << "  E.g: ts_reader elephants.ts -e 0xf,0x1b\n";
    std::cout << "       ts_reader elephants.ts --stats\n";
}



struct CommandLineValues
{
    std::filesystem::path ts_file_path{};
    std::vector<uint8_t> stream_type_list{};
    bool collect_stats{ false };
};



CommandLineValues parse_command_line(int argc, char* argv[])
{
    namespace po = boost::program_options;

    std::filesystem::path ts_file_path{};
    std::string stream_type_list_str{};
    bool collect_stats{ false };

    po::positional_options_description pd;
    pd.add("ts-file-path", -1);

    po::options_description description{};
    description.add_options()
        ("ts-file-path", po::value<std::filesystem::path>(&ts_file_path), "TS file path")
        ("extract,e", po::value<std::string>(&stream_type_list_str), "extract stream type to file")
        ("stats,s", "collect stats")
        ;

    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(description).positional(pd).run(), vm);
        po::notify(vm);
    }
    catch (const std::exception& err)
    {
        throw UnrecognizedOption{ err.what() };
    }

    // Parse TS file path
    //
    if (!vm.count("ts-file-path"))
    {
        throw InvalidNumberOfArguments{};
    }
    if (!std::filesystem::exists(ts_file_path))
    {
        throw TSFilePathNotFound{ ts_file_path };
    }

    // Parse extract option
    // 
    std::vector<uint8_t> stream_type_list{};
    if (vm.count("extract"))
    {
        std::istringstream iss{ stream_type_list_str };
        std::string stream_type_str{};
        while (std::getline(iss, stream_type_str, ','))
        {
            uint8_t stream_type{ 0 };

            // Convert stream type to number
            // These operations can throw
            if (stream_type_str.starts_with("0x") or stream_type_str.starts_with("0X"))
            {
                stream_type = std::stoi(stream_type_str, 0, 16);
            }
            else if (stream_type_str.starts_with("0"))
            {
                stream_type = std::stoi(stream_type_str, 0, 8);
            }
            else
            {
                stream_type = std::stoi(stream_type_str);
            }

            // Check stream type is valid
            if (not StreamTypeMap::get_instance().is_valid_stream_type(stream_type))
            {
                throw InvalidStreamType{ stream_type_str.c_str() };
            }

            // Add it to the list of stream types
            stream_type_list.push_back(stream_type);
        }
    }

    // Parse stats option
    //
    collect_stats = vm.count("stats");

    return { ts_file_path, stream_type_list, collect_stats };
}



int main(int argc, char* argv[])
{
    bool error{ true };
    try
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [ ts_file_path, stream_type_list, collect_stats ] = parse_command_line(argc, argv);

        FileReader ts_reader{ ts_file_path, std::move(stream_type_list), collect_stats };
        ts_reader.start();
        error = false;

        auto end = std::chrono::high_resolution_clock::now();
        using fms = std::chrono::duration<long double, std::chrono::seconds::period>;
        auto interval = fms(end - start);
        std::cout << "Time: " << std::fixed << fms(interval).count() << " s\n";
    }
    catch (const CommandLineParserException& err)
    {
        std::cout << "Error: " << err.what() << "\n\n";
        print_usage();
    }
    catch (const std::exception& err)
    {
        std::cout << "Error: " << err.what() << "\n";
    }

    std::cout << "\nBye!\n";
    return error;
}
