#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include "Exception.h"
#include "FileReader.h"

using namespace TS;

void print_usage()
{
    std::cout << "\tUsage: ts_reader <TS FILE PATH>\n";
}

auto parse_command_line(int argc, char* argv[])
{
    std::filesystem::path ts_file_path{};
    if (argc != 2)
    {
        throw InvalidNumberOfArguments{};
    }
    else
    {
        ts_file_path = std::string{argv[1]};
        if (!std::filesystem::exists(ts_file_path))
        {
            throw TSFilePathNotFound{ argv[1] };
        }
    }
    return ts_file_path;
}

int main(int argc, char* argv[])
{
    bool error{ true };
    try
    {
        auto ts_file_path = parse_command_line(argc, argv);
        FileReader ts_reader{ts_file_path};
        ts_reader.start();
        error = false;
    }
    catch (const InvalidNumberOfArguments& err)
    {
        std::cout << err.what();
        print_usage();
    }
    catch (const std::exception& err)
    {
        std::cout << err.what();
    }

    std::cout << "Bye!\n";
    return error;
}
