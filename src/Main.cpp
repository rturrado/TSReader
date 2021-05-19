#include "Exception.h"
#include "FileReader.h"

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

using namespace TS;



void print_usage()
{
    std::cout << "Usage: ts_reader <TS FILE PATH> [-s|--stats]\n";
}



struct CommandLineValues
{
    std::filesystem::path ts_file_path{};
    bool collect_stats{ false };
};



CommandLineValues parse_command_line(int argc, char* argv[])
{
    namespace po = boost::program_options;

    std::filesystem::path ts_file_path{};
    bool collect_stats{ false };

    po::positional_options_description pd;
    pd.add("ts-file-path", -1);

    po::options_description description{};
    description.add_options()
        ("ts-file-path", po::value<std::filesystem::path>(&ts_file_path), "TS file path")
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

    if (!vm.count("ts-file-path"))
    {
        throw InvalidNumberOfArguments{};
    }
    if (!std::filesystem::exists(ts_file_path))
    {
        throw TSFilePathNotFound{ ts_file_path };
    }
    collect_stats = vm.count("stats");

    return { ts_file_path, collect_stats };
}



int main(int argc, char* argv[])
{
    bool error{ true };
    try
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto [ ts_file_path, collect_stats ] = parse_command_line(argc, argv);
        FileReader ts_reader{ ts_file_path, collect_stats };
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
